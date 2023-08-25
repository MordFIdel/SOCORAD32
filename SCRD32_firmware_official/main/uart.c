#include "main.h"

#define TAG             "UART"
#define TX_TASK_TAG     "TX_TASK"
#define RX_TASK_TAG     "RX_TASK"

uart_event_t         uart_event;
QueueHandle_t        uart_queue;

char uartTxBuf[1025], uartRxBuf[1025];

extern uint8_t gVolume;
extern uint8_t gVox;
extern uint8_t gChannelNum;
extern float   gTFV;
extern float   gRFV;
extern bool    gPowerLevel; //1:High, 0:Low
extern bool    gBand;
extern bool    gBusyLock;
extern bool    gBleMsgRecived;
extern uint8_t gBleMsgRxLen;
extern char BleRxBuf[], BleTxBuf[];
extern bool	gScreenRefresh;

extern esp_gatt_if_t gGatts_if;
extern uint16_t gBleConnID;
extern bool bleConnected;

extern bool gVolumePlusBtnClicked;
extern bool gVolumeMinusBtnClicked;
// extern bool gVoxPlusBtnClicked;
// extern bool gVoxMinusBtnClicked;
extern bool gChannelPlusBtnClicked;
extern bool gChannelMinusBtnClicked;
extern uint8_t	gCtcssList[38][2];
extern uint8_t gRxCtcss;
extern uint8_t gTxCtcss;

extern bool bleConnected;

// static uint8_t retryCnt = 3;

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB, //UART_SCLK_DEFAULT,
    };

    // We won't use a buffer for sending data.
    // uart_driver_install(UART_NUM_1, 1024, 1024, sizeof(uart_queue), &uart_queue, 0);
    uart_driver_install(UART_NUM_1, 1024 * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* logName, const char* data, uint8_t txLen)
{
    const int len = (int)txLen;//strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    // ESP_LOGI(logName, "Wrote %d bytes %s", txBytes, data);
    ESP_LOG_BUFFER_HEXDUMP(TX_TASK_TAG, data, txBytes, ESP_LOG_INFO);
    return txBytes;
}

void uartVolumeSetting()
{
    if(gVolume > 8) gVolume = 8;
    sprintf(uartTxBuf, "AT+DMOVOL=%d\r\n", gVolume);
    
    sendData(TX_TASK_TAG, uartTxBuf, 13);
    memset(uartTxBuf, 0, 1024);
}

void uartVoxSetting()
{
    if(gVox > 8) gVox = 8;
    sprintf(uartTxBuf, "AT+DMOVOX=%d\r\n", gVox);
    
    sendData(TX_TASK_TAG, uartTxBuf, 13);
    memset(uartTxBuf, 0, 1024);
}

void uartChannelSetting()
{
    sprintf(uartTxBuf, "AT+DMOGRP=%.5f,%.5f,RC,TC,B,P\r\n", gRFV, gTFV);

    if(uartTxBuf[30] == 'R' && uartTxBuf[31] == 'C')
    {
        if(gRxCtcss == 0) gRxCtcss = 1;
        else if(gRxCtcss > 38) gRxCtcss = 38;

        if(gTxCtcss == 0) gTxCtcss = 1;
        else if(gTxCtcss > 38) gTxCtcss = 38;
         
        uartTxBuf[30] = gCtcssList[gRxCtcss-1][0];
        uartTxBuf[31] = gCtcssList[gRxCtcss-1][1];
    }
    if(uartTxBuf[33] == 'T' && uartTxBuf[34] == 'C')
    {
        uartTxBuf[33] = gCtcssList[gTxCtcss-1][0];
        uartTxBuf[34] = gCtcssList[gTxCtcss-1][1];
    }
    if(uartTxBuf[36] == 'B')
    {
        if      (gBand == NARROW_BAND && gBusyLock == false) uartTxBuf[36] = '0';
        else if (gBand == NARROW_BAND && gBusyLock == true ) uartTxBuf[36] = '1';
        else if (gBand == WIDE_BAND   && gBusyLock == false) uartTxBuf[36] = '2';
        else if (gBand == WIDE_BAND   && gBusyLock == true)  uartTxBuf[36] = '3';
    }
    if(uartTxBuf[38] == 'P')
    {
        if      (gPowerLevel == HIGH_LEVEL) uartTxBuf[38] = '0';
        else if (gPowerLevel == LOW_LEVEL ) uartTxBuf[38] = '1';
    }
    uartTxBuf[39] = '\r';//0x0D;
    uartTxBuf[40] = '\n';//0x0A;

    sendData(TX_TASK_TAG, uartTxBuf, 41);
    memset(uartTxBuf, 0, 1024);
}

void uartAutoPowerOffDisable()
{
    sprintf(uartTxBuf, "AT+DMOSAV=1\r\n");
    
    sendData(TX_TASK_TAG, uartTxBuf, 13);
    memset(uartTxBuf, 0, 1024);
}

void uartMessageSend()
{
    //sprintf(uartTxBuf, "AT+DMOMES=7ABCDEFG\r\n");//Here 7 is length, must change from 0x37 to 0x07, max msg lenght : 100
    //sprintf(uartTxBuf, "AT+DMOMES=12ABCDEFGHIJKL\r\n");//Here 12 is length, must change from 0x31 0x32 to 0x0C, max msg lenght : 100
    if(BleRxBuf[0]=='A' && BleRxBuf[1]=='T' && BleRxBuf[2]=='+' && 
    BleRxBuf[3]=='D' && BleRxBuf[4]=='M' && BleRxBuf[5]=='O' && 
    BleRxBuf[6]=='M' && BleRxBuf[7]=='E' && BleRxBuf[8]=='S' && BleRxBuf[9]=='=')
    {
        uint8_t msgLen = 0;
        if(gBleMsgRxLen >= 14 && gBleMsgRxLen <= 22)
        {
            memset(uartTxBuf, 0, 1024);
            msgLen = BleRxBuf[10] - 0x30;
            memcpy(uartTxBuf, BleRxBuf, gBleMsgRxLen);
            uartTxBuf[10] = msgLen;
            sendData(TX_TASK_TAG, uartTxBuf, gBleMsgRxLen);
            memset(uartTxBuf, 0, 1024);
        }
        else if(gBleMsgRxLen >= 23 && gBleMsgRxLen <= 113)
        {
            memset(uartTxBuf, 0, 1024);
            msgLen = (BleRxBuf[10] - 0x30)*10 + BleRxBuf[11] - 0x30;
            memcpy(uartTxBuf, BleRxBuf, gBleMsgRxLen);
            uartTxBuf[10] = msgLen;
            memmove(&uartTxBuf[11], &uartTxBuf[12], msgLen);
            sendData(TX_TASK_TAG, uartTxBuf, gBleMsgRxLen - 1);
            memset(uartTxBuf, 0, 1024);
        }
        else if(gBleMsgRxLen == 115)
        {
            memset(uartTxBuf, 0, 1024);
            msgLen = 100;
            memcpy(uartTxBuf, BleRxBuf, gBleMsgRxLen);
            uartTxBuf[10] = msgLen;
            memmove(&uartTxBuf[11], &uartTxBuf[13], msgLen);
            sendData(TX_TASK_TAG, uartTxBuf, gBleMsgRxLen - 2);
            memset(uartTxBuf, 0, 1024);
        }
    }
}

void saveChannelInfo()
{
    char local_write[3] = {0};
    local_write[0] = 'C';
    local_write[1] = 'H';
    local_write[2] = gChannelNum;
    ESP_LOGI("EEPROM", "gChannelNum = %d", gChannelNum);
    writeEPPROM(local_write, sizeof(local_write), ADDRESS_CHANNEL);
}

void readChannelInfo()
{
    char local_read[3] = {0};
    readEPPROM(local_read, sizeof(local_read), ADDRESS_CHANNEL);
    if(local_read[0] == 'C' && local_read[1] == 'H')
    {
        gChannelNum = local_read[2];
    }
}

static void uart_tx_task(void *arg)
{
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    memset(uartTxBuf, 0, 1024);

    while (1) {
        if((gVolumePlusBtnClicked || gVolumeMinusBtnClicked))
        {
            ESP_LOGI(TX_TASK_TAG, "V+ clicked:%d, V- clicked:%d, gVolume=%d", gVolumePlusBtnClicked, gVolumeMinusBtnClicked, gVolume);
            if(gVolumePlusBtnClicked == true)  gVolumePlusBtnClicked = false;
            if(gVolumeMinusBtnClicked == true) gVolumeMinusBtnClicked = false;
            
            uartVolumeSetting();
        } 

        if(gChannelPlusBtnClicked || gChannelMinusBtnClicked)
        {
            if(gChannelPlusBtnClicked)     gChannelPlusBtnClicked = false;
            if(gChannelMinusBtnClicked)    gChannelMinusBtnClicked = false;

            getChannelInfo(gChannelNum);
            uartVoxSetting();
        }

        if(gBleMsgRecived)
        {
            // uartAutoPowerOffDisable();
            // uartMessageSend();
            if(BleRxBuf[0]=='A' && BleRxBuf[1]=='T' && BleRxBuf[2]=='+' && 
            BleRxBuf[3]=='D' && BleRxBuf[4]=='M' && BleRxBuf[5]=='O' && 
            BleRxBuf[6]=='M' && BleRxBuf[7]=='E' && BleRxBuf[8]=='S' && BleRxBuf[9]=='=')
            {
                uint8_t msgLen = 0;
                if(gBleMsgRxLen >= 14 && gBleMsgRxLen <= 22)
                {
                    memset(uartTxBuf, 0, 1024);
                    msgLen = BleRxBuf[10] - 0x30;
                    memcpy(uartTxBuf, BleRxBuf, gBleMsgRxLen);
                    uartTxBuf[10] = msgLen;
                    sendData(TX_TASK_TAG, uartTxBuf, gBleMsgRxLen);
                    memset(uartTxBuf, 0, 1024);
                }
                else if(gBleMsgRxLen >= 23 && gBleMsgRxLen <= 113)
                {
                    memset(uartTxBuf, 0, 1024);
                    msgLen = (BleRxBuf[10] - 0x30)*10 + BleRxBuf[11] - 0x30;
                    memcpy(uartTxBuf, BleRxBuf, gBleMsgRxLen);
                    uartTxBuf[10] = msgLen;
                    memmove(&uartTxBuf[11], &uartTxBuf[12], msgLen);
                    sendData(TX_TASK_TAG, uartTxBuf, gBleMsgRxLen - 1);
                    memset(uartTxBuf, 0, 1024);
                }
                else if(gBleMsgRxLen == 115)
                {
                    memset(uartTxBuf, 0, 1024);
                    msgLen = 100;
                    memcpy(uartTxBuf, BleRxBuf, gBleMsgRxLen);
                    uartTxBuf[10] = msgLen;
                    memmove(&uartTxBuf[11], &uartTxBuf[13], msgLen);
                    sendData(TX_TASK_TAG, uartTxBuf, gBleMsgRxLen - 2);
                    memset(uartTxBuf, 0, 1024);
                }
            }
            else if(BleRxBuf[0]=='A' && BleRxBuf[1]=='T' && BleRxBuf[2]=='+' && 
            BleRxBuf[3]=='D' && BleRxBuf[4]=='M' && BleRxBuf[5]=='O' && 
            BleRxBuf[6]=='C' && BleRxBuf[7]=='H' && BleRxBuf[8]=='N' && BleRxBuf[9]=='=')
            {
                if(gBleMsgRxLen == 13)
                {
                    gChannelNum = (BleRxBuf[10] - 0x30);
                    getChannelInfo(gChannelNum);
                    uartVoxSetting();
                    // uartChannelSetting();
                }
                else if(gBleMsgRxLen == 14)
                {
                    gChannelNum = (BleRxBuf[10] - 0x30)* 10 + (BleRxBuf[11] - 0x30);
                    getChannelInfo(gChannelNum);
                    uartVoxSetting();
                    // uartChannelSetting();
                }
            }
            else
            {
                memset(uartTxBuf, 0, 1024);
                memcpy(uartTxBuf, BleRxBuf, gBleMsgRxLen);
                sendData(TX_TASK_TAG, uartTxBuf, gBleMsgRxLen);
                memset(uartTxBuf, 0, 1024);
            }
            gBleMsgRecived = false;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void uart_rx_task(void *arg)
{
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        memset(uartRxBuf, 0, 1025);
        const int rxLen = uart_read_bytes(UART_NUM_1, uartRxBuf, 1024, 1000/portTICK_RATE_MS);
        if(rxLen > 0)
        {
            uartRxBuf[rxLen] = 0;

            if(uartRxBuf[0]=='+' && uartRxBuf[1]=='D' && uartRxBuf[2]=='M' && uartRxBuf[3]=='O')
            {
                ESP_LOGI(RX_TASK_TAG, "%s", uartRxBuf);
                if(uartRxBuf[4]=='V' && uartRxBuf[5]=='O' && uartRxBuf[6]=='L' && uartRxBuf[7]==':')
                {
                    if(uartRxBuf[8]== 0x30 && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Volume Setting Success");
                        setChannelAndVolume();
                        // retryCnt = 3;
                    }
                    else if(uartRxBuf[8]=='1' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Volume Setting Fail");
                    }
                }
                else if(uartRxBuf[4]=='V' && uartRxBuf[5]=='O' && uartRxBuf[6]=='X' && uartRxBuf[7]==':')
                {
                    if(uartRxBuf[8]=='0' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Vox Setting Success");
                        setVoxBandCts();
                        uartChannelSetting();
                        // retryCnt = 3;
                    }
                    else if(uartRxBuf[8]=='1' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Vox Setting Fail");
                        //Save Setting to EEPROM and Notify to tx task
                    }
                }
                else if(uartRxBuf[4]=='G' && uartRxBuf[5]=='R' && uartRxBuf[6]=='P' && uartRxBuf[7]==':')
                {
                    if(uartRxBuf[8]=='0' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Channel Setting Success");
                        
                        saveChannelInfo();
                        
                        getChannelInfo(gChannelNum);

                        gScreenRefresh = true;
                    }
                    else if(uartRxBuf[8]=='1' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Channel Setting Fail");
                    }
                }
                else if(uartRxBuf[4]=='S' && uartRxBuf[5]=='A' && uartRxBuf[6]=='V' && uartRxBuf[7]==':')
                {
                    if(uartRxBuf[8]=='0' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Auto Power Off Disable Success");
                    }
                    else if(uartRxBuf[8]=='1' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Auto Power Off Disable fail");
                    }
                }
                else if(uartRxBuf[4]=='M' && uartRxBuf[5]=='E' && uartRxBuf[6]=='S' && uartRxBuf[7]==':')
                {
                    if(uartRxBuf[8]=='0' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Message Sending Success");
                    }
                    else if(uartRxBuf[8]=='1' && uartRxBuf[9]=='\r' && uartRxBuf[10]=='\n')
                    {
                        ESP_LOGI(RX_TASK_TAG, "Message Sending fail");
                    }
                }
                if(bleConnected)
                {
                    uint16_t txLen = 0;
                    if(rxLen > 30) txLen = 30;
                    else txLen = rxLen;
                    
                    memcpy(BleTxBuf, uartRxBuf, txLen);
                    sendResponse(gGatts_if, gBleConnID, (uint8_t*)BleTxBuf, txLen);
                }
            }
            
            memset(uartRxBuf, 0, 1024);
        }
        vTaskDelay(10/ portTICK_PERIOD_MS);
    }
}

void createUartTasks(void)
{
    uart_init();
    xTaskCreatePinnedToCore(uart_tx_task, "uart_tx_task", 4096, NULL, configMAX_PRIORITIES-1, NULL, 1);
    xTaskCreatePinnedToCore(uart_rx_task, "uart_rx_task", 4096, NULL, configMAX_PRIORITIES,   NULL, 1);
}