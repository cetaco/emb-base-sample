#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/uart.h>
#include "camera_service.h"
#include "validate_plate.h"
#include "display_thread.c"
#include "speed_sensor_thread.c"

/*define the global thread values*/
#define THREAD_STACK_SIZE 1024
#define THREAD_PRIORITY 3

/*defines the zbus for the camera acess*/
ZBUS_MSG_SUBSCRIBER_DEFINE(msub_camera_evt);
ZBUS_CHAN_ADD_OBS(chan_camera_evt, msub_camera_evt, 3);

/*defines the gpio dt for inputs and device to simulate inputs*/
static const struct gpio_dt_spec mag_1 = GPIO_DT_SPEC_GET(DT_ALIAS(mag_sensor_1), gpios);
static const struct gpio_dt_spec mag_2 = GPIO_DT_SPEC_GET(DT_ALIAS(mag_sensor_2), gpios);
static const struct device* const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

/*thread only for manual test via keyboard
    press '1' to simulate the magnetic sensor 1
    press '2' to simulate the magnetic sensor 2 
*/
void manual_tests_thread_start(void *arg_1, void *arg_2, void *arg_3){
    static struct device dummy;
    static struct gpio_callback dummy_cb;

    unsigned char c;

    while (1) {
        k_usleep(10);
        if (uart_poll_in(console_dev, &c) == -1) c = '0';

		if (c == '1') {
            mag_1_callback_func(&dummy, &dummy_cb, 1 << 6);
        }
        else if (c == '2')  {
            mag_2_callback_func(&dummy, &dummy_cb, 1 << 6);
        }
	}
}

/*main thread*/
int main(void)
{
    /*check if the mag sensors are ready*/
    if (!gpio_is_ready_dt(&mag_1) || !gpio_is_ready_dt(&mag_2)) {
        printk("mag sensors not ready\r\n");
        return -1;
    }
    
    /*configures the mag sensors*/
    gpio_pin_configure_dt(&mag_1, GPIO_INPUT | GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&mag_1_cb_data, mag_1_callback_func, BIT(mag_1.pin));
    gpio_add_callback(mag_1.port, &mag_1_cb_data);
    gpio_pin_interrupt_configure_dt(&mag_1, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_pin_configure_dt(&mag_2, GPIO_INPUT | GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&mag_2_cb_data, mag_2_callback_func, BIT(mag_2.pin));
    gpio_add_callback(mag_2.port, &mag_2_cb_data);
    gpio_pin_interrupt_configure_dt(&mag_2, GPIO_INT_EDGE_TO_ACTIVE);

    int ret;
    const struct zbus_channel *chan;
    speed_sensor_data speed_sensor;
    display_data display;

    while(1){
        k_msleep(500);

        /*wait for the speed sensor produce some data*/
        ret = k_msgq_get(&speed_sensor_msgq, &speed_sensor, K_FOREVER);
        if (ret != 0) {
            printk("Could not get msgq data. Error: %d\n", ret);
            continue;
        }

        /*if the speed sensor produces a a valid data, ask the camera api for the plate*/
        printk("%d, %d\r\n", speed_sensor.axis_count, speed_sensor.speed);
		ret = camera_api_capture(K_FOREVER);
		if (ret) {
			printk("Could not init capture. Error: %d\n", ret);
			continue;
		}

		struct msg_camera_evt rsp;

		ret = zbus_sub_wait_msg(&msub_camera_evt, &chan, &rsp, K_FOREVER);
		if (ret) {
			printk("ERROR: %d\n", ret);
			continue;
		}

        /*if the camera api returns something, check if its a plate*/
		if (rsp.type == MSG_CAMERA_EVT_TYPE_ERROR) {
			printk("Camera service unavailable. Error code %d\n", rsp.error_code);
            strcpy(display.plate, "INVALID");
		} else if (rsp.type == MSG_CAMERA_EVT_TYPE_DATA) {
            /*check if the plate is a valid plate*/
            memcpy(display.plate, rsp.captured_data->plate, strlen(rsp.captured_data->plate) + 1);
            if (!validate_mercosul_plate(display.plate)) strcpy(display.plate, "INVALID");
		}

        /*put the data on a struct and send to display msgq*/
        display.speed = speed_sensor.speed;
        display.axis_count = speed_sensor.axis_count;
        
        k_msgq_put(&display_msgq, &display, K_NO_WAIT);
    }
	
    return 0;
}

K_THREAD_DEFINE(speed_sensor_thread, THREAD_STACK_SIZE, speed_sensor_thread_start, 
                    NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(manual_tests_thread, THREAD_STACK_SIZE, manual_tests_thread_start, 
                    NULL, NULL, NULL, 0, 0, 0);
K_THREAD_DEFINE(display_thread, THREAD_STACK_SIZE, display_thread_start, 
                    NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
        
        