// Taiko controller.

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_timer.h>

#include "adc_controller.h"
#include "keyboard_taiko_controller.h"
#include "params.h"
#include "taiko_input_processor.h"
#include "utils.h"

#include "taiko_input_processor_v1.h"
#include "taiko_input_processor_v2.h"
#include "visual_debugger.h"

uint32_t count = 0;
int64_t last_time = 0;

extern "C" void app_main(void) {
  AdcController adc_controller(kAdcPins,
                               sizeof(kAdcPins) / sizeof(kAdcPins[0]));
  KeyBoardTaikoController taiko_controller;
  TaikoInputProcessor *processor = nullptr;
  if (kTaikoInputProcessorVersion == 1) {
    processor = new TaikoInputProcessorV1(&adc_controller, &taiko_controller);
  } else if (kTaikoInputProcessorVersion == 2) {
    processor = new TaikoInputProcessorV2(&adc_controller, &taiko_controller);
  } else {
    processor = new VisualDebugger(&adc_controller, &taiko_controller);
    processor->StartProcessing();
  }

  while (1) {
    if (taiko_controller.IsConnected()) {
      processor->StartProcessing();
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
