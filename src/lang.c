#include <zephyr/settings/settings.h>
#include <zmk/lang.h>
#include <stdbool.h>
#include <string.h>

static bool is_eng = true;
static bool is_macos = false;

bool zmk_ruen_get_eng(void) {
    return is_eng;
}

bool zmk_ruen_get_macos(void) {
    return is_macos;
}

void zmk_ruen_set_eng(bool state) {
    is_eng = state;
}

void zmk_ruen_set_macos(bool state) {
    is_macos = state;
    settings_save_one("ruen/is_macos", &is_macos, sizeof(is_macos));
}

static int ruen_settings_set(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg) {
    if (!strcmp(key, "is_macos") && len == sizeof(is_macos)) {
        read_cb(cb_arg, &is_macos, len);
    }
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(ruen, "ruen", NULL, ruen_settings_set, NULL, NULL);
