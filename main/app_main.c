/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "protocol_examples_common.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_types.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

#define LED 2
#define IRDA_Pin 15
static const char *TAG = "MQTT_EXAMPLE";
static const char *subtopic = "/611/AirCondition";
int delay_flag=0;

#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

#define IRDA_Pin_LOW() gpio_set_level(IRDA_Pin,0)
#define IRDA_Pin_HIGH() gpio_set_level(IRDA_Pin,1)
#define Delay_ms(x) vTaskDelay(x/portTICK_PERIOD_MS)
void IRDA_AirCondition_Open(void);


void IRAM_ATTR timer_group0_isr(void *para)
{
    timer_spinlock_take(TIMER_GROUP_0);
    int timer_idx = (int) para;

    /* Retrieve the interrupt status and the counter value
       from the timer that reported the interrupt */
    uint32_t timer_intr = timer_group_get_intr_status_in_isr(TIMER_GROUP_0);
    uint64_t timer_counter_value = timer_group_get_counter_value_in_isr(TIMER_GROUP_0, timer_idx);

    /* Clear the interrupt
       and update the alarm time for the timer with without reload */
    if (timer_intr & TIMER_INTR_T0) {
        evt.type = TEST_WITHOUT_RELOAD;
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
	    delay_flag=1;
	    
    	}

    /* After the alarm has been triggered
      we need enable it again, so it is triggered the next time */
    timer_spinlock_give(TIMER_GROUP_0);
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            msg_id = esp_mqtt_client_subscribe(client, subtopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, subtopic, "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            if(event->data[0] == '1')
            {
                gpio_set_level(LED,1);
                IRDA_AirCondition_Open();
            }
            else if(event->data[0] == '0')
            {
                gpio_set_level(LED,0);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://mqtt2.janeymqtt.xyz",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

static void our_timer_init(int timer_idx,
                                   bool auto_reload, double timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = 0,
    }; // default clock source is APB
    timer_init(TIMER_GROUP_0, timer_idx, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    /* Configure the alarm value and the interrupt on alarm. */
//     timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, timer_idx);
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr,
                       (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

//     timer_start(TIMER_GROUP_0, timer_idx);
}

static void Delay_us(int x)
{
	timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);
    	timer_set_alarm_value(TIMER_GROUP_0, timer_idx, x * TIMER_SCALE/1000000);
	timer_start(TIMER_GROUP_0, timer_idx);
	while(!delay_flag);
	delay_flag=0;
}

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED,GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(IRDA_Pin);
    gpio_set_direction(IRDA_Pin,GPIO_MODE_OUTPUT);
    mqtt_app_start();
}

void Start_Code()
{
	IRDA_Pin_LOW();
	Delay_us(9018);
	IRDA_Pin_HIGH();
	Delay_us(4435);
}

void Connect_Code()
{
	IRDA_Pin_LOW();
	Delay_us(728);
	IRDA_Pin_HIGH();
	Delay_us(19849);
}

void Finish_Code()
{
	IRDA_Pin_LOW();
	Delay_us(663);
	IRDA_Pin_HIGH();
}

void Code_1()
{
	IRDA_Pin_LOW();
	Delay_us(691);
	IRDA_Pin_HIGH();
	Delay_us(1629);
}

void Code_0()
{
	IRDA_Pin_LOW();
	Delay_us(666);
	IRDA_Pin_HIGH();
	Delay_us(509);
}

void IRDA_AirCondition_Open()
{
	Start_Code();
	Code_1();
	Code_0();
	Code_0();
	Code_1();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_1();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_1();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_1();
	Code_0();
	Code_1();
	Code_0();
	Code_0();
	Code_1();
	Code_0();
	Connect_Code();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_1();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_0();
	Code_1();
	Code_0();
	Code_0();
	Code_1();
	Finish_Code();
}
