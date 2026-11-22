#include "task_comm.h"
#include "task.h"
#include "semphr.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "hal_uart.h"
#include "comm_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define COMM_TASK_STACK_WORDS  256U
#define COMM_POLL_MS           5U      /* buffer UART của Dev 1 chỉ 64 byte */
#define COMM_LINE_MAX          32U
#define COMM_IDLE_TIMEOUT_MS   500U
#define COMM_TX_TIMEOUT_MS     20U     /* mỗi byte */
#define COMM_MUTEX_TIMEOUT_MS  50U

static void reply(const char *s)
{
    (void)APP_UART_SendString(s, COMM_TX_TIMEOUT_MS);
    (void)APP_UART_SendString("\r\n", COMM_TX_TIMEOUT_MS);
}

static bool state_lock(void)
{
    return (xSystemStateMutex != NULL) &&
           (xSemaphoreTake(xSystemStateMutex, pdMS_TO_TICKS(COMM_MUTEX_TIMEOUT_MS)) == pdTRUE);
}

/* float không âm -> số nguyên x10 (nhiệt độ DHT11 0..50, độ ẩm, khoảng cách) */
static int x10(float v)
{
    return (v <= 0.0f) ? 0 : (int)(v * 10.0f + 0.5f);
}

static void send_data(void)
{
    SystemState_t s;
    char buf[96];

    if (!state_lock()) { reply("ERROR BUSY"); return; }
    s = gSystemState;                        /* sao chép nhanh rồi nhả mutex */
    (void)xSemaphoreGive(xSystemStateMutex);

    int t = x10(s.sensor.temperature_c);
    int h = x10(s.sensor.humidity_percent);
    int d = x10(s.sensor.distance_cm);
    int th = x10(s.distance_threshold_cm);

    snprintf(buf, sizeof(buf), "DATA T=%d.%d H=%d.%d D=%d.%d TH=%d.%d SRC=%s ALARM=%lu",
             t / 10, t % 10, h / 10, h % 10, d / 10, d % 10, th / 10, th % 10,
             (s.threshold_source == THRESHOLD_SOURCE_PC) ? "PC" : "POT",
             (unsigned long)s.alarm_flags);
    reply(buf);
}

static void execute(const CommCmd_t *cmd)
{
    char buf[32];
    switch (cmd->type) {
    case CMD_NONE:
        break;
    case CMD_GET_DATA:
        send_data();
        break;
    case CMD_SET_DIST:
        if (!state_lock()) { reply("ERROR BUSY"); break; }
        gSystemState.distance_threshold_cm = (float)cmd->value;
        gSystemState.threshold_source      = THRESHOLD_SOURCE_PC;
        (void)xSemaphoreGive(xSystemStateMutex);
        snprintf(buf, sizeof(buf), "OK DIST=%ld", cmd->value);
        reply(buf);
        break;
    case CMD_SET_SOURCE:
        if (!state_lock()) { reply("ERROR BUSY"); break; }
        gSystemState.threshold_source =
            (cmd->value == 1) ? THRESHOLD_SOURCE_PC : THRESHOLD_SOURCE_POT;
        (void)xSemaphoreGive(xSystemStateMutex);
        reply((cmd->value == 1) ? "OK SOURCE=PC" : "OK SOURCE=POT");
        break;
    case CMD_ERR_INVALID_VALUE:
        reply("ERROR INVALID_VALUE");
        break;
    default:
        reply("ERROR INVALID_COMMAND");
        break;
    }
}

static void prvCommTask(void *argument)
{
    (void)argument;
    char     line[COMM_LINE_MAX];
    uint8_t  idx = 0U, c;
    bool     discard = false;
    TickType_t last_rx = xTaskGetTickCount();

    (void)APP_UART_StartRx();   /* trả false nếu nơi khác đã bật nhận: không sao */

    for (;;) {
        while (APP_UART_ReceiveChar(&c)) {
            last_rx = xTaskGetTickCount();
            if (c == '\r' || c == '\n') {
                if (!discard && idx > 0U) {
                    CommCmd_t cmd;
                    line[idx] = '\0';
                    Comm_ParseLine(line, &cmd);
                    execute(&cmd);
                }
                idx = 0U; discard = false;
            } else if (!discard) {
                if (idx < (COMM_LINE_MAX - 1U)) { line[idx++] = (char)c; }
                else { discard = true; idx = 0U; reply("ERROR LINE_TOO_LONG"); }
            }
        }

        /* Lệnh dở dang quá 500 ms thì bỏ */
        if (idx > 0U &&
            (xTaskGetTickCount() - last_rx) > pdMS_TO_TICKS(COMM_IDLE_TIMEOUT_MS)) {
            idx = 0U;
        }
        vTaskDelay(pdMS_TO_TICKS(COMM_POLL_MS));
    }
}

BaseType_t Comm_TaskCreate(void)
{
    /* Mức ưu tiên do Leader chốt; thấp hơn Sensor Task */
    return xTaskCreate(prvCommTask, "Comm", COMM_TASK_STACK_WORDS, NULL,
                       tskIDLE_PRIORITY + 1U, NULL);
}