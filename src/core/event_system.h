#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <stddef.h>

typedef enum {
    EVT_AUDIO_LOADED,
    EVT_FFT_READY,
    EVT_TRACK_END,
    EVT_COUNT
} EventType;

/*takes a pointer to event-specific payload*/
typedef void (*EventHandler)(void *payload);

/*register a handler for an event type*/
void es_register(EventType evt, EventHandler handler);

/*emit an event to all registered handlers*/
void es_emit(EventType evt, void *payload);

/*clear handlers (test teardown)*/
void es_reset(void);

#endif
