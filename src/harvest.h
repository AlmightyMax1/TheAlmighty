#ifndef HARVEST_H
#define HARVEST_H

#include <stdint.h>
#include <stdbool.h>

struct harvest_data {
    uint32_t teng_voltage_mv;
    uint32_t teng_power_mw_x10;
    uint32_t em_voltage_mv;
    uint32_t em_power_mw_x10;
    bool     gate_open;
};

int  harvest_init(void);
int  harvest_read(struct harvest_data *out);
void harvest_set_gate(bool open);

#endif /* HARVEST_H */
