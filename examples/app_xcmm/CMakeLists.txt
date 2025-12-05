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

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/spi.json
    COMMAND ${LLVM_TBLGEN_EXECUTABLE}
    -I${XMOS_SANDBOX_DIR}/lib_xcmm/lib_xcmm/src/XCMinusMinus/support/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spi.td --dump-json -o ${CMAKE_CURRENT_BINARY_DIR}/spi.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/spi.td
    COMMENT "Generating spi.json using llvm-tblgen"
)

set(GENERATED_HEADER_DIR ${CMAKE_CURRENT_LIST_DIR}/src/generated)
file(MAKE_DIRECTORY ${GENERATED_HEADER_DIR})

add_custom_command(
    OUTPUT  ${GENERATED_HEADER_DIR}/spi_gen.h ${GENERATED_HEADER_DIR}/spi_gen.inc.c
    COMMAND  ${XMOS_SANDBOX_DIR}/lib_xcmm/lib_xcmm/src/XCMinusMinus/genx/rpcgen_in_venv.sh
            -os ${GENERATED_HEADER_DIR}/spi_gen.inc.c
            -oh ${GENERATED_HEADER_DIR}/spi_gen.h
            --header-name spi_gen.h
            ${CMAKE_CURRENT_BINARY_DIR}/spi.json
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/spi.json
    COMMENT "Generating RPC stubs"
)
add_custom_target(app_xcmm_codegen
    DEPENDS ${GENERATED_HEADER_DIR}/spi_gen.h ${GENERATED_HEADER_DIR}/spi_gen.inc.c
)
add_dependencies(app_xcmm_remote          app_xcmm_codegen)
add_dependencies(app_xcmm_remote_3        app_xcmm_codegen)
add_dependencies(app_xcmm_distributed     app_xcmm_codegen)
add_dependencies(app_xcmm_distributed_3   app_xcmm_codegen)

target_include_directories(app_xcmm_remote PRIVATE ${GENERATED_HEADER_DIR})
target_include_directories(app_xcmm_remote_3 PRIVATE ${GENERATED_HEADER_DIR})
target_include_directories(app_xcmm_distributed PRIVATE ${GENERATED_HEADER_DIR})
target_include_directories(app_xcmm_distributed_3 PRIVATE ${GENERATED_HEADER_DIR})

# End table gen stuff

