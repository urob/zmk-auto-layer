#include <zmk/lang.h>
#include <stdbool.h>

static bool is_eng = true;

void zmk_lang_set_state(bool state) {
    is_eng = state;
}

bool zmk_lang_get_state(void) {
    return is_eng;
}
