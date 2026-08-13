// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#include <mcu/core.h>

#include "vystriyae/core.h"

#include "page.h"

i32 main() {
   System_init();
   set_target_fps(60);
   Page_open();
   System_run();
   System_close();

   return 0;
}

