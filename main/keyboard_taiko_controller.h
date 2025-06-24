// Keyboard controller.

#ifndef MAIN_KEYBOARD_TAIKO_CONTROLLER_
#define MAIN_KEYBOARD_TAIKO_CONTROLLER_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "taiko_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

class KeyBoardTaikoController : public TaikoController {
public:
  KeyBoardTaikoController();

  void SendEvents(Event events[6], uint8_t event_count,
                  unsigned int power = 0) override;
  bool IsConnected() override;

private:
  void InitTaskAndQueue();
  static void HIDReportTask(void *);

  static QueueHandle_t message_queue_;
  static TaskHandle_t task_handle_;
};

#ifdef __cplusplus
}
#endif

#endif // MAIN_KEYBOARD_TAIKO_CONTROLLER_