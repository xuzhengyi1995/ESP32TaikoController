
#ifndef MAIN_PARAMS_
#define MAIN_PARAMS_

#include <cstdint>
#include <tinyusb.h>

// Input pins for each channel.
#define P1_L_DON_IN 4
#define P1_L_KAT_IN 5
#define P1_R_DON_IN 6
#define P1_R_KAT_IN 7

#define P2_L_DON_IN 8
#define P2_L_KAT_IN 1
#define P2_R_DON_IN 9
#define P2_R_KAT_IN 10

// Sensitive.
#define L_DON_SEN 15
#define L_KAT_SEN 16
#define R_DON_SEN 17
#define R_KAT_SEN 18

#define USE_ANALOG_SENSITIVE_INPUT true

constexpr int kTaikoInputProcessorVersion = 2;

// Threshold
static constexpr unsigned long kHitThreshold[] = {
    850, 90,  800, 800, // P1, L_KA, L_DON, R_DON, R_KA
    400, 100, 120, 300, // P2, L_KA, L_DON, R_DON, R_KA
};
static constexpr unsigned long kResetThreshold[] = {
    750, 50, 300, 750, // P1, L_KA, L_DON, R_DON, R_KA
    250, 50, 50,  250, // P2, L_KA, L_DON, R_DON, R_KA
};

// Sliding window length.
static constexpr uint8_t kWindowLength = 32;

// Players.
static constexpr uint8_t kPlayerCount = 2;
static constexpr uint8_t kChannelPerPlayer = 4;
static constexpr uint8_t kChannelCount = kPlayerCount * kChannelPerPlayer;

// Frame period in micro seconds (μs). Default is 500 -> 2000Hz.
static constexpr int kSamplingPeriodMicroSeconds = 250;

// Reset time in microseconds, used in V2.
static constexpr uint64_t kResetTime = 60 * kSamplingPeriodMicroSeconds;

static constexpr unsigned long kHitThresholdV2[] = {
    1000, 10,  1,   1000, // P1, L_KA, L_DON, R_DON, R_KA
    100,  100, 100, 100,  // P2, L_KA, L_DON, R_DON, R_KA
};

// Keyboard press release interval (ms).
static constexpr uint8_t kKeyboardPressReleaseInterval = 2;

static constexpr char LOG_TAG[] = "TaikoController";

static constexpr uint8_t kAdcPins[] = {
    P1_L_KAT_IN, P1_L_DON_IN, P1_R_DON_IN, P1_R_KAT_IN, // P1
    P2_L_KAT_IN, P2_L_DON_IN, P2_R_DON_IN, P2_R_KAT_IN, // P2
#if USE_ANALOG_SENSITIVE_INPUT
    L_DON_SEN,   L_KAT_SEN,   R_DON_SEN,   R_KAT_SEN // SEN
#endif

};

// Keymap.
static constexpr uint8_t kKeyCodeMap[] = {
    HID_KEY_D, HID_KEY_F, HID_KEY_J, HID_KEY_K, // P1 (LK, LD, RD, RK)
    HID_KEY_E, HID_KEY_R, HID_KEY_U, HID_KEY_I, // P2 (LK, LD, RD, RK)
};
static_assert(sizeof(kKeyCodeMap) == 8);

#endif // MAIN_PARAMS_r