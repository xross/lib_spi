cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)

project(app_xcmm)

set(APP_HW_TARGET       XK-EVK-XU316)

set(APP_DEPENDENT_MODULES   "lib_spi" "xross/lib_xcmm" "lib_xassert")

set(SHARED_COMPILER_FLAGS         -Os -g -report -DXCORE_AI_EXPLORER=1 -save-temps)

set(APP_COMPILER_FLAGS_remote           ${SHARED_COMPILER_FLAGS} -DREMOTE=1 -DNUM_CLIENTS=1)
set(APP_COMPILER_FLAGS_remote_3         ${SHARED_COMPILER_FLAGS} -DREMOTE=1 -DNUM_CLIENTS=3)
set(APP_COMPILER_FLAGS_distributed      ${SHARED_COMPILER_FLAGS} -DREMOTE=0 -DNUM_CLIENTS=1)
set(APP_COMPILER_FLAGS_distributed_3    ${SHARED_COMPILER_FLAGS} -DREMOTE=0 -DNUM_CLIENTS=3)

# Set src file (so we dont glob the generated included files)
set(APP_C_SRCS    src/main.c)

set(XMOS_SANDBOX_DIR    ${CMAKE_CURRENT_LIST_DIR}/../../..)

# Required for now until we take this properly into the xcommonn-cmake build system
include(${XMOS_SANDBOX_DIR}/lib_xcmm/lib_xcmm/cmake/xcmm_codegen.cmake)

set(APP_INCLUDES        src/)

# Note the use of XCMM_APP here! it calls the xcommon-cmake function XMOS_REGISTER_APP()
# Eventually we move this functionally into XMOS_REGISTER_APP()
XMOS_REGISTER_XCMM_APP()
