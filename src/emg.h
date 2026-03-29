#ifndef EMG_H
#define EMG_H

#include <stdint.h>
#include <stdbool.h>

#define EMG_NUM_SITES 4

struct emg_data {
    int16_t mv[EMG_NUM_SITES];
    bool    valid;
};

int emg_init(void);
int emg_read(struct emg_data *out);

#endif /* EMG_H */
