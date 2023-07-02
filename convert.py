import argparse
import glob
import os
import re
from datetime import datetime

from typing import List

temp = open("ServiceHTemplate.txt")
htemplate = temp.read()
temp.close()


def endbracket(string, start=0):
    """
    find the corresponding closing bracket
    :param string: string to search for the closing bracket
    :param start: offset at which the opening bracket is to be found
    :return: position in string at which the closing bracket is found
    """
    chardict = {'{': '}',
                '[': ']',
                '(': ')',
                }
    if not string[start] in chardict:
        raise RuntimeError(
            f"does not start with a supported bracket type \nstarts with {string[start]}, \nsurrounding string: \"{string[start - 5:start + 5]}\"")

    openingbracket = string[start]
    closingbracket = chardict[string[start]]
    stack = 0
    for i in range(start, len(string)):
        char = string[i]
        if char == openingbracket:
            stack += 1
        elif char == closingbracket:
            stack -= 1
        if stack == 0:
            return i
    raise EOFError("no closing bracket found")


def getmembervars(srv_state: str):
    srv_state = re.sub(R"struct srv_state {.*\n.*\w+_COMMON;", "", srv_state)
    srv_state = re.sub(R"(\w+)(\[.*\])*([,;])", "\g<1>_\g<2> = 0\g<3>", srv_state)
    return srv_state[:-2].strip()


def getregisters(service_c: str):
    regdef = get_regdef(service_c)
    if not regdef:
        return ""
    regs = re.findall(R"JD_\w*REG_\w+", regdef)

    # sensor registers are already handled by inheritance
    try:
        regs.remove("JD_REG_STREAMING_SAMPLES")
        regs.remove("JD_REG_STREAMING_INTERVAL")
    except ValueError:
        pass
    return regs


def addregisters(membervar: str, registers: List[str]):
    """
    given a list of registers and the string declaring all member variables return a string which connects register[i] to the i-th membervar
    """
    membervar = re.findall(R"(\w+_)(?:\[.*\])* = 0;", membervar)
    result = "//TODO check if the registers below line up with the correct variable \n"
    for i in range(len(registers)):
        if i < len(membervar):
            result += f"\taddRegister({registers[i]}, {membervar[i]});\n"
        else:
            result += f"\taddRegister({registers[i]}, /* TODO: pass correct variable here */);\n"
    return result


def remove_regs_argument(service_c: str):
    return re.sub(R"(service_handle_register\w*\([\s\S]+), *\w+_regs\)", "\g<1>)", service_c)


def get_regdef(service_c: str):
    match = re.search(R"REG_DEFINITION\(", service_c)
    if not match:
        return ""
    return service_c[match.start():endbracket(service_c, match.end() - 1) + 1]


def registers(service_c: str, membervar: str, initfuncmatch: re.Match):
    """
    changes register registration to the cpp version
    :param service_c: jacdac service definition as a string
    :param membervar: string containing the member variable declarations in the header
    :param initfuncmatch: a regex match to the declaration of the jacdac c service init function
    :return: copy of the input string with the changes
    """
    regs = addregisters(membervar, getregisters(service_c))

    start = service_c.find("{", initfuncmatch.end())
    end = endbracket(service_c, start)
    initfuncdef = service_c[start:end]
    initfuncdef = initfuncdef.replace("\n", "\n" + regs, 1)
    service_c = service_c[:start] + initfuncdef + service_c[end:]

    service_c = remove_regs_argument(service_c)
    service_c = service_c.replace(get_regdef(service_c), "")
    return service_c


def remove_state_argument(service_c: str, funcrepl: List[str]):
    """
    remove the state struct as an argument to be passed to the given functions, affects both declaration and usages
    :param service_c: jacdac service definition as a string
    :param funcrepl: functions beyond those declared in service_c that need to be modified
    :return: copy of input string with the modifications
    """
    for func in re.finditer(R"(\w+) (\w+)\(", service_c):
        funcrepl.append(func.group(0))

    # remove state for functions that don't need it anymore
    for func in funcrepl:
        start = 0
        while True:
            match = service_c.find(func, start)
            if match == -1:
                break
            start = match + len(func)
            end = endbracket(service_c, start - 1)
            function_args = service_c[start:end]
            function_args = re.sub(R".*state *,* *", "", function_args)
            service_c = service_c[:start] + function_args + service_c[end:]

    #   use membervar state for everything else
    service_c = re.sub(R"(\w+\(\s*)state", "\g<1> (srv_t*) state_", service_c)
    return service_c


def functions(service_c: str, classname: str, funcrepl: List[str], basename: str):
    """
    change service function declarations to cpp versions
    :param basename:
    :param service_c: jacdac service definition as a string
    :param classname: name of the class
    :param funcrepl: functions beyond those declared in service_c that need to be modified
    :return: (service_c, private, public) tuple, containing the changes to the input string and the function declarations to be put in the header
    """
    # process and handle packet function implementations
    service_c = re.sub(fR"{basename}_process\(", "process(", service_c)
    service_c = re.sub(fR"{basename}_handle_packet\(", "handle_packet(", service_c)

    functionlist = re.finditer(R"(\w+) (\w+)\(", service_c)
    for func in functionlist:
        funcrepl.append(func.group(2) + '(')

    service_c = remove_state_argument(service_c, funcrepl)

    private, public = get_private_public_methods(service_c)

    def replacefunc(match):
        if isNotInBrackets(service_c, match.start()):
            return f"\n {match.group(2)} {classname}::{match.group(3)}("
        return match.group(0)

    service_c = re.sub(R"[^#](static )*(\b\w+) (\w+)\(", replacefunc, service_c)

    service_c = re.sub(Rf"\w+ {classname}::(\w+)(?<!maybe)_init\(", f"{classname}::{classname}(", service_c)

    service_c = service_c.replace("sensor_maybe_process(", "sensor_maybe_process(")

    return service_c, private, public


def state2membervars(serviceC: str):
    """
    changes all instances of state->var to var_, as the structs members are moved to the class
    :param serviceC: input string
    :return: a copy of the input with the required changes
    """
    membervars = set(re.findall(R"state->(\w+)", serviceC))
    membervars = list(membervars)
    # need to do this to avoid stuff like sample and sample_arr from conflicting and turning into sample__arr
    membervars.sort(reverse=True, key=len)
    for var in membervars:
        serviceC = serviceC.replace("state->" + var, var + "_")
    return serviceC


def isNotInBrackets(fullstr: str, end: int):
    count = 0

    for char in fullstr[:end]:
        if char == '{':
            count += 1
        elif char == '}':
            count -= 1

    return count == 0


def get_private_public_methods(service_c):
    """
    constructs a list of strings containing function declarations for the header file
    :param service_c: jacdac service definition as a string
    :return: tuple (private, public) lists of function declarations
    """
    private = []
    public = []
    for func in re.finditer(RF"[^#](static )*(\b\w+) (\w+)\(", service_c):
        # constructor, process() and handle_packet() are already in the template
        if re.match(R"(\w+)(?<!maybe)_init", func.group(3)) or func.group(3) in ["process", "handle_packet"]:
            continue

        # only get top level function definitions
        if not isNotInBrackets(service_c, func.start()):
            continue

        # static means function only visible in the same file, so it's private while the others are public
        function = f"{func.group(2)} {func.group(3)}("
        arguments = service_c[func.end()
                              :
                              endbracket(service_c, func.end() - 1) + 1]
        if func.group(1):
            private.append(function + arguments + ';')
        else:
            public.append(function + arguments + ';')
    return private, public


def getservicename(initfunc: str):
    """
    :param initfunc: jacdac init function name without the _init at the end
    :return: name of the cpp class
    """
    return "Jd" + "".join([x.capitalize() for x in initfunc.split("_")])


def srv_def_and_alloc(service_c: str):
    """
    finds and removes srv initialisation macros
    :param service_c: jacdac class definition as a string
    :return: (serviceC, serviceclass) tuple, serviceC is the input with the initialisation macros removed
    """
    serviceclass = re.search(R"SRV_DEF\(\w+, (.*)\);", service_c)
    if not serviceclass:
        raise RuntimeError("couldn't find service class")
    service_c = re.sub(R"SRV_DEF\(\w+, \w+\);", "", service_c)
    service_c = re.sub(R"SRV_ALLOC\(\w+\);", "", service_c)
    return service_c, serviceclass.group(1)


def includes(serviceC):
    """
    wraps any includes in extern c {}
    """
    return re.sub(R"((?:#include .*\n*)+)", "extern \"C\" {\n\g<1>}\n", serviceC)


parser = argparse.ArgumentParser(prog='takes in a jacdac-c service and attempts to convert it to a c++ service')
parser.add_argument("-i", "--input", default="../lib/Jacdac/src/jacdac-c/services")
parser.add_argument("-o", "--outdir", default="./services")

args = parser.parse_args()

indirs = []
if os.path.isfile(args.input):
    indirs.append(args.input)
elif os.path.isdir(args.input):
    indirs = glob.glob(args.input + "/*.c")
else:
    raise parser.error("input error")

# exempt these:
exempt = [
    "jd_status_light.c",
    "analogsensor.c",
    "jd_sensor.c"
]


def main():
    for i in indirs:
        if i.endswith(tuple(exempt)):
            print(F"skipping {i}, exempted")
            continue

        with open(i) as file:
            serviceC = file.read()
            serviceH = ""
            serviceCPP = ""

            # get srv_state struct
            match = re.search(R"struct srv_state {", serviceC)
            if not match:
                raise re.error("couldnt find srv_state: " + i)
            srv_struct = serviceC[match.start(): endbracket(serviceC, match.end() - 1) + 2]
            serviceC = serviceC.replace(srv_struct, "")

            funcrepl = [
                "service_handle_register(",
                "service_handle_register_final("
            ]

            if srv_struct.find("SRV_COMMON") != -1:
                baseclass = "JacdacService"
            else:
                baseclass = "JdSensor"
                funcrepl += [
                    "get_status_code(",
                    "sensor_send_status(",
                    "respond_ranges(",
                    "sensor_handle_packet(",
                    "sensor_process_simple(",
                    "sensor_handle_packet_simple(",
                    "sensor_get_reading(",
                    "sensor_maybe_init(",
                    "sensor_maybe_process(",
                    "sensor_should_stream(",
                    "env_sensor_process(",
                    "env_sensor_handle_packet("
                ]
            serviceC = includes(serviceC)

            serviceC = state2membervars(serviceC)

            membervars = getmembervars(srv_struct)
            filename = os.path.splitext(os.path.basename(i))[0]

            # jacdac sometimes has maybe_init as a function, so we need to avoid matching that
            initfuncmatch = re.search(R"void (\w+)(?<!maybe)_init\(", serviceC)
            if not initfuncmatch:
                print(F"skipping {i}, couldn't find init function")
                continue
            constructorargs = serviceC[initfuncmatch.end():endbracket(serviceC, initfuncmatch.end() - 1)]

            serviceC = registers(serviceC, membervars, initfuncmatch)
            servicename = getservicename(initfuncmatch.group(1))

            # skip if there already exists a non-generated version
            if os.path.exists(args.outdir + "/" + servicename + ".h"):
                with open(args.outdir + "/" + servicename + ".h") as hfile:
                    if not re.match(R"//\n// generated on .*\n// regenerate=(true|True|1|TRUE)", hfile.read()):
                        print(F"skipping {i}, exists and should not be regenerated")
                        continue

            serviceC, privateFunc, publicFunc = functions(serviceC, servicename, funcrepl, filename)

            serviceC, serviceClass = srv_def_and_alloc(serviceC)

            serviceH = htemplate.format(creationdate=datetime.today().strftime("%Y-%m-%d"),
                                        filename=filename,
                                        filenameCAPS=filename.upper(),
                                        servicename=servicename,
                                        constructorargs=constructorargs,
                                        serviceclass=serviceClass,
                                        publicfunctions="\n\t".join(publicFunc),
                                        membervars=membervars,
                                        privatefunctions="\n\t".join(privateFunc),
                                        baseclass=baseclass)

            serviceCPP = f"#include \"{servicename}.h\"\n" + serviceC

            with open(args.outdir + "/" + servicename + ".h", "w") as outfile:
                outfile.write(serviceH)

            with open(args.outdir + "/" + servicename + ".cpp", "w") as outfile:
                outfile.write(serviceCPP)


main()
