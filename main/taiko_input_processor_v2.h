// Handle taiko sensor input.

#ifndef MAIN_TAIKO_INPUT_PROCESSOR_V2_
#define MAIN_TAIKO_INPUT_PROCESSOR_V2_

#include "taiko_input_processor.h"

#include "cache.h"
#include "params.h"


#ifdef __cplusplus
extern "C" {
#endif

class AdcController;
class TaikoController;

class TaikoInputProcessorV2 : public TaikoInputProcessor {
public:
  TaikoInputProcessorV2(AdcController *adc_controller,
                      TaikoController *taiko_controller);

  void StartProcessing();

private:
  void InitVars();
  void UpdateSensitivities();
  void KeepFrameTime();

  AdcController *adc_controller_;
  TaikoController *taiko_controller_;

  static float sensitivities_[4];

  static int64_t frame_time_;
  static int64_t last_time_;
  static Cache<int, kWindowLength> input_window_[kPlayerCount][kChannelCount];
  static int power_[kPlayerCount][kChannelCount];
  static int last_power_[kPlayerCount][kChannelCount];
  static bool triggered_[kPlayerCount];
  static uint64_t triggered_time_[kPlayerCount];
};

#ifdef __cplusplus
}
#endif

#endif // MAIN_TAIKO_INPUT_PROCESSOR_V2_