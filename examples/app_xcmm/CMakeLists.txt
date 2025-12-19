cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)

project(app_xcmm)

find_program(LLVM_TBLGEN_EXECUTABLE NAMES llvm-tblgen)

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

set(APP_INCLUDES        src/)

XMOS_REGISTER_APP()


# Table gen stuff

# We need a dummy target to attach the generation steps to if we want them done before the main apps
# Or we can attach to each app. Since we have multiple apps (remote, distributed, etc) sharing the same source,
# it might be cleaner to have a dedicated target for generation that they all depend on, 
# OR just pick one to drive the generation and others will just use the files.

# Let's create a common interface library to hold the generated headers/sources
add_library(app_xcmm_generated_code INTERFACE)

# Generate both JSONs and RPC stubs using the wrapper
xcmm_generate_interface(
    TARGET app_xcmm_generated_code
    SEARCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src
    STUB_OUT_DIR ${CMAKE_CURRENT_LIST_DIR}/src/generated
)

# 3. Link the generated code target to the actual applications
target_link_libraries(app_xcmm_remote PRIVATE app_xcmm_generated_code)
target_link_libraries(app_xcmm_remote_3 PRIVATE app_xcmm_generated_code)
target_link_libraries(app_xcmm_distributed PRIVATE app_xcmm_generated_code)
target_link_libraries(app_xcmm_distributed_3 PRIVATE app_xcmm_generated_code)

# End table gen stuff
