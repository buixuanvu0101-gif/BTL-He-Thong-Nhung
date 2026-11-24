#ifndef BUTTON_EXTI_H
#define BUTTON_EXTI_H

#include <stdint.h>

/* Optional interrupt-driven button path.
 * Enable with -DDEV2_BUTTON_MODE_EXTI=1 only after Leader changes
 * PB10 to GPIO_MODE_IT_RISING_FALLING in CubeMX.
 * The ISR sends short/long UiEvent_t to xUiEventQueue using FromISR.
 * A long press event is generated on RELEASE, not while held.
 */

#if defined(DEV2_BUTTON_MODE_EXTI) && DEV2_BUTTON_MODE_EXTI == 1
void button_exti_enable(void);
uint32_t button_exti_get_queue_drop_count(void);
#endif

#endif
