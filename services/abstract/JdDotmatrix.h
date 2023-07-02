//
// generated on 2023-05-16.
// regenerate=0
//

#ifndef JD_CPP_DOTMATRIX_H
#define JD_CPP_DOTMATRIX_H


#if defined(NUM_DISPLAY_COLS)
#include "JacdacService.h"
#include "jacdac-c/jacdac/dist/c/dotmatrix.h"

class JdDotmatrix : public JacdacService {
public:
    explicit JdDotmatrix();
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_DOT_MATRIX;
    }
    

private:
    #define BUFFER_SIZE (NUM_DISPLAY_COLS * ((NUM_DISPLAY_ROWS + 7) >> 3))

    uint16_t rows_ = 0;
    uint16_t cols_ = 0;
    uint8_t variant_ = 0;
    uint8_t brightness_ = 0;
    uint8_t dots_[BUFFER_SIZE]{0};
    uint32_t refresh_ = 0;
    uint8_t was_enabled_ = 0;

    bool is_empty();
	void sync_regs();
};
#endif

#endif //JD_CPP_DOTMATRIX_H