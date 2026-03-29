#ifndef MUX_H
#define MUX_H

#include <stdint.h>

typedef enum { MUX_A = 0, MUX_B = 1 } mux_id_t;

#define MUXA_TENG    0U
#define MUXA_EM_GEN  1U
#define MUXA_BATTERY 2U
#define MUXA_GSR     3U

#define MUXB_EMG0    0U
#define MUXB_EMG1    1U
#define MUXB_EMG2    2U
#define MUXB_EMG3    3U

#define MUX_A_ADC_CH  0
#define MUX_B_ADC_CH  1

int  mux_init(void);
int  mux_select(mux_id_t mux, uint8_t ch);
void mux_release(mux_id_t mux);

#endif /* MUX_H */
