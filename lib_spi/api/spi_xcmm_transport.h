#pragma once

#include <stdint.h>
#include <xcore/port.h>

#include "transport.h"

/** This type indicates what mode an SPI component should use */
typedef enum spi_mode_t {
  SPI_MODE_0, /**< SPI Mode 0 - Polarity = 0, Clock Edge = 1 */
  SPI_MODE_1, /**< SPI Mode 1 - Polarity = 0, Clock Edge = 0 */
  SPI_MODE_2, /**< SPI Mode 2 - Polarity = 1, Clock Edge = 0 */
  SPI_MODE_3, /**< SPI Mode 3 - Polarity = 1, Clock Edge = 1 */
} spi_mode_t;

typedef struct rxc_server spi_server_t;
typedef struct rxc_client *spi_client_t;

/* Client/API consumer functions */
void spi_client_begin_transaction(spi_client_t, unsigned device_index, unsigned speed_in_khz, spi_mode_t mode);
void spi_client_end_transaction(spi_client_t, uint32_t ss_deassert_time);
uint8_t spi_client_transfer8(spi_client_t, uint8_t data);
uint32_t spi_client_transfer32(spi_client_t, uint32_t data);

void spi_server(const struct rxc_server_vt *,
        spi_server_t srv,
        port_t sclk,
        port_t mosi,
        port_t miso,
        port_t p_ss[],
        const size_t num_slaves
        );

void spi_server_remote(spi_server_t,
        port_t sclk,
        port_t mosi,
        port_t miso,
        port_t p_ss[],
        const size_t num_slaves
        );

void spi_server_distributed(void * d,
        port_t sclk,
        port_t mosi,
        port_t miso,
        port_t p_ss[],
        const size_t num_slaves);


