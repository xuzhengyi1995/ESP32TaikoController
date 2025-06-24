
#include "visual_debugger.h"

#include "adc_controller.h"
#include "cache.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "params.h"
#include "utils.h"

VisualDebugger::VisualDebugger(AdcController *adc_controller,
                               TaikoController *taiko_controller)
    : adc_controller_(adc_controller), taiko_controller_(taiko_controller) {
  InitUART();
}

void VisualDebugger::StartProcessing() {
  int power[kPlayerCount][kChannelPerPlayer];
  int current_value = 0;

  Cache<int, kWindowLength> input_window[kPlayerCount][kChannelPerPlayer];

  last_time_ = esp_timer_get_time();
  memset(power, 0, sizeof(power));

  while (1) {
    for (uint8_t p = 0; p < kPlayerCount; p++) {
      for (uint8_t i = 0; i < kChannelPerPlayer; i++) {
        adc_controller_->ReadPinValue(kAdcPins[p * kChannelPerPlayer + i],
                                      &current_value);

        // input_window[p][i].put(current_value);
        // power[p][i] =
        // power[p][i] - input_window[p][i].get(1) + input_window[p][i].get();
        power[p][i] = current_value;
      }
    }

    uart_write_bytes(UART_NUM_0, (const char *)power, sizeof(power));
    uart_write_bytes(UART_NUM_0, "\r\n", 3);

    // KeepFrameTime();
  }
}

void VisualDebugger::InitUART() {
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
  ESP_ERROR_CHECK(
      uart_set_pin(UART_NUM_0, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

void VisualDebugger::KeepFrameTime() {
  int64_t frame_time = esp_timer_get_time() - last_time_;
  if (frame_time < kSamplingPeriodMicroSeconds) {
    DelayMicroseconds(kSamplingPeriodMicroSeconds - frame_time);
  }
  last_time_ = esp_timer_get_time();
}

int64_t VisualDebugger::last_time_;
bool VisualDebugger::debug_start_ = false;
