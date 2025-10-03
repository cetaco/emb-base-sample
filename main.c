#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main); // Ativa o módulo de log para este arquivo

#define INTERVAL_MS CONFIG_TIMER_INTERVAL_MS  // OK!

void timer_handler(struct k_timer *dummy) {
    LOG_INF("Hello world com timer de %d ms", INTERVAL_MS);
    LOG_DBG("Hello world com timer de %d ms", INTERVAL_MS);
    LOG_ERR("Hello world com timer de %d ms", INTERVAL_MS);
}


// Declara o timer
K_TIMER_DEFINE(my_timer, timer_handler, NULL);

void main(void)
{
    LOG_INF("Sistema inicializado. Intervalo do timer: %d ms", INTERVAL_MS);

    // Inicia o timer com repetição
    k_timer_start(&my_timer, K_MSEC(INTERVAL_MS), K_MSEC(INTERVAL_MS));
}


