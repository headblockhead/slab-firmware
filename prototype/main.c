#include "edbus.pio.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdio.h>

int init_tx_program(PIO *pio, uint *sm, uint *offset) {
  bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
      &edbus_tx_program, pio, sm, offset, 6, 2, true);
  hard_assert(success);
  edbus_tx_program_init(*pio, *sm, *offset, 6, 7);
  return 0;
}

int init_rx_program(PIO *pio, uint *sm, uint *offset) {
  bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
      &edbus_rx_program, pio, sm, offset, 6, 2, true);
  hard_assert(success);
  edbus_rx_program_init(*pio, *sm, *offset, 6, 7);
  return 0;
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

  uint16_t wip;
  uint32_t expected_value;
  uint32_t actual_value;
  bool pass = true;
  while (wip != UINT16_MAX) {
    expected_value = 0 | (wip << 16);
    pio_sm_put_blocking(tx_pio, tx_sm, expected_value);
    actual_value = pio_sm_get_blocking(rx_pio, rx_sm);
    if (expected_value != actual_value) {
      printf("FAILED, expected 0x%08x, got 0x%08x.\n", expected_value,
             actual_value);
      pass = false;
      break;
    }
    wip++;
  }

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  if (pass) {
    printf("PASSED!!!\n");
  }
  gpio_put(PICO_DEFAULT_LED_PIN, pass);

  return 0;
}
