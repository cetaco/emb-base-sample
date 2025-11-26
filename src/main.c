#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

#define INTERVAL_MS CONFIG_TIMER_INTERVAL_MS

static struct k_timer the_timer;

void timer_handler(struct k_timer *dummy) {
    LOG_INF("INFO timer de %d ms", INTERVAL_MS);
    LOG_DBG("DEBUG timer de %d ms", INTERVAL_MS);
    LOG_ERR("ERROR timer de %d ms", INTERVAL_MS);
}

int main(void)
{
    LOG_INF("Sistema inicializado. Intervalo do timer: %d ms", INTERVAL_MS);

    k_timer_init(&the_timer, timer_handler, NULL);
    k_timer_start(&the_timer, K_MSEC(INTERVAL_MS), K_MSEC(INTERVAL_MS));

    while(1){
        k_sleep(K_FOREVER);
    }

    return 0;
}

