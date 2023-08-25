#ifndef UI_H
#define UI_H

typedef struct channel_configs{
    float       rfv;
    float       tfv;
    uint8_t     rx_ctcss; //rx ctcss code (1~38)
    uint8_t     tx_ctcss; //tx ctcss code (1~38)
    uint8_t     vox;
    bool        band;//1:NarrowBand, 0:WideBand
    bool        power_level; //1:High, 0:Low
}channel_config_t;

#define MAX_CHANNEL_NUM	50

void uiTask(void *arg);
void setChannelAndVolume();
void setTxFreqAndPower();
void setRxFreq();
void setVoxBandCts();
void getChannelInfo(uint8_t channel);

#endif
