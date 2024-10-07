#include "keymap.h"
#include "tusb.h"
#include <stdlib.h>

#include "squirrel.h"
#include "squirrel_keymap.h"
#include "squirrel_quantum.h"

void make_keys(void) {
  layers[0].active = true;
  static uint8_t key = HID_KEY_A;
  layers[0].keys[0].pressed = keyboard_press;
  layers[0].keys[0].pressed_argument = &key;
};
