cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)
project(app_an00160)

set(APP_HW_TARGET       XK-EVK-XU316)

set(APP_DEPENDENT_MODULES   "lib_spi" "lib_xcmm" "lib_xassert")

set(APP_COMPILER_FLAGS      -O3 -g -report -DXCORE_AI_EXPLORER=1)

set(XMOS_SANDBOX_DIR    ${CMAKE_CURRENT_LIST_DIR}/../../..)

set(APP_INCLUDES        src/)

XMOS_REGISTER_APP()

