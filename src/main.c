#include "bsp/board.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "keymap.h"
#include "pico/flash.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico_pca9555.h"
#include "quadrature_encoder.pio.h"
#include "ssd1306.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "ws2812.pio.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "squirrel.h"
#include "squirrel_consumer.h"
#include "squirrel_init.h"
#include "squirrel_key.h"
#include "squirrel_keyboard.h"
#include "squirrel_quantum.h"

// send_hid_kbd_codes sends a HID report with the given keycodes to the host.
void send_hid_kbd_codes(uint8_t keycode_assembly[6], uint8_t modifiers) {
  if (!tud_hid_ready()) {
    return;
  };
  // Send the currently active keycodes and modifiers to the host.
  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifiers, keycode_assembly);
}

// Every 10ms, we will send 1 HID report (per device) to the host.
// First, the keyboard. Subsequent reports will be sent in the
// tud_hid_report_complete_cb callback.
void hid_task(void) {
  const uint32_t interval_ms = 10;    // Time between reports
  static uint32_t next_report_ms = 0; // Time of next report

  if (board_millis() - next_report_ms < interval_ms) {
    return; // Not time for a report yet.
  };
  next_report_ms += interval_ms; // Set the time for the next report

  uint8_t modifiers = keyboard_get_modifiers(); // Get the current modifiers.
  // Define an array to store the active keycodes. 6 is the limit for USB HID.
  uint8_t active_keycodes[6] = {0, 0, 0, 0, 0, 0};
  keyboard_get_keycodes(&active_keycodes); // Fill the array with the
  //  keycodes.
  send_hid_kbd_codes(active_keycodes, modifiers); // Send the HID report.
}

// tud_hid_report_complete_cb is invoked when a report is sent to the host.
// report[0] is the report ID of the report just sent.
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report,
                                uint16_t len) {
  (void)instance;
  (void)report;
  (void)len;
  if (report[0] == REPORT_ID_KEYBOARD) {
    // If the keyboard report was just sent, send the consumer report.
    // uint16_t consumer_code = consumer_get_consumer_code();
    //    tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &consumer_code,
    //                   2); // Send the report.
    return;
  }
}

// GET_REPORT callback (host requests data from device).
// Currently unused.
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;
  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  // report_id was the first byte of the buffer, report_id was removed from the
  // buffer before this function was called.

  // Recived data from the host.
  if (report_type == HID_REPORT_TYPE_OUTPUT) {
    // Keyboard type report
    if (report_id == REPORT_ID_KEYBOARD) {
      // Set keyboard LED e.g Capslock, Numlock etc...
      // bufsize should be (at least) 1
      if (bufsize < 1) {
        return;
      }

      uint8_t const kbd_leds = buffer[0];
      if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
        /*put_pixel(urgb_u32(255, 0, 0));*/
      } else if (kbd_leds & KEYBOARD_LED_NUMLOCK) {
        /*put_pixel(urgb_u32(0, 255, 0));*/
      } else if (kbd_leds & KEYBOARD_LED_SCROLLLOCK) {
        /*put_pixel(urgb_u32(0, 0, 255));*/
      } else {
        /*put_pixel(0);*/
      }
    }
  }
}

// 3 address pins are grounded.
// See https://www.nxp.com/docs/en/data-sheet/PCA9555.pdf for more details.
const uint8_t PCA9555_ADDR = 0b0100000;

void i2c_devices_init(void) {
  // Initialize the I2C bus.
  i2c_init(&i2c1_inst, 400000); // 400kHz

  // Configure the I2C pins.
  gpio_set_function(6, GPIO_FUNC_I2C);
  gpio_set_function(7, GPIO_FUNC_I2C);

  // We don't need to lock the I2C mutex here because this function is run
  // before the multicore loop.
  pca9555_configure(&i2c1_inst, PCA9555_ADDR, 0x0000);
}

// outputs_lookup is a lookup table that provides the correct pin outputs from
// the PCA9555 chip for each column of the keyboard.
const uint16_t outputs_lookup[16] = {
    0b0000000010000000, // physical column 1
    0b0000000001000000, // 2
    0b0000000000100000, // 3
    0b0000000000010000, // 4
    0b0000000000000001, // 5
    0b0000000000000010, // 6
    0b0000000000000100, // 7
    0b0000000000001000, // 8
    0b0100000000000000, // 9
    0b0010000000000000, // 10
    0b0001000000000000, // 11
    0b0000100000000000, // 12
    0b0000010000000000, // 13
    0b0000001000000000, // 14
    0b0000000100000000, // 15
    0b1000000000000000, // unused - not connected
};

// debounce checks keys twice over 200us and if the key is still in the same
// position, it counts is as confirmed and runs check_key. This prevents
// chatter. (false key activations)
void debounce(uint8_t column) {
  // Get the state of all keys in the column
  bool r1 = gpio_get(1);
  bool r2 = gpio_get(0);
  bool r3 = gpio_get(29);
  bool r4 = gpio_get(28);
  bool r5 = gpio_get(27);

  bool r1_prev = r1;
  bool r2_prev = r2;
  bool r3_prev = r3;
  bool r4_prev = r4;
  bool r5_prev = r5;

  // Wait for 200us
  sleep_ms(20);

  // Get the state of all keys in the column again.
  r1 = gpio_get(1);
  r2 = gpio_get(0);
  r3 = gpio_get(29);
  r4 = gpio_get(28);
  r5 = gpio_get(27);
  if (r1 | r2 | r3 | r4 | r5) {
    // check_key(0, r1 | r2 | r3 | r4 | r5);
    layers[0].keys[0].pressed(0, 0, layers[0].keys[0].pressed_argument);
    // keyboard_activate_keycode(HID_KEY_A);
    // keyboard_keycodes[HID_KEY_A] = true;
    if (keyboard_keycodes[HID_KEY_A]) {
      gpio_put(17, 0);
      gpio_put(16, 0);
      gpio_put(25, 0);
    }
  } else {
    gpio_put(17, 1);
    gpio_put(16, 1);
    gpio_put(25, 1);
    keyboard_deactivate_keycode(HID_KEY_A);
  }

  // If the key is still in the same state after 20ms, run check_key.
  // Also, if any key is pressed, update the last_interaction time.
  if (r1 == r1_prev) {
    //    check_key(column, r1);
  }
  if (r2 == r2_prev) {
    //    check_key(column + 15, r2);
  }
  if (r3 == r3_prev) {
    //    check_key(column + 30, r3);
  }
  if (r4 == r4_prev) {
    //    check_key(column + 45, r4);
  }
  if (r5 == r5_prev) {
    //    check_key(column + 60, r5);
  }
}

// check_keys loops through all columns and runs a check on each key.
void check_keys() {
  // PCA9555 uses two sets of 8-bit outputs
  // Loop through all columns
  for (uint8_t x = 0; x < 15; x++) {
    uint16_t column_outputs = outputs_lookup[x];
    pca9555_output(&i2c1_inst, PCA9555_ADDR, column_outputs);
    debounce(x);
  }
}

void row_setup(void) {
  // Configure the row pins as inputs with pull-down resistors.
  gpio_init(1);
  gpio_set_dir(1, GPIO_IN);
  gpio_pull_down(1);

  gpio_init(0);
  gpio_set_dir(0, GPIO_IN);
  gpio_pull_down(0);

  gpio_init(29);
  gpio_set_dir(29, GPIO_IN);
  gpio_pull_down(29);

  gpio_init(28);
  gpio_set_dir(28, GPIO_IN);
  gpio_pull_down(28);

  gpio_init(27);
  gpio_set_dir(27, GPIO_IN);
  gpio_pull_down(27);
}

// Core 0 deals with keyboard and USB HID.
void core0_main() {
  while (true) {
    check_keys(); // Check the keys on the keyboard for their states.
    tud_task();   // tinyusb device task.
    hid_task();   // Send HID reports to the host.
  }
}

#define SQUIRREL_KEYCOUNT 1

// The main function, runs initialization.
int main(void) {
  board_init();               // Initialize the pico board
  tud_init(BOARD_TUD_RHPORT); // Initialize the tinyusb device stack
  tusb_init();                // Initialize tinyusb
  squirrel_init();            // Initialize the squirrel keyboard with 75
  //  keys.

  gpio_init(25);
  gpio_set_dir(25, GPIO_OUT);
  gpio_init(16);
  gpio_set_dir(16, GPIO_OUT);
  gpio_init(17);
  gpio_set_dir(17, GPIO_OUT);
  gpio_put(25, 1);
  gpio_put(16, 1);
  gpio_put(17, 1);

  make_keys();        // Initialize the keys on the keyboard
  row_setup();        // Initialize the rows of the keyboard
  i2c_devices_init(); // Initialize the I2C devices

  core0_main();
}
