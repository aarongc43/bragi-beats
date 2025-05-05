#include "vendor/unity/src/unity.h"
#include "../src/audio/audio_engine.h"
#include "vendor/unity/src/unity_internals.h"

#define FIXTURE_PATH "tests/fixtures/test_mono_44k.wav"

static WAV wav;

void setUp(void) {
    wav.samples = NULL;
}

void tearDown(void) {
    if (wav.samples) wav_free(&wav);
}

void test_wav_load_success(void) {
    int err = wav_load(FIXTURE_PATH, &wav);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_EQUAL_UINT16(1, wav.audio_format);
    TEST_ASSERT_EQUAL_UINT16(1, wav.num_channels);
    TEST_ASSERT_EQUAL_UINT32(44100, wav.sample_rate);
    TEST_ASSERT_EQUAL_UINT16(16, wav.bits_per_sample);
    TEST_ASSERT_NOT_NULL(wav.samples);
    TEST_ASSERT_TRUE(wav.num_samples > 0);
}

void test_wav_load_missing_file(void) {
    int err = wav_load("does_not_exist.wav", &wav);
    TEST_ASSERT_NOT_EQUAL(0, err);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_wav_load_success);
    RUN_TEST(test_wav_load_missing_file);
    return UNITY_END();
}
