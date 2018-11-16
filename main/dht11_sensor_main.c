#include "mjd.h"
#include "mjd_dht11.h"

/*
 * Logging
 */
/////static const char *TAG = "myapp";
static const char TAG[] = "myapp";


/*
 * KConfig: LED, WIFI, SENSOR MODEL
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_TEMPERATURE_SENSOR_1WIRE_GPIO_NUM = CONFIG_MY_TEMPERATURE_SENSOR_1WIRE_GPIO_NUM;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)


/*
 * Project Globs
 */

/*
 * INIT
 */

/*
 * TASK
 */

void sensor_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "@info LED output pin         GPIO#%5i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "@info Sensor DHT11 input pin GPIO#%5i", MY_TEMPERATURE_SENSOR_1WIRE_GPIO_NUM);

    mjd_led_config_t led_config = { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM; // (Huzzah32 #13) (Lolin32lite #22)
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // (Huzzah32 1=GND) (Lolin32lite 2=VCC)
    mjd_led_config(&led_config);

    mjd_dht11_config_t dht11_config = { 0 };
    dht11_config.gpio_pin = MY_TEMPERATURE_SENSOR_1WIRE_GPIO_NUM;
    dht11_config.rmt_channel = RMT_CHANNEL_0;
    mjd_dht11_init(&dht11_config);

    int total_nbr_of_fatal_error_readings = 0;

    while (1) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        mjd_dht11_data_t dht11_data = { 0 };

        if (mjd_dht11_read(&dht11_config, &dht11_data) != ESP_OK) {
            ESP_LOGW(TAG, "DHT11 Sensor failure");
            total_nbr_of_fatal_error_readings++;
            continue; // while
        }

        printf("*** DHT11 SENSOR READING: Temperature %.2f*C | Humidity %.2f%% | #fatal error readings: %d\n", dht11_data.temperature_celsius, dht11_data.humidity_percent, total_nbr_of_fatal_error_readings);

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        ESP_LOGD(TAG, "@doc Wait 5 seconds at the end of the while(1)");
        vTaskDelay(RTOS_DELAY_5SEC);
    }
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    BaseType_t xReturned;

        /* SOC init */
    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi to work later on");
    nvs_flash_init();

    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active!)");
    vTaskDelay(RTOS_DELAY_2SEC);


    /*
     * LED & Sensor DHT11
     */
    xReturned = xTaskCreatePinnedToCore(&sensor_task, "sensor_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL, RTOS_TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
    if (xReturned == pdPASS) {
        printf("OK Task sensor_task has been created, and is running right now\n");
    }
    ESP_LOGI(TAG, "app_main() END");
}
