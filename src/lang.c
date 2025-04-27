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
    if (!strcmp(key, "is_mac") && len == sizeof(is_mac)) {
        read_cb(cb_arg, &is_mac, len);
    }
    return 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(
    ruen,               /* module name */
    "ruen",             /* subtree */
    NULL,               /* export_cb */
    ruen_settings_set,  /* set_cb */
    NULL,               /* commit_cb */
    NULL                /* export_item_cb */
);
