#pragma once

#include <stdbool.h>

bool zmk_lang_get_state(void);
bool zmk_lang_get_macos(void);
void zmk_lang_set_state(bool state);
void zmk_lang_set_macos(bool state);
