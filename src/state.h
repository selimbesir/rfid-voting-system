#ifndef STATE_H
#define STATE_H

#define _XTAL_FREQ 4000000

typedef enum {
    STATE_IDLE,
    STATE_AUTH_CHECK,
    STATE_VOTING,
    STATE_DENIED,
    STATE_RESULTS_MODE,
    STATE_RESET_MODE
} SystemState;

extern SystemState state;

void State_Run(void);

#endif