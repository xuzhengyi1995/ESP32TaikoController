// Adc handler factory.

#ifndef MAIN_ADC_CONTROLLER_
#define MAIN_ADC_CONTROLLER_

#include <esp_adc/adc_oneshot.h>

#ifdef __cplusplus
extern "C" {
#endif

class AdcController {
public:
  AdcController(const uint8_t pins[], int size);
  ~AdcController();

  adc_oneshot_unit_handle_t get_adc_handler(adc_unit_t unit_id) {
    return adc_handlers_[unit_id];
  }

  void ReadPinValue(uint8_t pin, int *value_out);

private:
  void InitAdcChannel(adc_unit_t unit_id, adc_channel_t channel_id);

  struct PinInfo {
    adc_unit_t unit_id;
    adc_channel_t channel_id;
  };

  static adc_oneshot_unit_handle_t adc_handlers_[2];
  static PinInfo pin_info_cache_[21];
};

#ifdef __cplusplus
}
#endif

#endif  // MAIN_ADC_CONTROLLER_