//

#include "taiko_input_processor_v2.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "adc_controller.h"
#include "taiko_controller.h"
#include "utils.h"

TaikoInputProcessorV2::TaikoInputProcessorV2(AdcController *adc_controller,
                                         TaikoController *taiko_controller)
    : adc_controller_(adc_controller), taiko_controller_(taiko_controller) {}

void TaikoInputProcessorV2::StartProcessing() {
  DisableWatchDog(0);
  InitVars();

  int current_value;
  last_time_ = esp_timer_get_time();

  TaikoController::Event events[6];
  memset(events, 0, sizeof(events));
  uint8_t event_index = 0;

  int64_t current_time = 0;

  while (1) {
    // Step 1: Update the sensitivities, read them from ADC.
    UpdateSensitivities();

    // Step 2: Calculate max power and events to send.
    current_time = esp_timer_get_time();
    for (uint8_t p = 0; p < kPlayerCount; p++) {
      if (current_time - triggered_time_[p] >= kResetTime) {
        triggered_[p] = false;
      }

      for (uint8_t i = 0; i < kChannelPerPlayer; i++) {
        adc_controller_->ReadPinValue(kAdcPins[p * kChannelPerPlayer + i],
                                      &current_value);

        // ESP_LOGI(LOG_TAG, "P%d, Channel %d, raw_value: %d", p, i,
        //          current_value);

        input_window_[p][i].put(sensitivities_[i] * current_value);
        power_[p][i] = power_[p][i] - input_window_[p][i].get(1) +
                       input_window_[p][i].get();

        // ESP_LOGI(LOG_TAG, "P%d, Channel %d, power: %d", p, i, power_[p][i]);

        const int use_index = p * kChannelPerPlayer + i;
        if (!triggered_[p] &&
            power_[p][i] - last_power_[p][i] > 0 &&
            power_[p][i] >= kHitThresholdV2[use_index] && event_index < 6) {
          triggered_[p] = true;
          triggered_time_[p] = current_time;
          events[event_index++] = static_cast<TaikoController::Event>(use_index);          
        }

        last_power_[p][i] = power_[p][i];
      }
    }

    // Step 3: Send events.
    if (event_index != 0) {
      taiko_controller_->SendEvents(events, event_index, 0);
      memset(events, 0, sizeof(events));
      event_index = 0;
    }

    // Step 4: Keep the frame time.
    KeepFrameTime();
  }
}

void TaikoInputProcessorV2::InitVars() {
  int64_t current_time = esp_timer_get_time();
  for (uint8_t p = 0; p < kPlayerCount; p++) {
    triggered_[p] = false;
    triggered_time_[p] = current_time;
    for (uint8_t i = 0; i < kChannelPerPlayer; i++) {
      power_[p][i] = 0;
      last_power_[p][i] = 0;
      sensitivities_[i] = 1.0;
    }
  }
}

void TaikoInputProcessorV2::UpdateSensitivities() {
  int current_value;
  for (uint8_t i = 0; i < kChannelPerPlayer; i++) {
    adc_controller_->ReadPinValue(
        kAdcPins[kPlayerCount * kChannelPerPlayer + i], &current_value);
    float x = current_value / 2048.0 - 1;
    float x2 = x * x;
    float x3 = x2 * x;
    sensitivities_[i] = 1.0 + x + 0.5 * x2 + 0.166667 * x3;

    // ESP_LOGI(LOG_TAG, "Channel %d, sensitivity: %.2f", i, sensitivities_[i]);
  }
}

void TaikoInputProcessorV2::KeepFrameTime() {
  frame_time_ = esp_timer_get_time() - last_time_;
  if (frame_time_ < kSamplingPeriodMicroSeconds) {
    DelayMicroseconds(kSamplingPeriodMicroSeconds - frame_time_);
  } else {
    ESP_LOGW(
        LOG_TAG,
        "Frame time not reach expectation, time expect: %d, time used: %lld",
        kSamplingPeriodMicroSeconds, frame_time_);
  }
  last_time_ = esp_timer_get_time();
}

// Static vars.
float TaikoInputProcessorV2::sensitivities_[4];

int64_t TaikoInputProcessorV2::frame_time_;
int64_t TaikoInputProcessorV2::last_time_;
Cache<int, kWindowLength> TaikoInputProcessorV2::input_window_[kPlayerCount]
                                                            [kChannelCount];
int TaikoInputProcessorV2::power_[kPlayerCount][kChannelCount];
int TaikoInputProcessorV2::last_power_[kPlayerCount][kChannelCount];
bool TaikoInputProcessorV2::triggered_[kPlayerCount];
uint64_t TaikoInputProcessorV2::triggered_time_[kPlayerCount];