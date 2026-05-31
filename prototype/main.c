#include "edbus.pio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include <stdio.h>

int main() {
  PIO pio;
  uint sm;
  uint offset;

  bool success = pio_claim_free_sm_and_add_program_for_gpio_range(
      &edbus_program, &pio, &sm, &offset, 25, 1, true);
  hard_assert(success);

  edbus_program_init(pio, sm, offset, 25);

  while (1) {
    pio_sm_put_blocking(pio, sm, 1);
    sleep_ms(500);
    pio_sm_put_blocking(pio, sm, 0);
    sleep_ms(500);
  }

  pio_remove_program_and_unclaim_sm(&edbus_program, pio, sm, offset);
}
