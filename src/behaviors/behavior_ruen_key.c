#define DT_DRV_COMPAT zmk_behavior_ruen_key

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/lang.h>
#include <zmk/events/keycode_state_changed.h>

static int on_ruen_key_pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    bool is_eng = zmk_ruen_get_eng();
    uint32_t encoded = is_eng ? binding->param1 : binding->param2;
    raise_zmk_keycode_state_changed_from_encoded(encoded, true, event.timestamp);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_ruen_key_released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    bool is_eng = zmk_ruen_get_eng();
    uint32_t encoded = is_eng ? binding->param1 : binding->param2;
    raise_zmk_keycode_state_changed_from_encoded(encoded, false, event.timestamp);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_ruen_key_driver_api = {
    .binding_pressed = on_ruen_key_pressed,
    .binding_released = on_ruen_key_released,
};

#define RUEN_KEY_INST(n) BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_ruen_key_driver_api)
DT_INST_FOREACH_STATUS_OKAY(RUEN_KEY_INST)
