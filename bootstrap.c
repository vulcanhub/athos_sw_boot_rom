// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Copyright (C) May 2022, Belmont Computing, Inc. -- All Rights Reserved
// Licensed under the BCI License. See LICENSE for details.

#include "boot_rom/bootstrap.h"

#include <stddef.h>
#include "dif/device.h"
#include "base/memory.h"
#include "base/mmio.h"
#include "dif/dif_gpio.h"
#include "flash_ctrl/flash_ctrl.h"
#include "dif/hart.h"
#include "dif/log.h"
#include "dif/check.h"
 
#include "top/sw/autogen/top_athos.h"

#define GPIO_BOOTSTRAP_BIT_MASK 0x00020000u


int bootstrap(void) {
  if (kDeviceType == kDeviceSimVerilator) {
    mmio_region_t flash_region = mmio_region_from_addr(FLASH_MEM_BASE_ADDR);
    uint32_t value = mmio_region_read32(flash_region, 0x0);
    if (value == 0 || value == UINT32_MAX) return 1;
  }

  return 0;
}
