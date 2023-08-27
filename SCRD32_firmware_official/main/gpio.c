#include "main.h"

// #define BUTTON_STATE_NORMAL_OFF
#define BUTTON_STATE_NORMAL_ON

#define LED_TX_PIN  (GPIO_NUM_18)
#define LED_RX_PIN  (GPIO_NUM_19)

#define RX_STATE_PIN (GPIO_NUM_39)
#define TX_STATE_PIN (GPIO_NUM_36)

#define MIN_BTN_PIN  (GPIO_NUM_25)//(GPIO_NUM_34)
#define PLUS_BTN_PIN (GPIO_NUM_35)

#define GPIO_INPUT_PIN_SEL   (  (1ULL<< PLUS_BTN_PIN) | (1ULL<< MIN_BTN_PIN) | (1ULL<< RX_STATE_PIN) | (1ULL<< TX_STATE_PIN)  )
#define GPIO_OUTPUT_PIN_SEL  (  (1ULL << LED_RX_PIN)  | (1ULL<< LED_TX_PIN ) ) 

bool gVolumePlusBtnClicked = false;
bool gVolumeMinusBtnClicked = false;
bool gVoxPlusBtnClicked = false;
bool gVoxMinusBtnClicked = false;
bool gChannelPlusBtnClicked = false;
bool gChannelMinusBtnClicked = false;

extern uint8_t gVolume;
extern uint8_t gVox;
extern uint8_t gChannelNum;

void Reset_pin(void)
{
  gpio_reset_pin(LED_RX_PIN);
  gpio_reset_pin(LED_TX_PIN);
}

void plus_btn_timer_enable()
{
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_start(TIMER_GROUP_0, TIMER_0);
}

void plus_btn_timer_disable()
{
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_pause(TIMER_GROUP_0, TIMER_0);
}

void minus_btn_timer_enable()
{
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);
    timer_start(TIMER_GROUP_0, TIMER_1);
}

void minus_btn_timer_disable()
{
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);
    timer_pause(TIMER_GROUP_0, TIMER_1);
}

void Input_pin_config(void)
{
  gpio_config_t io_conf;
  //interrupt of rising edge
  io_conf.intr_type = GPIO_INTR_ANYEDGE;
  //bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //diable pull-up mode
  io_conf.pull_up_en = 0;
  //enable pull-down mode
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);
}

void Gpio_deafult_state_set(void)
{
  /* gpio_set_level(RS485_SELECT,0); // for making the RS484 default to receive */  
  gpio_set_level(LED_RX_PIN, 0); 
  gpio_set_level(LED_TX_PIN, 0); 

  gpio_set_level(LED_RX_PIN, 0);
  gpio_set_level(LED_TX_PIN, 0);
}

void Output_pin_config(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    Gpio_deafult_state_set();
}

static void IRAM_ATTR plusBtnTimerHandler(void *args)
{
    static int cnt = 0;

    cnt++;
    if(cnt > 1200 && cnt < 1500 )
    {
    #ifdef BUTTON_STATE_NORMAL_OFF
        if (gpio_get_level(PLUS_BTN_PIN) == 0)  //1 -> 0
    #elif defined(BUTTON_STATE_NORMAL_ON)
        if (gpio_get_level(PLUS_BTN_PIN) == 1)  //0 -> 1
    #endif 
        {
            cnt = 0;
            plus_btn_timer_disable();

            //volume+ 
            gVolume++;
            if(gVolume > 8) gVolume = 8;
            gVolumePlusBtnClicked = true;
        }
    }
    else if(cnt > 2000)
    {
    #ifdef BUTTON_STATE_NORMAL_OFF
        if (gpio_get_level(PLUS_BTN_PIN) == 0)
    #elif defined(BUTTON_STATE_NORMAL_ON)
        if (gpio_get_level(PLUS_BTN_PIN) == 1)
    #endif        
        {
            plus_btn_timer_disable();
            cnt = 0;
            
            //channel+
            gChannelNum++;
            if(gChannelNum > MAX_CHANNEL_NUM) gChannelNum = MAX_CHANNEL_NUM;
            gChannelPlusBtnClicked = true;
        }
    }
}

static void IRAM_ATTR minusBtnTimerHandler(void *args)
{
    static int cnt = 0;

    cnt++;
    if(cnt > 1200 && cnt < 1500 )
    {
    #ifdef BUTTON_STATE_NORMAL_OFF
        if (gpio_get_level(MIN_BTN_PIN) == 0)
    #elif defined(BUTTON_STATE_NORMAL_ON)
        if (gpio_get_level(MIN_BTN_PIN) == 1)
    #endif
        {
            cnt = 0;
            minus_btn_timer_disable();

            //volume- 
            gVolume--;
            if(gVolume > 8) gVolume = 0;
            gVolumeMinusBtnClicked = true;
        }
    }
    else if(cnt > 2000)
    {
    #ifdef BUTTON_STATE_NORMAL_OFF
        if (gpio_get_level(MIN_BTN_PIN) == 0)
    #elif defined(BUTTON_STATE_NORMAL_ON)
        if (gpio_get_level(MIN_BTN_PIN) == 1)
    #endif
        {
            minus_btn_timer_disable();
            cnt = 0;
            
            //channel-
            if(gChannelNum > 0 && gChannelNum < MAX_CHANNEL_NUM)
            {
                gChannelNum--;
                if(gChannelNum > MAX_CHANNEL_NUM) gChannelNum = 0;
                gChannelMinusBtnClicked = true;
            }
        }
    }
}

void plusBtn_timer_init(void)
{
    uint64_t timer_interval_sec = 10000;//5018: 1ms timer

    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = 16,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, TIMER_0, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, timer_interval_sec  );
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);

    example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
    timer_info->timer_group = TIMER_GROUP_0;
    timer_info->timer_idx = TIMER_0;
    timer_info->auto_reload = true;
    timer_info->alarm_interval = timer_interval_sec;
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, plusBtnTimerHandler, timer_info, 0);
}

void minusBtn_timer_init(void)
{
    uint64_t timer_interval_sec = 5018;//1ms timer

    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = 16,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, TIMER_1, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, timer_interval_sec  );
    timer_enable_intr(TIMER_GROUP_0, TIMER_1);

    example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
    timer_info->timer_group = TIMER_GROUP_0;
    timer_info->timer_idx = TIMER_1;
    timer_info->auto_reload = true;
    timer_info->alarm_interval = timer_interval_sec;
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_1, minusBtnTimerHandler, timer_info, 0);
}

void plusBtnPinConfig(void)
{
  gpio_config_t io_conf;
  //interrupt of rising edge
  io_conf.intr_type = GPIO_INTR_ANYEDGE;
  //bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = (1ULL<< PLUS_BTN_PIN);
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //diable pull-up mode
  io_conf.pull_up_en = 1;
  //enable pull-down mode
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);

  plusBtn_timer_init();
}

void minusBtnPinConfig(void)
{
  gpio_config_t io_conf;
  //interrupt of rising edge
  io_conf.intr_type = GPIO_INTR_ANYEDGE;
  //bit mask of the pins, use GPIO4/5 here
  io_conf.pin_bit_mask = (1ULL<< MIN_BTN_PIN);
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //diable pull-up mode
  io_conf.pull_up_en = 1;
  //enable pull-down mode
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);

  minusBtn_timer_init();
}



void IRAM_ATTR plusBtnIsrHandler(void* arg)
{   
#ifdef BUTTON_STATE_NORMAL_OFF
    if (gpio_get_level(PLUS_BTN_PIN) == 1)
#elif defined(BUTTON_STATE_NORMAL_ON)
    if (gpio_get_level(PLUS_BTN_PIN) == 0)
#endif 
    {
        plus_btn_timer_enable();
    }
}

void IRAM_ATTR minusBtnIsrHandler(void* arg)
{   
#ifdef BUTTON_STATE_NORMAL_OFF
    if (gpio_get_level(MIN_BTN_PIN) == 1)
#elif defined(BUTTON_STATE_NORMAL_ON)
    if (gpio_get_level(MIN_BTN_PIN) == 0)
#endif 
    {
        minus_btn_timer_enable();
    }
}

void set_interrupts(void)
{
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for plusBtnPIN
    gpio_isr_handler_add(PLUS_BTN_PIN, plusBtnIsrHandler, (void*) PLUS_BTN_PIN);
    //hook isr handler for minusBtnPIN
    gpio_isr_handler_add(MIN_BTN_PIN, minusBtnIsrHandler, (void*) MIN_BTN_PIN);

    #if DEBUG_LOG
    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
    #endif
}

void gpioInit(void)
{
    Reset_pin();
    Input_pin_config();
    Output_pin_config();

    minusBtnPinConfig();
    plusBtnPinConfig();
    

    set_interrupts();
}

void gpioTask(void *arg)
{
    gpioInit();
    while(1)
    {
        gpio_set_level(LED_RX_PIN, !gpio_get_level(RX_STATE_PIN));
        gpio_set_level(LED_TX_PIN, gpio_get_level(TX_STATE_PIN));

        // ESP_LOGI("GPIO", "+ Btn = %d,  - Btn = %d", gpio_get_level(PLUS_BTN_PIN), gpio_get_level(MIN_BTN_PIN));
        vTaskDelay(10/ portTICK_PERIOD_MS);
    }
}
