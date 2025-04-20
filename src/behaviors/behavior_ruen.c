#define DT_DRV_COMPAT zmk_behavior_ruen

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/lang.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_ruen_config {
    uint8_t lang_id;
};

// Внешняя функция, реализующая переключение языка
extern void zmk_lang_set_active(uint8_t lang_id);

static int on_ruen_pressed(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_ruen_config *config = dev->config;

    uint8_t current_lang = zmk_lang_get_active();
    LOG_INF("Current language state: %d", current_lang);

    zmk_lang_set_active(config->lang_id);

    current_lang = zmk_lang_get_active();
    LOG_INF("New language state: %d", current_lang);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_ruen_released(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_ruen_driver_api = {
    .binding_pressed = on_ruen_pressed,
    .binding_released = on_ruen_released,
};

#define RUEN_INST(n)                                                                          \
    static struct behavior_ruen_config behavior_ruen_config_##n = {                           \
        .lang_id = DT_INST_PROP(n, lang_id),                                                  \
    };                                                                                        \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_ruen_init, NULL, NULL,                                \
                            &behavior_ruen_config_##n, POST_KERNEL,                           \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_ruen_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RUEN_INST)
