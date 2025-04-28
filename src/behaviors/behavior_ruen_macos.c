#define DT_DRV_COMPAT zmk_behavior_ruen_macos

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/lang.h>

static int on_ruen_macos_switch_pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_ruen_macos_switch_released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    bool is_macos = binding->param1 != 0;
    if (is_macos != zmk_ruen_get_macos()) {
        zmk_ruen_set_macos(is_macos);
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_ruen_macos_switch_driver_api = {
    .binding_pressed = on_ruen_macos_switch_pressed,
    .binding_released = on_ruen_macos_switch_released,
};

#define RUEN_MACOS_SWITCH_INST(n) BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_ruen_macos_switch_driver_api)
DT_INST_FOREACH_STATUS_OKAY(RUEN_MACOS_SWITCH_INST)
