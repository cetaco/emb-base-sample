#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>

LOG_MODULE_REGISTER(app);

#define THREAD_PRIORITY 3
#define THREAD_STACK_SIZE 1024

// define message queue, input, output

typedef enum {
    TEMPERATURE,
    HUMIDITY
} s_type;

typedef struct {
    int8_t data;
    s_type sensor_type;
} sensor;

K_MSGQ_DEFINE(input_msgq, sizeof(sensor), 16, 4);
K_MSGQ_DEFINE(output_msgq, sizeof(sensor), 16, 4);

sensor get_sensor_data(s_type sensor_type){
    sensor read_sensor;

    if (sensor_type == TEMPERATURE){
        read_sensor.sensor_type = TEMPERATURE;
        read_sensor.data = (sys_rand32_get() % 30) + 10;
        /*gera temperaturas entre 10 e 39*/
    }
    else if (sensor_type == HUMIDITY){
        read_sensor.sensor_type = HUMIDITY;
        read_sensor.data = (sys_rand32_get() % 50 + 30);
        /*gera humidade entre 30 e 79*/
    }
    return read_sensor;
}


void producer(void *arg1, void *arg2, void *arg3){
    

    while(1){
        sensor humidity_sensor;
        sensor temperature_sensor;

        humidity_sensor = get_sensor_data(HUMIDITY);
        temperature_sensor = get_sensor_data(TEMPERATURE);
    
        LOG_INF("LEITURAS-> humidade: %d | temperatura: %d", humidity_sensor.data, temperature_sensor.data);
        k_msgq_put(&input_msgq, &humidity_sensor, K_NO_WAIT);
        k_msgq_put(&input_msgq, &temperature_sensor, K_NO_WAIT);
        k_msleep(1000);
    }
}

void filter(void *arg1, void *arg2, void *arg3){
    sensor raw_sensor_reads;
    bool sensor_ok = false;
    
    while (1)
    {
        sensor_ok = true;
        if (k_msgq_get(&input_msgq, &raw_sensor_reads, K_MSEC(10)) == 0) {

            if (raw_sensor_reads.sensor_type == HUMIDITY){
                if (raw_sensor_reads.data < 40 || raw_sensor_reads.data > 70){
                    sensor_ok = false;
                    LOG_ERR("Humidade invalida: %d", raw_sensor_reads.data);
                }
            }
                
            else if (raw_sensor_reads.sensor_type == TEMPERATURE){
                if (raw_sensor_reads.data < 18 || raw_sensor_reads.data > 30){
                    sensor_ok = false;
                    LOG_ERR("Temperatura invalida: %d", raw_sensor_reads.data);
                }
            }

            if (sensor_ok == true){
                k_msgq_put(&output_msgq, &raw_sensor_reads, K_NO_WAIT);
            }
        }
    } 
}
void consumer(void *arg1, void *arg2, void *arg3){
    sensor sensor_read;
    while (1)
    {
        if (k_msgq_get(&output_msgq, &sensor_read, K_MSEC(10)) == 0) {
            printk("------------>DISPLAY  %s: %d<-----------\n", sensor_read.sensor_type == HUMIDITY ? "HUMIDADE" : "TEMPERATURA", sensor_read.data);
        }
    }
}

K_THREAD_DEFINE(producer_tid, THREAD_STACK_SIZE, producer, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(filter_tid, THREAD_STACK_SIZE, filter, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(consumer_tid, THREAD_STACK_SIZE, consumer, NULL, NULL, NULL, THREAD_PRIORITY, 0, 0);
