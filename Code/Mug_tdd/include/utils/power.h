#ifndef __UTILS_POWER_H__
#define __UTILS_POWER_H__

void PreInitHardware(void);
void PostInitHardware(void);

void TurnOn(void);
void Shutdown(void);
void EnterSleepMode(void);
void WakeUp(void);

#endif /* __UTILS_POWER_H__ */
