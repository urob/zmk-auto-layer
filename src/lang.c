#include <zephyr/init.h>
#include <zephyr/device.h>
#include <zephyr/settings/settings.h>
#include <zmk/lang.h>
#include <stdbool.h>
#include <string.h>

static bool is_eng = true;
static bool is_mac = false;

bool zmk_lang_get_state(void) {
    return is_eng;
}

bool zmk_lang_get_mac(void) {
    return is_mac;
}

void zmk_lang_set_state(bool state) {
    is_eng = state;
}

void zmk_lang_set_mac(bool state) {
    is_mac = state;
    settings_save_one("ruen/is_mac", &is_mac, sizeof(is_mac));
}

static int ruen_settings_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg) {
    if (strcmp(key, "is_mac") == 0 && len == sizeof(is_mac)) {
        read_cb(cb_arg, &is_mac, len);
    }
    return 0;
}

static struct settings_handler ruen_settings_handler = {
    .name = "ruen",
    .h_set = ruen_settings_set,
};

static int ruen_init(const struct device *dev) {
    ARG_UNUSED(dev);
    settings_subsys_init();
    settings_register(&ruen_settings_handler);
    settings_load_subtree("ruen");
    return 0;
}
SYS_INIT(ruen_init, POST_KERNEL, CONFIG_SETTINGS_INIT_PRIORITY);
