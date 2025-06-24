// Xuzhengyi

#include "keyboard_taiko_controller.h"

#include <esp_log.h>
#include <tinyusb.h>

#include "params.h"

namespace {

// HID releated options.
#define POLLING_INTERVAL_MS 1

#define TUSB_DESC_TOTAL_LEN                                                    \
  (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

const uint8_t kHidReportDescriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD))};

const char *kHidStringDescriptor[] = {
    (char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "Microsoft",          // 1: Manufacturer
    "Taiko controller",   // 2: Product
    "000001",             // 3: Serials, should use chip ID
    "Taiko controller interface", // 4: HID
};

static const uint8_t kHidConfigurationDescriptor[] = {
    // Configuration number, interface count, string index, total length,
    // attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP
    // In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(kHidReportDescriptor), 0x81, 16,
                       POLLING_INTERVAL_MS),
};

static const tinyusb_config_t kUsbCfg = {
    .device_descriptor = NULL,
    .string_descriptor = kHidStringDescriptor,
    .string_descriptor_count =
        sizeof(kHidStringDescriptor) / sizeof(kHidStringDescriptor[0]),
    .external_phy = false,
    .configuration_descriptor = kHidConfigurationDescriptor,
};

} // namespace

// HID callbacks.

extern "C" uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  return kHidReportDescriptor;
}

extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                          hid_report_type_t report_type,
                                          uint8_t *buffer, uint16_t reqlen) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                                      hid_report_type_t report_type,
                                      uint8_t const *buffer, uint16_t bufsize) {
}

// Implementations.

extern "C" KeyBoardTaikoController::KeyBoardTaikoController() {
  ESP_ERROR_CHECK(tinyusb_driver_install(&kUsbCfg));
  InitTaskAndQueue();
}

extern "C" void
KeyBoardTaikoController::SendEvents(KeyBoardTaikoController::Event events[6],
                                    uint8_t event_count, unsigned int power) {
  uint8_t key_codes[6];
  memset(key_codes, 0, sizeof(key_codes));
  for (int i = 0; i < event_count; i++) {
    key_codes[i] = kKeyCodeMap[events[i]];
  }

  xQueueSend(message_queue_, (void *)key_codes, 0);
  xTaskNotifyGive(task_handle_);
}

extern "C" bool KeyBoardTaikoController::IsConnected() { return tud_mounted(); }

extern "C" void KeyBoardTaikoController::InitTaskAndQueue() {
  xTaskCreatePinnedToCore(KeyBoardTaikoController::HIDReportTask,
                          "HID_REPORT_TASK", 1800, nullptr,
                          configMAX_PRIORITIES - 2, &task_handle_, 1);
  message_queue_ = xQueueCreate(10, sizeof(hid_keyboard_report_t::keycode));
}

extern "C" void KeyBoardTaikoController::HIDReportTask(void *) {
  uint8_t key_codes[6];
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    while (1) {
      if (xQueueReceive(message_queue_, &key_codes, 1) == pdPASS) {
        if (!tud_hid_ready()) {
          continue;
        }

        tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, key_codes);
        vTaskDelay(pdMS_TO_TICKS(kKeyboardPressReleaseInterval));
        tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, NULL);
      } else {
        break;
      }
    }
  }
}

// Static vars.
QueueHandle_t KeyBoardTaikoController::message_queue_;
TaskHandle_t KeyBoardTaikoController::task_handle_;