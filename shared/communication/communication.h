#pragma once

#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>

enum com_type {
  // SQUIRREL data packet
  COM_TYPE_WANT_PACKET,
  COM_TYPE_PACKET,
  // SLAB data packet
  COM_TYPE_WANT_EXTRA,
  COM_TYPE_EXTRA,
  // ALIVE packet
  COM_TYPE_ALIVE,
};

// master_i2c_inst is the i2c_inst_t that the master talks on
// slave_i2c_inst is the i2c_inst_t that the slave listens on
void communication_init(i2c_inst_t *master_i2c_inst,
                        i2c_inst_t *slave_i2c_inst);

void communication_task(bool usb_present, bool should_screensaver,
                        uint32_t millis);
