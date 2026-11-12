#include "edbus.h"
#include "pico/stdlib.h"
#include <stdio.h>

void interchange_edbus_message_handler(const uint32_t data[static 7]) {}

void interchange_edbus_event_handler(edbus_event_t event) {}

int main() {
  stdio_init_all();

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);

  sleep_ms(5000);

  edbus_config_t c = edbus_get_default_config(
      6, 7, interchange_edbus_message_handler, interchange_edbus_event_handler);
  edbus_init(&c);

  edbus_message_t m = {0xFFFFFFFF, 0xAAAAAAAA, 0x00000000, 0x00100100,
                       0xABCDEF01, 0xA0000000, 0x0000000A};

  while (true) {
    edbus_insert_crc_send_blocking(m);

    int input = getchar_timeout_us(0);
    if (input == '2') {
      printf("Setting drive strength to 2mA\n");
      gpio_set_drive_strength(6, GPIO_DRIVE_STRENGTH_2MA);
      gpio_set_drive_strength(7, GPIO_DRIVE_STRENGTH_2MA);
    } else if (input == '4') {
      printf("Setting drive strength to 4mA\n");
      gpio_set_drive_strength(6, GPIO_DRIVE_STRENGTH_4MA);
      gpio_set_drive_strength(7, GPIO_DRIVE_STRENGTH_4MA);
    } else if (input == '8') {
      printf("Setting drive strength to 8mA\n");
      gpio_set_drive_strength(6, GPIO_DRIVE_STRENGTH_8MA);
      gpio_set_drive_strength(7, GPIO_DRIVE_STRENGTH_8MA);
    } else if (input == '1') {
      printf("Setting drive strength to 12mA\n");
      gpio_set_drive_strength(6, GPIO_DRIVE_STRENGTH_12MA);
      gpio_set_drive_strength(7, GPIO_DRIVE_STRENGTH_12MA);
    } else if (input == 'a') {
      printf("Setting slew rate to fast\n");
      gpio_set_slew_rate(6, GPIO_SLEW_RATE_FAST);
      gpio_set_slew_rate(7, GPIO_SLEW_RATE_FAST);
    } else if (input == 'b') {
      printf("Setting slew rate to slow\n");
      gpio_set_slew_rate(6, GPIO_SLEW_RATE_SLOW);
      gpio_set_slew_rate(7, GPIO_SLEW_RATE_SLOW);
    }

    tight_loop_contents();
  }
}
