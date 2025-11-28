#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/*defines the gpio callback*/
static struct gpio_callback mag_1_cb_data;
static struct gpio_callback mag_2_cb_data;

/*defines the semaphore*/
K_SEM_DEFINE(speed_sem, 0, 1);   

/*data utilized by the sensor mensage queue*/
typedef struct {
    uint16_t speed;
    uint8_t axis_count;
} speed_sensor_data;
K_MSGQ_DEFINE(speed_sensor_msgq, sizeof(speed_sensor_data), 16, 4);

/*structure utilized as a data storage to the state machine*/
struct {
    uint8_t axis_count;
    uint16_t speed;
    int64_t last_sleed_measure_time;
}speed_measure = {0};

void reset_speed_sensor(){
    speed_measure.speed = 0;
    speed_measure.axis_count = 0;
    speed_measure.last_sleed_measure_time = 0;
}

void mag_1_callback_func(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    /*starts the state machine and count axis*/
    if (speed_measure.axis_count == 0)
        speed_measure.last_sleed_measure_time = k_uptime_get();
    speed_measure.axis_count += 1;
}

void mag_2_callback_func(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    /*update the state machine calculating the speed and resets the wait time*/
    if (speed_measure.speed == 0 && speed_measure.axis_count >= 1){
        speed_measure.speed = CONFIG_RADAR_SENSOR_DISTANCE_MM/(k_uptime_get() - speed_measure.last_sleed_measure_time)* 3.6f;
        /*give a green sim to the sensor thread*/
        k_sem_give(&speed_sem);
    }
    else if(speed_measure.speed != 0){
        speed_measure.last_sleed_measure_time = k_uptime_get();
    }
}

void speed_sensor_thread_start(void *arg_1, void *arg_2, void *arg_3)
{
    while(1){
        /*stop the thread waiting for a green sem*/
        if (speed_measure.axis_count == 0){
            k_sem_take(&speed_sem, K_FOREVER);
        }

        /*check if readed speed = 0*/
        if (speed_measure.speed == 0){
            if (k_uptime_get() - speed_measure.last_sleed_measure_time > 1000) {
                printk("speed not readed\n\r");
                reset_speed_sensor();
            }
        }
        /*wait a timer defined by (distance * 8 /speed) to make sure the entire veichle has passed
        and there is no other one*/
        else{
            if (k_uptime_get() - speed_measure.last_sleed_measure_time > (CONFIG_RADAR_SENSOR_DISTANCE_MM * 8) / speed_measure.speed) {
                speed_sensor_data speed_sensor = {.axis_count = speed_measure.axis_count, 
                                                  .speed = speed_measure.speed};

                k_msgq_put(&speed_sensor_msgq, &speed_sensor, K_NO_WAIT);
                reset_speed_sensor();
            }
        }
    }
}