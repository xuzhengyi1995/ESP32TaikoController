// Utils.

#ifndef MAIN_UTILS_
#define MAIN_UTILS_

#include <cstdint>

#define NOP() asm volatile("nop")

void DisableWatchDog(int core_id);

void DelayMicroseconds(int64_t us);

#endif // MAIN_UTILS_