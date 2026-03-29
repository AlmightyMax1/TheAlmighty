#ifndef STF_H
#define STF_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    STF_STATE_SOFT   = 0,
    STF_STATE_LOCKED = 1,
} stf_state_t;

int         stf_init(void);
stf_state_t stf_update(int32_t accel_mag_mg, uint8_t threshold_g);
stf_state_t stf_get_state(void);

#endif /* STF_H */
