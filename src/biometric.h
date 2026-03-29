#ifndef BIOMETRIC_H
#define BIOMETRIC_H

#include <stdint.h>
#include <stdbool.h>

struct biometric_data {
    uint8_t  spo2_pct;
    uint8_t  heart_rate_bpm;
    int16_t  collar_temp_c_x100;
    uint16_t gsr_kohm;
    bool     valid;
};

int biometric_init(void);
int biometric_read(struct biometric_data *out);

#endif /* BIOMETRIC_H */
