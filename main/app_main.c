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
#include "esp32/rom/ets_sys.h"

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
static const char *TAG = "MQTT";
static const char *subtopic = "/611/AirCondition";


#define IRDA_Pin_LOW() gpio_set_level(IRDA_Pin,0)
#define IRDA_Pin_HIGH() gpio_set_level(IRDA_Pin,1)
#define Delay_ms(x) vTaskDelay(x/portTICK_PERIOD_MS)
#define Delay_us(x) ets_delay_us(x)
void IRDA_AirCondition_Open(void);




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
    gpio_pullup_en(IRDA_Pin);
    gpio_set_pull_mode(IRDA_Pin,GPIO_PULLUP_ONLY);
    IRDA_Pin_HIGH();
    mqtt_app_start();
}

void Start_Code()
{
	IRDA_Pin_LOW();
	Delay_us(9000);
	IRDA_Pin_HIGH();
	Delay_us(4433);
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

void delay_690us()
{
    for(int i=0;i<12000;i++);
}

void Code_1()
{
	IRDA_Pin_LOW();
	Delay_us(690);
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
