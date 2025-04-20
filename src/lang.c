#include <zmk/lang.h>

static uint8_t active_lang_id = 0;

void zmk_lang_set_active(uint8_t lang_id) {
    active_lang_id = lang_id;
}

uint8_t zmk_lang_get_active(void) {
    return active_lang_id;
}
