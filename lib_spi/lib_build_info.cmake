set(LIB_NAME lib_spi)
set(LIB_VERSION 3.3.0)
set(LIB_INCLUDES api)
set(LIB_DEPENDENT_MODULES "")
set(LIB_COMPILER_FLAGS_spi_async.xc -Wno-reinterpret-alignment)
XMOS_REGISTER_MODULE()