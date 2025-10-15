cmake_minimum_required(VERSION 3.21)
include($ENV{XMOS_CMAKE_PATH}/xcommon.cmake)

project(app_an00160)

find_program(LLVM_TBLGEN_EXECUTABLE NAMES llvm-tblgen)


set(APP_HW_TARGET       XK-EVK-XU316)

set(APP_DEPENDENT_MODULES   "lib_spi" "lib_xcmm" "lib_xassert")

set(SHARED_COMPILER_FLAGS         -Os -g -report -DXCORE_AI_EXPLORER=1 -save-temps)

set(APP_COMPILER_FLAGS_remote           ${SHARED_COMPILER_FLAGS} -DREMOTE=1 -DNUM_CLIENTS=1)
set(APP_COMPILER_FLAGS_remote_3         ${SHARED_COMPILER_FLAGS} -DREMOTE=1 -DNUM_CLIENTS=3)
set(APP_COMPILER_FLAGS_distributed      ${SHARED_COMPILER_FLAGS} -DREMOTE=0 -DNUM_CLIENTS=1)
set(APP_COMPILER_FLAGS_distributed_3    ${SHARED_COMPILER_FLAGS} -DREMOTE=0 -DNUM_CLIENTS=3)

set(XMOS_SANDBOX_DIR    ${CMAKE_CURRENT_LIST_DIR}/../../..)

set(APP_INCLUDES        src/)

XMOS_REGISTER_APP()

# Table gen stuff

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/api.json
    COMMAND ${LLVM_TBLGEN_EXECUTABLE}
        -I${CMAKE_CURRENT_SOURCE_DIR}/../../../support/api
        ${CMAKE_CURRENT_SOURCE_DIR}/api.td --dump-json -o ${CMAKE_CURRENT_BINARY_DIR}/api.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api.td
    COMMENT "Generating api.json using llvm-tblgen"
)

set(GENERATED_HEADER_DIR ${CMAKE_CURRENT_LIST_DIR}/src/generated)

add_custom_command(
    OUTPUT  gen_api.h gen_api.inc.c
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../../../genx/rpcgen_in_venv.sh
              -os ${GENERATED_HEADER_DIR}/gen_api.inc.c
              -oh ${GENERATED_HEADER_DIR}/gen_api.h
              --header-name gen_api.h
              ${CMAKE_CURRENT_BINARY_DIR}/api.json
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/api.json)
    add_custom_target(example_rpc_simpletdrpc_gen DEPENDS gen_api.h gen_api.inc.c)
    add_dependencies(app_an00160_remote example_rpc_simpletdrpc_gen)

target_include_directories(app_an00160_remote PRIVATE ${GENERATED_HEADER_DIR} )

# End table gen stuff

