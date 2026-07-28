#include "edbus.h"
#include "pico/stdlib.h"
#include <stdio.h>

void consumer(uint32_t identifier, const uint8_t data[24]) {
  gpio_put(PICO_DEFAULT_LED_PIN, true);
}

int main() {
  stdio_init_all();

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);

  sleep_ms(5000);

  edbus_config_t c = {
      pio0, 0, 1, 0, 0, 0, 1, 6, 7, consumer,
  };
  edbus_configure(&c);
  edbus_enable();

  edbus_message_t msg;
  uint8_t data[24] = {0xFF, 0xAA, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                      0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  edbus_construct_message(0xFFFF0000, data, msg);
  printf("Sending message.\n");
  edbus_send_message(msg);
  printf("Sent.\n");

  while (true) {
    tight_loop_contents();
  }
}
