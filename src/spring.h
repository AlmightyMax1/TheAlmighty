#ifndef SPRING_H
#define SPRING_H

#include <stdint.h>

typedef enum {
    SPRING_MODE_ENDURANCE = 0,
    SPRING_MODE_SPRINT    = 1,
    SPRING_MODE_STABILITY = 2,
} spring_mode_t;

int           spring_init(void);
int           spring_set_mode(spring_mode_t mode);
spring_mode_t spring_get_mode(void);

#endif /* SPRING_H */
