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

void receive(PIO rx_pio, uint rx_sm, uint rx_restart_offset) {
  while (1) {
    printf("---\nStart test\n");
    pio_sm_exec(rx_pio, rx_sm, pio_encode_jmp(rx_restart_offset));
    pio_sm_clear_fifos(rx_pio, rx_sm);

    bool passing = true;
    uint32_t values_expected[8];
    uint32_t values_received[8];
    int i;
    int j;
    for (i = 0; i < UINT16_MAX && passing; i++) {
      for (int k = 0; k < 8; k++) {
        values_expected[k] = i;
      }
      values_received[0] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[1] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[2] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[3] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[4] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[5] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[6] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[7] = pio_sm_get_blocking(rx_pio, rx_sm);
      for (j = 0; j < 8; j++) {
        if (values_expected[j] != values_received[j]) {
          passing = false;
          break;
        }
      }
    }
    if (passing) {
      printf("RX TEST PASSED\n");
      gpio_put(PICO_DEFAULT_LED_PIN, true);
    } else {
      printf("RX TEST FAILED\n");
      printf("Failure at i=%d, j=%d: ", i, j);
      printf("Expected %08x, got %08x.\n", values_expected[j],
             values_received[j]);
    }
    sleep_ms(500);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    sleep_ms(500);
  }
}

void send(PIO tx_pio, uint tx_sm, uint tx_restart_offset, PIO rx_pio,
          uint rx_sm, uint rx_restart_offset) {
  while (1) {
    printf("---\nStart test\n");
    pio_sm_set_enabled(rx_pio, rx_sm, false);
    pio_sm_set_enabled(tx_pio, tx_sm, false);
    pio_sm_clear_fifos(rx_pio, rx_sm);
    pio_sm_clear_fifos(tx_pio, tx_sm);
    pio_sm_exec(rx_pio, rx_sm, pio_encode_jmp(rx_restart_offset));
    pio_sm_exec(tx_pio, tx_sm, pio_encode_jmp(tx_restart_offset));
    pio_sm_set_enabled(rx_pio, rx_sm, true);
    pio_sm_set_enabled(tx_pio, tx_sm, true);

    bool passing = true;
    uint32_t values_to_send[8];
    uint32_t values_received[8];
    int i;
    int j;
    for (i = 0; i < UINT16_MAX && passing; i++) {
      for (int k = 0; k < 8; k++) {
        values_to_send[k] = i;
      }
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[0]);
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[1]);
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[2]);
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[3]);
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[4]);
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[5]);
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[6]);
      pio_sm_put_blocking(tx_pio, tx_sm, values_to_send[7]);
      values_received[0] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[1] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[2] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[3] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[4] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[5] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[6] = pio_sm_get_blocking(rx_pio, rx_sm);
      values_received[7] = pio_sm_get_blocking(rx_pio, rx_sm);
      for (j = 0; j < 8; j++) {
        if (values_to_send[j] != values_received[j]) {
          passing = false;
          break;
        }
      }
    }
    if (passing) {
      printf("TX TEST PASSED\n");
      gpio_put(PICO_DEFAULT_LED_PIN, true);
    } else {
      printf("TX TEST FAILED\n");
      printf("Failure at i=%d, j=%d: ", i, j);
      printf("Expected %08x, got %08x.\n", values_to_send[j],
             values_received[j]);
    }
    sleep_ms(500);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    sleep_ms(1000);
  }
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

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);

  sleep_ms(5000);

  /*send(tx_pio, tx_sm, tx_offset + edbus_tx_offset_reset, rx_pio, rx_sm,*/
  /*rx_offset + edbus_rx_offset_reset);*/
  receive(rx_pio, rx_sm, rx_offset + edbus_rx_offset_reset);

  return 0;
}
