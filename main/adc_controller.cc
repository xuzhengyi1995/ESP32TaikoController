
#include "adc_controller.h"

#include <string.h>

namespace {
static const adc_oneshot_chan_cfg_t config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};
}

extern "C" AdcController::AdcController(const uint8_t pins[], int size) {
  memset(adc_handlers_, 0, sizeof(adc_handlers_));

  adc_unit_t unit_id;
  adc_channel_t channel_id;

  for (int i = 0; i < size; i++) {
    adc_oneshot_io_to_channel(pins[i], &unit_id, &channel_id);
    pin_info_cache_[pins[i]] = {unit_id, channel_id};
    InitAdcChannel(unit_id, channel_id);
  }
}

extern "C" AdcController::~AdcController() {
  for (auto adc_handler : adc_handlers_) {
    if (adc_handler) {
      ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handler));
    }
  }
}

extern "C" void AdcController::ReadPinValue(uint8_t pin, int *value_out) {
  adc_oneshot_read(adc_handlers_[pin_info_cache_[pin].unit_id],
                   pin_info_cache_[pin].channel_id, value_out);
}

extern "C" void AdcController::InitAdcChannel(adc_unit_t unit_id,
                                              adc_channel_t channel_id) {
  if (!adc_handlers_[unit_id]) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit_id,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&init_config, &adc_handlers_[unit_id]));
  }

  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc_handlers_[unit_id], channel_id, &config));
}

// Static vars.
adc_oneshot_unit_handle_t AdcController::adc_handlers_[2];
AdcController::PinInfo AdcController::pin_info_cache_[21];