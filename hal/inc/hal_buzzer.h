#ifndef HAL_BUZZER_H
#define HAL_BUZZER_H

#include <stdint.h>

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_SetFreq(uint16_t hz);       /* 2000..4000, ngoài khoảng thì bị kẹp */
void Buzzer_SetDuty(uint8_t percent);   /* 0..100 */
void Buzzer_Write(uint8_t on);          /* khớp DisplayBuzzerWriteFn_t của Dev 2 */

#endif