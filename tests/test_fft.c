#include "../src/audio/fft/fft.h"
#include "vendor/unity/src/unity.h"

void setUp(void) {
    fft_shutdown();
}

void tearDown(void) {
    fft_shutdown();
}

void test_fft_init_valid_size(void) {
    TEST_ASSERT_EQUAL_INT(0, fft_init(256));
    TEST_ASSERT_EQUAL_size_t(256, fft_get_size());
}

void test_fft_init_invalid_size(void) {
    TEST_ASSERT_NOT_EQUAL(0, fft_init(200));
    TEST_ASSERT_EQUAL_size_t(0, fft_get_size());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fft_init_valid_size);
    RUN_TEST(test_fft_init_invalid_size);
    return UNITY_END();
}
