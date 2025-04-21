#define DT_DRV_COMPAT zmk_behavior_ruen_switch

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/lang.h>

static int on_ruen_switch_pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_ruen_switch_released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    uint32_t code = binding->param2;
    raise_zmk_keycode_state_changed_from_encoded(code, true, event.timestamp);
    k_msleep(10);
    raise_zmk_keycode_state_changed_from_encoded(code, false, event.timestamp + 10);
    k_msleep(50);
    bool is_eng = binding->param1 != 0;
    zmk_lang_set_state(is_eng);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_ruen_switch_driver_api = {
    .binding_pressed = on_ruen_switch_pressed,
    .binding_released = on_ruen_switch_released,
};

#define RUEN_SWITCH_INST(n) BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_ruen_switch_driver_api)
DT_INST_FOREACH_STATUS_OKAY(RUEN_SWITCH_INST)
