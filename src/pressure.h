#ifndef PRESSURE_H
#define PRESSURE_H

#include <stdint.h>
#include <stdbool.h>

struct pressure_data {
    uint32_t fore_kpa;
    uint32_t heel_kpa;
    uint32_t medial_kpa;
    uint32_t lateral_kpa;
    bool     valid;
};

int pressure_init(void);
int pressure_read(struct pressure_data *out);

#endif /* PRESSURE_H */
