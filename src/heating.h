#ifndef HEATING_H
#define HEATING_H

#include <stdint.h>

#define HEAT_NUM_ZONES 4
#define HEAT_TEMP_MAX  45U

struct heating_state {
    int32_t temp_c_x100[HEAT_NUM_ZONES];
    uint8_t duty_pct[HEAT_NUM_ZONES];
};

int heating_init(void);
int heating_update(const uint8_t targets_c[HEAT_NUM_ZONES],
                   struct heating_state *state);

#endif /* HEATING_H */
