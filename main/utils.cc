
#include "utils.h"

#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void DisableWatchDog(int core_id) {
  TaskHandle_t idle = xTaskGetIdleTaskHandleForCPU(core_id);
  if (idle) {
    esp_task_wdt_delete(idle);
  }
}

void DelayMicroseconds(int64_t us) {
  int64_t m = esp_timer_get_time();
  if (us) {
    int64_t e = (m + us);
    if (m > e) { // overflow
      while (esp_timer_get_time() > e) {
        NOP();
      }
    }
    while (esp_timer_get_time() < e) {
      NOP();
    }
  }
}