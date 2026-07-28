#include "edbus.h"
#include "pico/stdlib.h"
#include <stdio.h>

int main() {
  stdio_init_all();
  edbus_enable();
  return 0;
}
