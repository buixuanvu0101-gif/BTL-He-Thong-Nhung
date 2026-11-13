
#ifndef APP_IPC_H
#define APP_IPC_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "app_cfg.h"

/* Queue chuyen du lieu tu Dev 1 sang Dev 3 */
extern QueueHandle_t xSensorQueue;

/* Queue chuyen su kien nut tu Dev 2 sang Dev 3 */
extern QueueHandle_t xUiEventQueue;

/* Mutex bao ve du lieu he thong */
extern SemaphoreHandle_t xSystemStateMutex;

/* Event Group quan ly canh bao */
extern EventGroupHandle_t xAlarmEventGroup;

/* Bien trang thai he thong */
extern SystemState_t gSystemState;

/* Ham khoi tao cac doi tuong IPC */
BaseType_t app_ipc_init(void);

#endif
