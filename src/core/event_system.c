#include "event_system.h"
#include <stdlib.h>
#include <string.h>

#define MAX_HANDLERS_PER_EVENT 16

static EventHandler handlers[EVT_COUNT][MAX_HANDLERS_PER_EVENT];
static size_t handler_count[EVT_COUNT];

void es_register(EventType evt, EventHandler handler) {
    if (evt >= EVT_COUNT || handler_count[evt] >= MAX_HANDLERS_PER_EVENT) {
        return;
    }
    handlers[evt][handler_count[evt]++] = handler;
}

void es_emit(EventType evt, void *payload) {
    if (evt >= EVT_COUNT) return;
    for (size_t i = 0; i < handler_count[evt]; ++i) {
        handlers[evt][i](payload);
    }
}

void es_reset(void) {
    memset(handler_count, 0, sizeof(handler_count));
    /*no need to clear handlers array*/
}
