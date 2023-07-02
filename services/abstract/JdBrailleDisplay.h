//
// generated on 2023-05-16.
// regenerate=0
//

#ifndef JD_CPP_BRAILLE_DISPLAY_H
#define JD_CPP_BRAILLE_DISPLAY_H

#include "services/JacdacService.h"
#include "jacdac-c/jacdac/dist/c/brailledisplay.h"
#include "jacdac-c/services/jd_services.h"

class JdBrailleDisplay : public JacdacService {
public:
    explicit JdBrailleDisplay(uint8_t length);
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_BRAILLE_DISPLAY;
    }

    void initialize() override;

private:
    virtual void init() = 0;
    virtual void channel_clear(uint8_t channel) = 0;
    virtual void channel_set(uint8_t channel, int element) = 0;
    virtual void clear_all() = 0;
    virtual void write_channels() = 0;
    virtual uint16_t write_raw(uint16_t d) = 0;
    virtual uint32_t get_channels(int row, int col) = 0;

#define DOTS_MAX 76

    uint8_t enabled_ = 0;
    uint8_t length_ = 0;
    uint8_t flags_ = 0;
    uint8_t dots_[DOTS_MAX]{0};
    uint8_t tmpbuf_[JD_SERIAL_PAYLOAD_SIZE]{0};

    void braile_display_send();
	void handle_get();
	void handle_set(jd_packet_t *pkt);
};


#endif //JD_CPP_BRAILLE_DISPLAY_H