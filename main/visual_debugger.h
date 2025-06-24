
#ifndef MAIN_VISUAL_DEBUGGER_
#define MAIN_VISUAL_DEBUGGER_

#include <stdint.h>

#include "taiko_input_processor.h"

#ifdef __cplusplus
extern "C" {
#endif

class AdcController;
class TaikoController;

class VisualDebugger : public TaikoInputProcessor {
public:
  VisualDebugger(AdcController *adc_controller,
                 TaikoController *taiko_controller);

  void StartProcessing() override;

private:
  void InitUART();
  void KeepFrameTime();

  AdcController *adc_controller_;
  TaikoController *taiko_controller_;

  static bool debug_start_;
  static int64_t last_time_;
};

#ifdef __cplusplus
}
#endif

#endif // MAIN_VISUAL_DEBUGGER_