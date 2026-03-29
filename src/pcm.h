#ifndef PCM_H
#define PCM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PCM_STATE_INACTIVE = 0,
    PCM_STATE_STORING  = 1,
    PCM_STATE_MELTING  = 2,
} pcm_state_t;

int         pcm_init(void);
pcm_state_t pcm_update(int32_t avg_temp_c_x100, bool valve_open_req);
pcm_state_t pcm_get_state(void);

#endif /* PCM_H */
