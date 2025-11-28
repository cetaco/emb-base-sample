#include <zephyr/kernel.h>

/*data utilized by the display*/
typedef struct {
    uint16_t speed;
    uint8_t axis_count;
    char plate[10];
}display_data;
K_MSGQ_DEFINE(display_msgq, sizeof(display_data), 8, 4);

enum {
    RED = 0,
    YELLOW,
    GREEN
}Colors;

void display_thread_start(void *arg_1, void *arg_2, void *arg_3){
    display_data display;
    int ret;
    int color;

    while(1){
        k_msleep(10);
        /*wait to get some message on msgq*/
        ret = k_msgq_get(&display_msgq, &display, K_FOREVER);

        if(ret != 0){
            printk("Error on display msgq: %d\n\r", ret);
        }

        /*check the speed to assing the correct color on the display*/
        color = GREEN;
        if (display.axis_count <= 2){
            if (display.speed > CONFIG_RADAR_SPEED_LIMIT_LIGHT_KMH){
                color = RED;
            }
            else if (display.speed > ((CONFIG_RADAR_SPEED_LIMIT_LIGHT_KMH * CONFIG_RADAR_WARNING_THRESHOLD_PERCENT)
                                        /100)){
                color = YELLOW;
            }
        }
        else{
            if (display.speed > CONFIG_RADAR_SPEED_LIMIT_HEAVY_KMH){
                color = RED;
            }
            else if (display.speed > ((CONFIG_RADAR_SPEED_LIMIT_HEAVY_KMH * 100)
                                        /CONFIG_RADAR_WARNING_THRESHOLD_PERCENT)){
                color = YELLOW;
            }
        }
        char *terminal_color_code;
        if (color == RED) terminal_color_code = "\x1b[91m";
        if (color == YELLOW) terminal_color_code = "\x1b[33m";
        if (color == GREEN) terminal_color_code = "\x1b[32m";

        /*print the display with correct color*/
        printk("%s╔═══════════════════════╗\x1b[0m\n", terminal_color_code);
        printk("%s║   SPEED: %d\t\t║\x1b[0m\n", terminal_color_code, display.speed);
        printk("%s║   AXIS: %d\t\t║\x1b[0m\n", terminal_color_code, display.axis_count);
        printk("%s║   PLATE: %s\t║\x1b[0m\n", terminal_color_code, display.plate);
        printk("%s╚═══════════════════════╝\x1b[0m\n", terminal_color_code);
        
        printk("speed: %d, axix count: %d, plate: %s\n", display.speed, display.axis_count, display.plate);
    }
}