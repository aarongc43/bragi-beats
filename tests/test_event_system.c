#include "vendor/unity/src/unity.h"
#include "../src/core/event_system.h"

static int call_count;
static void handler_a(void *payload) {
    call_count += *(int*)payload;
}
static void handler_b(void *payload) {
    call_count += 2 * (*(int*)payload);
}

void setUp(void) {
    es_reset();
    call_count = 0;
}

void tearDown(void) {
    es_reset();
}

void test_single_handler(void) {
    int payload = 3;
    es_register(EVT_AUDIO_LOADED, handler_a);
    es_emit(EVT_AUDIO_LOADED, &payload);
    TEST_ASSERT_EQUAL_INT(3, call_count);
}

void test_multiple_handlers(void) {
    int payload = 4;
    es_register(EVT_AUDIO_LOADED, handler_a);
    es_register(EVT_AUDIO_LOADED, handler_b);
    es_emit(EVT_AUDIO_LOADED, &payload);
    /* handler_a: +4, handler_b: +8 => total 12 */
    TEST_ASSERT_EQUAL_INT(12, call_count);
}

void test_no_handlers(void) {
    int payload = 5;
    /* no register */
    es_emit(EVT_FFT_READY, &payload);
    TEST_ASSERT_EQUAL_INT(0, call_count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_single_handler);
    RUN_TEST(test_multiple_handlers);
    RUN_TEST(test_no_handlers);
    return UNITY_END();
}
