/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_auto_layer

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>

#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/modifiers_state_changed.h>
#include <zmk/keys.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>

// backport zmk_key_param from #1742. Remove this once merged.
#ifndef ZMK_KEY_PARAM_DECODE
struct zmk_key_param {
    zmk_mod_flags_t modifiers;
    uint8_t page;
    uint16_t id;
};

#define ZMK_KEY_PARAM_DECODE(param)                                                                \
    (struct zmk_key_param) {                                                                       \
        .modifiers = SELECT_MODS(param), .page = ZMK_HID_USAGE_PAGE(param),                        \
        .id = ZMK_HID_USAGE_ID(param),                                                             \
    }
#endif // end backport

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct key_list {
    size_t size;
    struct zmk_key_param keys[];
};

struct behavior_auto_layer_config {
    const struct key_list *continue_keys;
    uint8_t layers;
    bool ignore_alphas;
    bool ignore_numbers;
    bool ignore_modifiers;
};

struct behavior_auto_layer_data {
    bool active;
};

static void activate_auto_layer(const struct device *dev) {
    struct behavior_auto_layer_data *data = dev->data;
    const struct behavior_auto_layer_config *config = dev->config;

// We check whether the track-momentary-layers patch is enabled, because
// zmk_keymap_layer_activate takes two arguments if it is.
#if IS_ENABLED(CONFIG_ZMK_TRACK_MOMENTARY_LAYERS)
    zmk_keymap_layer_activate(config->layers, false);
#else
    zmk_keymap_layer_activate(config->layers);
#endif
    data->active = true;
}

static void deactivate_auto_layer(const struct device *dev) {
    struct behavior_auto_layer_data *data = dev->data;
    const struct behavior_auto_layer_config *config = dev->config;

    zmk_keymap_layer_deactivate(config->layers);
    data->active = false;
}

static int on_auto_layer_binding_pressed(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_auto_layer_data *data = dev->data;

    if (data->active) {
        deactivate_auto_layer(dev);
    } else {
        activate_auto_layer(dev);
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_auto_layer_binding_released(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_auto_layer_driver_api = {
    .binding_pressed = on_auto_layer_binding_pressed,
    .binding_released = on_auto_layer_binding_released,
};

static int auto_layer_keycode_state_changed_listener(const zmk_event_t *eh);

ZMK_LISTENER(behavior_auto_layer, auto_layer_keycode_state_changed_listener);
ZMK_SUBSCRIPTION(behavior_auto_layer, zmk_keycode_state_changed);

#define DEVICE_COUNT DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT)
#define DEVICE_INST(n, _) DEVICE_DT_GET(DT_DRV_INST(n))

static const struct device *devs[] = {LISTIFY(DEVICE_COUNT, DEVICE_INST, (, ))};

static bool key_list_contains(const struct key_list *list, uint16_t usage_page, zmk_key_t usage_id,
                              zmk_mod_flags_t modifiers) {
    for (int i = 0; i < list->size; i++) {
        const struct zmk_key_param *key = &list->keys[i];

        if (key->page == usage_page && key->id == usage_id &&
            (key->modifiers & modifiers) == key->modifiers) {
            return true;
        }
    }

    return false;
}

static bool auto_layer_is_alpha(uint16_t usage_page, zmk_key_t usage_id) {
    if (usage_page != HID_USAGE_KEY) {
        return false;
    }

    return usage_id >= HID_USAGE_KEY_KEYBOARD_A && usage_id <= HID_USAGE_KEY_KEYBOARD_Z;
}

static bool auto_layer_is_numeric(uint16_t usage_page, zmk_key_t usage_id) {
    if (usage_page != HID_USAGE_KEY) {
        return false;
    }

    return ((usage_id >= HID_USAGE_KEY_KEYBOARD_1_AND_EXCLAMATION &&
             usage_id <= HID_USAGE_KEY_KEYBOARD_0_AND_RIGHT_PARENTHESIS) ||
            (usage_id >= HID_USAGE_KEY_KEYPAD_1_AND_END &&
             usage_id <= HID_USAGE_KEY_KEYPAD_0_AND_INSERT)) ||
           usage_id == HID_USAGE_KEY_KEYPAD_00 || usage_id == HID_USAGE_KEY_KEYPAD_000;
}

static bool auto_layer_should_continue(const struct behavior_auto_layer_config *config,
                                       struct zmk_keycode_state_changed *ev) {
    // Alpha keys do not deactivate the layer if ignore_numbers is set.
    if (config->ignore_alphas && auto_layer_is_alpha(ev->usage_page, ev->keycode)) {
        return true;
    }

    // Number keys do not deactivate the layer if ignore_numbers is set.
    if (config->ignore_numbers && auto_layer_is_numeric(ev->usage_page, ev->keycode)) {
        return true;
    }

    // Modifiers do not deactivate the layer if ignore_modifiers is set.
    if (config->ignore_modifiers && is_mod(ev->usage_page, ev->keycode)) {
        return true;
    }

    zmk_mod_flags_t modifiers = ev->implicit_modifiers | zmk_hid_get_explicit_mods();

    return key_list_contains(config->continue_keys, ev->usage_page, ev->keycode, modifiers);
}

static int auto_layer_keycode_state_changed_listener(const zmk_event_t *eh) {
    struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev == NULL || !ev->state) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    for (int i = 0; i < ARRAY_SIZE(devs); i++) {
        const struct device *dev = devs[i];
        const struct behavior_auto_layer_data *data = dev->data;
        if (!data->active) {
            continue;
        }

        const struct behavior_auto_layer_config *config = dev->config;

        if (!auto_layer_should_continue(config, ev)) {
            LOG_DBG("Deactivating auto_layer for 0x%02X - 0x%02X", ev->usage_page, ev->keycode);
            deactivate_auto_layer(dev);
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

static int behavior_auto_layer_init(const struct device *dev) { return 0; }

#define KEY_LIST_ITEM(i, n, prop) ZMK_KEY_PARAM_DECODE(DT_INST_PROP_BY_IDX(n, prop, i))

#define PROP_KEY_LIST(n, prop)                                                                     \
    COND_CODE_1(DT_NODE_HAS_PROP(DT_DRV_INST(n), prop),                                            \
                ({                                                                                 \
                    .size = DT_INST_PROP_LEN(n, prop),                                             \
                    .keys = {LISTIFY(DT_INST_PROP_LEN(n, prop), KEY_LIST_ITEM, (, ), n, prop)},    \
                }),                                                                                \
                ({.size = 0}))

#define KP_INST(n)                                                                                 \
    static const struct key_list auto_layer_continue_list_##n = PROP_KEY_LIST(n, continue_list);   \
                                                                                                   \
    static struct behavior_auto_layer_data behavior_auto_layer_data_##n = {.active = false};       \
    static struct behavior_auto_layer_config behavior_auto_layer_config_##n = {                    \
        .layers = DT_INST_PROP(n, layers),                                                         \
        .continue_keys = &auto_layer_continue_list_##n,                                            \
        .ignore_alphas = DT_INST_PROP(n, ignore_alphas),                                           \
        .ignore_numbers = DT_INST_PROP(n, ignore_numbers),                                         \
        .ignore_modifiers = DT_INST_PROP(n, ignore_modifiers),                                     \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_auto_layer_init, NULL, &behavior_auto_layer_data_##n,      \
                            &behavior_auto_layer_config_##n, POST_KERNEL,                          \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_auto_layer_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)
