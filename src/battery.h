#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

struct battery_data {
    uint32_t voltage_mv;
    uint8_t  soc_pct;
};

int battery_init(void);
int battery_read(struct battery_data *out);

#endif /* BATTERY_H */
