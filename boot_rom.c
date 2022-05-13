// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "boot_rom/bootstrap.h"
#include "boot_rom/chip_info.h"  // Generated.
#include "dif/device.h"
#include "base/mmio.h"
#include "dif/dif_gpio.h"
#include "dif/dif_uart.h"
#include "flash_ctrl/flash_ctrl.h"
#include "dif/hart.h"
#include "dif/log.h"
#include "dif/print.h"
#include "dif/check.h"
#include "dif/test_status.h"

#include "top/sw/autogen/top_athos.h"  // Generated.

/**
 * This symbol is defined in sw/device/boot_rom/rom_link.ld,
 * and describes the location of the flash header.
 *
 * The actual contents are not defined by the ROM, but rather
 * by the flash binary: see sw/device/exts/common/flash_link.ld
 * for that.
 */
extern struct { void (*entry)(void); } _flash_header;

static dif_uart_t uart0;

void _boot_start(void) {
  test_status_set(kTestStatusInBootRom);
  flash_init();

  CHECK(
      dif_uart_init(
          (dif_uart_params_t){
              .base_addr = mmio_region_from_addr(TOP_ATHOS_UART0_BASE_ADDR),
          },
          &uart0) == kDifUartOk,
      "failed to init UART");
  CHECK(dif_uart_configure(&uart0,
                           (dif_uart_config_t){
                               .baudrate = kUartBaudrate,
                               .clk_freq_hz = kClockFreqPeripheralHz,
                               .parity_enable = kDifUartToggleDisabled,
                               .parity = kDifUartParityEven,
                           }) == kDifUartConfigOk,
        "failed to configure UART");
  base_uart_stdout(&uart0);

  LOG_INFO("--------------------------------------------");
  LOG_INFO("---   Initiating Athos Boot Sequence        ");
  LOG_INFO(" %s", chip_info);
  LOG_INFO("--------------------------------------------");
  int bootstrap_err = bootstrap();
  if (bootstrap_err != 0) {
    LOG_ERROR("Bootstrap failed with status code: %d", bootstrap_err);
    // Currently the only way to recover is by a hard reset.
    test_status_set(kTestStatusFailed);
  }

  LOG_INFO("Boot successful...\n");  
  LOG_INFO("--------------------------------------------");
  LOG_INFO("---   Starting the program in FlashRAM      ");
  LOG_INFO("--------------------------------------------");

  // Jump into flash. At this point, the contents of the flash binary have been
  // verified, and we can transfer control directly to it. It is the
  // flash binary's responsibily to set up its own stack, and to never
  // return.
  _flash_header.entry();

  // If the flash image returns, we should abort anyway.
  abort();
}
