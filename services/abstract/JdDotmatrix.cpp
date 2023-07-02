#include "JdDotmatrix.h"
#include <cstdint>
extern "C" {
#include "jacdac-c/services/interfaces/jd_pins.h"
#include "jacdac-c/services/interfaces/jd_hw_pwr.h"
#include "jacdac-c/services/interfaces/jd_disp.h"

}


#if defined(NUM_DISPLAY_COLS)






 bool JdDotmatrix::is_empty() {
    if (brightness_ == 0)
        return 1;
    for (int i = 0; i < BUFFER_SIZE; ++i)
        if (dots_[i] != 0)
            return 0;
    return 1;
}

 void JdDotmatrix::process() {
    if (!jd_should_sample(&refresh_, 9900))
        return;
    if (is_empty()) {
        if (was_enabled_) {
            pwr_leave_no_sleep();
            was_enabled_ = 0;
        }
        return;
    }
    if (!was_enabled_) {
        was_enabled_ = 1;
        pwr_enter_no_sleep();
    }
    disp_refresh();
}

 void JdDotmatrix::sync_regs() {
    disp_set_brigthness(brightness_ * brightness_);
}

 void JdDotmatrix::handle_packet(jd_packet_t *pkt) {
    switch (service_handle_register_final(pkt)) {
    case JD_DOT_MATRIX_REG_DOTS:
        disp_show(dots_);
        break;
    case JD_DOT_MATRIX_REG_BRIGHTNESS:
        sync_regs();
        break;
    default:
        break;
    }
}


 JdDotmatrix::JdDotmatrix() {
	addRegister(JD_DOT_MATRIX_REG_ROWS, rows_);
	addRegister(JD_DOT_MATRIX_REG_COLUMNS, cols_);
	addRegister(JD_DOT_MATRIX_REG_VARIANT, variant_);
	addRegister(JD_DOT_MATRIX_REG_BRIGHTNESS, brightness_);
	addRegister(JD_DOT_MATRIX_REG_DOTS, dots_);
    
    rows_ = NUM_DISPLAY_ROWS;
    cols_ = NUM_DISPLAY_COLS;
    variant_ = JD_DOT_MATRIX_VARIANT_LED;
    brightness_ = 100;
    sync_regs();
}

#endif