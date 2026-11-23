#ifndef DEV2_MOCK_EVENT_GROUPS_H
#define DEV2_MOCK_EVENT_GROUPS_H

#include "FreeRTOS.h"

EventBits_t xEventGroupGetBits(
    EventGroupHandle_t group
);

#endif
