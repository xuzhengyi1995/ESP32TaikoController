//

#include "taiko_input_processor_v1.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "adc_controller.h"
#include "taiko_controller.h"
#include "utils.h"

TaikoInputProcessorV1::TaikoInputProcessorV1(AdcController *adc_controller,
                                         TaikoController *taiko_controller)
    : adc_controller_(adc_controller), taiko_controller_(taiko_controller) {}

void TaikoInputProcessorV1::StartProcessing() {
  DisableWatchDog(0);
  InitVars();

  int current_value;
  last_time_ = esp_timer_get_time();

  TaikoController::Event events[6];
  memset(events, 0, sizeof(events));
  uint8_t event_index = 0;

  // int max_for_debugging = 0;
  // int64_t debugging_time = esp_timer_get_time();

  while (1) {
    // Step 1: Update the sensitivities, read them from ADC.
    UpdateSensitivities();

    // Step 2: Calculate max power and events to send.
    for (uint8_t p = 0; p < kPlayerCount; p++) {
      if (max_index_[p] != -1 &&
          last_power_[p][max_index_[p]] < kResetThreshold[p * kChannelPerPlayer + max_index_[p]]) {
        triggered_[p] = false;
        max_index_[p] = -1;
        max_power_[p] = 0;
      }

      for (uint8_t i = 0; i < kChannelPerPlayer; i++) {
        adc_controller_->ReadPinValue(kAdcPins[p * kChannelPerPlayer + i],
                                      &current_value);

        // ESP_LOGI(LOG_TAG, "P%d, Channel %d, raw_value: %d", p, i,
        //          current_value);

        // if (current_value > max_for_debugging) {
        //   max_for_debugging = current_value;
        // }

        // if (esp_timer_get_time() - debugging_time >= 500000) {
        //   ESP_LOGI(LOG_TAG, "P%d, Channel %d, max_for_debugging: %d", p, i,
        //           max_for_debugging);
        //   debugging_time = esp_timer_get_time();
        //   max_for_debugging = 0;
        // }

        input_window_[p][i].put(sensitivities_[i] * current_value);
        power_[p][i] = power_[p][i] - input_window_[p][i].get(1) +
                       input_window_[p][i].get();

        // ESP_LOGI(LOG_TAG, "P%d, Channel %d, power: %d", p, i, power_[p][i]);

        if (last_power_[p][i] > max_power_[p] &&
            power_[p][i] < last_power_[p][i]) {
          max_power_[p] = last_power_[p][i];
          max_index_[p] = i;
        }
        last_power_[p][i] = power_[p][i];
      }

      const int use_index = p * kChannelPerPlayer + max_index_[p];
      if (!triggered_[p] && max_power_[p] >= kHitThreshold[use_index]) {
        triggered_[p] = true;
        events[event_index++] = static_cast<TaikoController::Event>(use_index);

        // ESP_LOGI(LOG_TAG, "P%d, triggered: %d", p, events[event_index - 1]);
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

void TaikoInputProcessorV1::InitVars() {
  for (uint8_t p = 0; p < kPlayerCount; p++) {
    max_power_[p] = 0;
    max_index_[p] = -1;
    triggered_[p] = false;
    for (uint8_t i = 0; i < kChannelPerPlayer; i++) {
      power_[p][i] = 0;
      last_power_[p][i] = 0;
      sensitivities_[i] = 1.0;
    }
  }
}

void TaikoInputProcessorV1::UpdateSensitivities() {
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

void TaikoInputProcessorV1::KeepFrameTime() {
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
float TaikoInputProcessorV1::sensitivities_[4];

int64_t TaikoInputProcessorV1::frame_time_;
int64_t TaikoInputProcessorV1::last_time_;
Cache<int, kWindowLength> TaikoInputProcessorV1::input_window_[kPlayerCount]
                                                            [kChannelCount];
int TaikoInputProcessorV1::power_[kPlayerCount][kChannelCount];
int TaikoInputProcessorV1::last_power_[kPlayerCount][kChannelCount];
int TaikoInputProcessorV1::max_index_[kPlayerCount];
int TaikoInputProcessorV1::max_power_[kPlayerCount];
bool TaikoInputProcessorV1::triggered_[kPlayerCount];