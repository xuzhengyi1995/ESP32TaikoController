// Taiko controller interface.

#ifndef MAIN_TAIKO_CONTROLLER_
#define MAIN_TAIKO_CONTROLLER_

#ifdef __cplusplus
extern "C" {
#endif

class TaikoController {
public:
  enum Event : uint8_t {
    P1_L_KA,
    P1_L_DON,
    P1_R_DON,
    P1_R_KA,
    P2_L_KA,
    P2_L_DON,
    P2_R_KA,
    P2_R_DON,
  };
  static_assert(sizeof(Event) == 1);

  virtual void SendEvents(Event events[6], uint8_t event_count,
                          unsigned int power = 0) = 0;

  virtual bool IsConnected() = 0;
};

#ifdef __cplusplus
}
#endif

#endif // MAIN_TAIKO_CONTROLLER_