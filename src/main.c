#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/device.h>

#define PWM_PERIOD_uS 50000U

enum led_mode{MODE_BLINK, MODE_PWM};

volatile enum led_mode current_led_mode;

static bool up_direction = true;
static unsigned int pwm_percentage = 0;
static int64_t last_step_up_spw_time = 0;
static uint32_t on_timer = 0;
static const struct gpio_dt_spec buttom = GPIO_DT_SPEC_GET(DT_ALIAS(buttom0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct device* const console_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static struct gpio_callback buttom_cb_data;

void buttom_callback_func(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    if (current_led_mode == MODE_BLINK) current_led_mode = MODE_PWM;
    else current_led_mode = MODE_BLINK;
}

void pwm_cycle(){
    printk("pwm percentage: %d\n", pwm_percentage);
    if (k_uptime_get() - last_step_up_spw_time > CONFIG_FADE_TIME_MS/100){
        
        if (up_direction)pwm_percentage ++;
        else pwm_percentage --;

        last_step_up_spw_time = k_uptime_get();
        if (pwm_percentage < 1 || pwm_percentage > 99){
            up_direction = !up_direction;
        } 
    }

    on_timer = ((PWM_PERIOD_uS * pwm_percentage) / 100);
    
    gpio_pin_set_dt(&led, 1);
	k_usleep(on_timer);
	gpio_pin_set_dt(&led, 0);
    k_usleep(PWM_PERIOD_uS - on_timer);
    
}

void blink_cycle(){
    static bool led_is_on = true;

	gpio_pin_toggle_dt(&led);

	led_is_on = !led_is_on;

    if (led_is_on) printk("BLINKING: led on\r\n");
    else printk("BLINKING: led off\r\n");

	k_msleep(CONFIG_BLINK_TIME_MS);
}

int main(void)
{
    unsigned char c;

    if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&buttom))
    {
        return 0;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT);
   
    gpio_pin_configure_dt(&buttom, GPIO_INPUT | GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&buttom_cb_data, buttom_callback_func, BIT(buttom.pin));
    gpio_add_callback(buttom.port, &buttom_cb_data);
    gpio_pin_interrupt_configure_dt(&buttom, GPIO_INT_EDGE_TO_ACTIVE);

    while(1){
        if (!uart_poll_in(console_dev, &c) && (c == '\n' || c == '\r'))
        {
            buttom_callback_func(buttom.port, &buttom_cb_data, BIT(buttom.pin));
        }

        if (current_led_mode == MODE_PWM)
        {
            pwm_cycle();
        }
        else
        {
            blink_cycle();
        }
    }
    return 0;
}
