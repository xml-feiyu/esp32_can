

#include "def.h"
#include "driver/gpio.h"
#include "driver/can.h"

#define TX_GPIO_NUM GPIO_NUM_21
#define RX_GPIO_NUM GPIO_NUM_22

#define CAN_TX_QUEUE_LEN CAN_LIST_LEN
#define CAN_RX_QUEUE_LEN CAN_LIST_LEN

bool esp32_can_open()
{
//#define CAN_ALERT_ALL                   0x1FFF
//#define CAN_ALERT_NONE                  0x0000
//#define CAN_ALERT_AND_LOG 
    //can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, CAN_MODE_NORMAL);
    can_general_config_t g_config =     {   .mode = CAN_MODE_NORMAL,
                                            .tx_io = TX_GPIO_NUM, 
                                            .rx_io = RX_GPIO_NUM,
                                            .clkout_io = CAN_IO_UNUSED, 
                                            .bus_off_io = CAN_IO_UNUSED,
                                            .tx_queue_len = CAN_TX_QUEUE_LEN, 
                                            .rx_queue_len = CAN_RX_QUEUE_LEN,
                                            .alerts_enabled = CAN_ALERT_NONE,
                                            .clkout_divider = 0};

    can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
    bool flag = (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK);
    if (flag) {
        flag = (can_start() == ESP_OK);
        if (!flag) {
            can_driver_uninstall();
        }
    }
    return flag;
}

bool esp32_can_close()
{
    bool flag1 = (can_stop() == ESP_OK);
    bool flag2 = (can_driver_uninstall() == ESP_OK);
    return (flag1 && flag2);
}


//-----------------------------------------
static can_message_t message_tx;
static can_message_t message_rx;
bool esp32_can_tx_msg(struct CAN_DATA *pCan)
{
    message_tx.identifier = pCan->id;
    message_tx.flags = CAN_MSG_FLAG_NONE;
    //message_tx.flags = CAN_MSG_FLAG_EXTD;
    message_tx.data_length_code = pCan->len;
    for (int i = 0; i < pCan->len; i++) {
        message_tx.data[i] = pCan->buf[i];
    }
    //return (can_transmit(&message_tx, pdMS_TO_TICKS(100)) == ESP_OK);
    return (can_transmit(&message_tx,portMAX_DELAY) == ESP_OK);
}

bool esp32_can_rx_msg(struct CAN_DATA *pCan)
{
    bool flag = (can_receive(&message_rx, pdMS_TO_TICKS(5)) == ESP_OK);
    //bool flag = (can_receive(&message_rx,portMAX_DELAY) == ESP_OK);
    if (flag) {
        //message_rx.flags & CAN_MSG_FLAG_EXTD
        pCan->id = message_rx.identifier;
        pCan->len = message_rx.data_length_code;
        if ((message_rx.flags & CAN_MSG_FLAG_EXTD) ||
            (message_rx.flags & CAN_MSG_FLAG_RTR) ) {
            flag = false;
        }
        else {
            for (int i=0;i<pCan->len && i<CAN_DATA_MAX_LEN;i++)
                pCan->buf[i] = message_rx.data[i];
        }
    }
    return flag;
}

