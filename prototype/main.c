#include "edbus.pio.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdio.h>

void init_tx_program(PIO *pio, uint *sm, uint *offset) {
  bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
      &edbus_tx_program, pio, sm, offset, 6, 2, true);
  hard_assert(success);
  edbus_tx_program_init(*pio, *sm, *offset, 6, 7);
}

void init_rx_program(PIO *pio, uint *sm, uint *offset) {
  bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
      &edbus_rx_program, pio, sm, offset, 6, 2, true);
  hard_assert(success);
  edbus_rx_program_init(*pio, *sm, *offset, 6, 7);
}

void send(PIO *tx_pio, uint *tx_sm, PIO *rx_pio, uint *rx_sm) {
  while (1) {
    pio_sm_clear_fifos(*tx_pio, *tx_sm);
    pio_sm_clear_fifos(*rx_pio, *rx_sm);
    uint32_t value_to_send = 0;
    uint32_t sent_value = 0;
    bool pass = true;
    while (value_to_send <= UINT16_MAX) {
      pio_sm_put_blocking(*tx_pio, *tx_sm, value_to_send);
      sent_value = pio_sm_get_blocking(*rx_pio, *rx_sm);
      if (sent_value != value_to_send) {
        printf("FAILED, sent 0x%08x, got 0x%08x.\n", sent_value, value_to_send);
        pass = false;
        break;
      }
      value_to_send++;
    }
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    if (pass) {
      printf("SEND PASSED!!!\n");
    }
    gpio_put(PICO_DEFAULT_LED_PIN, pass);
    sleep_ms(5000);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(100);
    printf("Restarting...\n");
  }
}

void receive(PIO *tx_pio, uint *tx_sm, PIO *rx_pio, uint *rx_sm) {
  while (1) {
    pio_sm_clear_fifos(*rx_pio, *rx_sm);
    uint32_t expected_value = 0;
    uint32_t actual_value = 0;
    bool pass = true;
    while (expected_value <= UINT16_MAX) {
      actual_value = pio_sm_get_blocking(*rx_pio, *rx_sm);
      if (actual_value != expected_value) {
        printf("FAILED, expected 0x%08x, got 0x%08x.\n", expected_value,
               actual_value);
        pass = false;
        break;
      }
      expected_value++;
    }
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    if (pass) {
      printf("RECEIVE PASSED!!!\n");
    }
    gpio_put(PICO_DEFAULT_LED_PIN, pass);
    sleep_ms(1000);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(100);
    printf("Restarting...\n");
  };
}

int main() {
  stdio_init_all();

  PIO tx_pio;
  uint tx_sm;
  uint tx_offset;

  PIO rx_pio;
  uint rx_sm;
  uint rx_offset;

  init_rx_program(&rx_pio, &rx_sm, &rx_offset);
  init_tx_program(&tx_pio, &tx_sm, &tx_offset);

  /*send(&tx_pio, &tx_sm, &rx_pio, &rx_sm);*/
  receive(&tx_pio, &tx_sm, &rx_pio, &rx_sm);

  return 0;
}
