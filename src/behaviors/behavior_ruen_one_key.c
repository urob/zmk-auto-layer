#define DT_DRV_COMPAT zmk_behavior_ruen_one_key

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/hid.h>
#include <zmk/lang.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/endpoints.h>
#include <zephyr/kernel.h>

struct ruen_one_key_config {
    uint32_t to_en;
    uint32_t to_ru;
};

static int on_ruen_one_key_pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_ruen_one_key_released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    bool is_eng = zmk_lang_get_state();
    bool need = binding->param1 != 0;
    uint32_t encoded = binding->param2;
    if (need == is_eng) {
        raise_zmk_keycode_state_changed_from_encoded(encoded, true, event.timestamp);
        k_msleep(5);
        raise_zmk_keycode_state_changed_from_encoded(encoded, false, event.timestamp + 5);
    } else {
        const struct ruen_one_key_config *config = binding->behavior_dev->config;
        uint32_t encoded1 = need ? config->to_en : config->to_ru;
        uint32_t encoded2 = need ? config->to_ru : config->to_en;
        zmk_hid_keyboard_clear();
        zmk_endpoints_send_report(HID_USAGE_KEY);
        zmk_lang_set_state(need);
        raise_zmk_keycode_state_changed_from_encoded(encoded1, true, event.timestamp);
        k_msleep(10);
        raise_zmk_keycode_state_changed_from_encoded(encoded1, false, event.timestamp + 10);
        k_msleep(50);
        raise_zmk_keycode_state_changed_from_encoded(encoded, true, event.timestamp + 60);
        k_msleep(5);
        raise_zmk_keycode_state_changed_from_encoded(encoded, false, event.timestamp + 65);
        k_msleep(5);
        zmk_hid_keyboard_clear();
        zmk_endpoints_send_report(HID_USAGE_KEY);
        zmk_lang_set_state(!need);
        raise_zmk_keycode_state_changed_from_encoded(encoded2, true, event.timestamp + 70);
        k_msleep(10);
        raise_zmk_keycode_state_changed_from_encoded(encoded2, false, event.timestamp + 80);
        k_msleep(50);
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_ruen_one_key_driver_api = {
    .binding_pressed = on_ruen_one_key_pressed,
    .binding_released = on_ruen_one_key_released,
};

#define RUEN_ONE_KEY_INST(n)                                                                    \
    static const struct ruen_one_key_config ruen_one_key_config_##n = {                         \
        .to_en = DT_INST_PROP(n, to_en),                               \
        .to_ru = DT_INST_PROP(n, to_ru),                               \
    };                                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &ruen_one_key_config_##n, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_ruen_one_key_driver_api)

DT_INST_FOREACH_STATUS_OKAY(RUEN_ONE_KEY_INST)
