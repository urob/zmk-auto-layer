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
#endif // ZMK_KEY_PARAM_DECODE

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define ZMK_BHV_AUTO_LAYER_MAX_ACTIVE 10

struct key_list {
    size_t size;
    struct zmk_key_param keys[];
};

struct behavior_auto_layer_config {
    const struct key_list *continue_keys;
    bool ignore_alphas;
    bool ignore_numbers;
    bool ignore_modifiers;
};

struct active_auto_layer {
    bool is_active;
    uint8_t layer;
    const struct behavior_auto_layer_config *config;
};

struct active_auto_layer active_auto_layers[ZMK_BHV_AUTO_LAYER_MAX_ACTIVE] = {};

static struct active_auto_layer *find_auto_layers(uint8_t layer) {
    for (int i = 0; i < ZMK_BHV_AUTO_LAYER_MAX_ACTIVE; i++) {
        if (active_auto_layers[i].layer == layer && active_auto_layers[i].is_active) {
            return &active_auto_layers[i];
        }
    }
    return NULL;
}

static int new_auto_layer(uint8_t layer, const struct behavior_auto_layer_config *config,
                          struct active_auto_layer **auto_layer) {
    for (int i = 0; i < ZMK_BHV_AUTO_LAYER_MAX_ACTIVE; i++) {
        struct active_auto_layer *const ref_auto_layer = &active_auto_layers[i];
        if (!ref_auto_layer->is_active) {
            ref_auto_layer->is_active = true;
            ref_auto_layer->layer = layer;
            ref_auto_layer->config = config;
            *auto_layer = ref_auto_layer;
            return 0;
        }
    }
    return -ENOMEM;
}

static void clear_auto_layer(struct active_auto_layer *auto_layer) {
    auto_layer->is_active = false;
}

static void activate_auto_layer(struct active_auto_layer *auto_layer) {
#if IS_ENABLED(CONFIG_ZMK_TRACK_MOMENTARY_LAYERS)
    // If patch is enabled, second argument signals whether layer change momentary or not
    zmk_keymap_layer_activate(auto_layer->layer, false);
#else
    zmk_keymap_layer_activate(auto_layer->layer);
#endif // IS_ENABLED(CONFIG_ZMK_TRACK_MOMENTARY_LAYERS)
}

static void deactivate_auto_layer(struct active_auto_layer *auto_layer) {
    zmk_keymap_layer_deactivate(auto_layer->layer);
    auto_layer->is_active = false;
}

static int on_auto_layer_binding_pressed(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_auto_layer_config *cfg = dev->config;

    struct active_auto_layer *auto_layer;
    auto_layer = find_auto_layers(binding->param1);
    if (auto_layer == NULL) {
        if (new_auto_layer(binding->param1, cfg, &auto_layer) == -ENOMEM) {
            LOG_ERR("Unable to create new auto_layer. Insufficient space in active_auto_layers[].");
            return ZMK_BEHAVIOR_OPAQUE;
        }
        activate_auto_layer(auto_layer);
        LOG_DBG("%d created new auto_layer", event.position);
    } else {
        deactivate_auto_layer(auto_layer);
        LOG_DBG("%d deactivated auto_layer", event.position);
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

    for (int i = 0; i < ZMK_BHV_AUTO_LAYER_MAX_ACTIVE; i++) {
        struct active_auto_layer *auto_layer = &active_auto_layers[i];
        if (!auto_layer->is_active) {
            continue;
        }

        if (!auto_layer_should_continue(auto_layer->config, ev)) {
            LOG_DBG("Deactivating auto_layer for 0x%02X - 0x%02X", ev->usage_page, ev->keycode);
            deactivate_auto_layer(auto_layer);
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

static int behavior_auto_layer_init(const struct device *dev) {
    static bool init_first_run = true;
    if (init_first_run) {
        for (int i = 0; i < ZMK_BHV_AUTO_LAYER_MAX_ACTIVE; i++) {
            clear_auto_layer(&active_auto_layers[i]);
        }
    }
    init_first_run = false;
    return 0;
}

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
    static struct behavior_auto_layer_config behavior_auto_layer_config_##n = {                    \
        .continue_keys = &auto_layer_continue_list_##n,                                            \
        .ignore_alphas = DT_INST_PROP(n, ignore_alphas),                                           \
        .ignore_numbers = DT_INST_PROP(n, ignore_numbers),                                         \
        .ignore_modifiers = DT_INST_PROP(n, ignore_modifiers),                                     \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_auto_layer_init, NULL, NULL,                               \
                            &behavior_auto_layer_config_##n, POST_KERNEL,                          \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_auto_layer_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KP_INST)
