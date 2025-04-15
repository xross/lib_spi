#pragma once

#include <stdint.h>
#include <xcore/port.h>

#include "remote_transport_support.h"

/** This type indicates what mode an SPI component should use */
typedef enum spi_mode_t {
  SPI_MODE_0, /**< SPI Mode 0 - Polarity = 0, Clock Edge = 1 */
  SPI_MODE_1, /**< SPI Mode 1 - Polarity = 0, Clock Edge = 0 */
  SPI_MODE_2, /**< SPI Mode 2 - Polarity = 1, Clock Edge = 0 */
  SPI_MODE_3, /**< SPI Mode 3 - Polarity = 1, Clock Edge = 1 */
} spi_mode_t;

/* Client - generic */
typedef struct
{
    void (*__attribute__((fptrgroup("client.spi_begin_transaction"))) spi_begin_transaction)(void *, unsigned, unsigned, spi_mode_t);
    void (*__attribute__((fptrgroup("client.spi_end_transaction"))) spi_end_transaction)(void *, uint32_t);
    uint8_t (*__attribute__((fptrgroup("client.spi_transfer8"))) spi_transfer8)(void *, uint8_t);
    uint32_t (*__attribute__((fptrgroup("client.spi_transfer32"))) spi_transfer32)(void *, uint32_t);
} spi_client_vtable_t;

/* Might be nice if this was opaque? */
typedef struct
{
    uintptr_t ctx;
    const spi_client_vtable_t *vt;
} spi_client_t;

/* Client/API consumer functions */
void spi_client_begin_transaction(const spi_client_t *, unsigned device_index, unsigned speed_in_khz, spi_mode_t mode);
void spi_client_end_transaction(const spi_client_t *, uint32_t ss_deassert_time);
uint8_t spi_client_transfer8(const spi_client_t *, uint8_t data);
uint32_t spi_client_transfer32(const spi_client_t *, uint32_t data);

/* Transport  - remote */
void spi_server(remote_link_t c,
        port_t sclk,
        port_t mosi,
        port_t miso,
        port_t p_ss[],
        const size_t num_slaves
        );

void spi_remote_client_init(spi_client_t *, remote_link_t);

/*** Below are extra items we need for distributed */

/* Declare underlying API */
/* This is normal 'OO in C' code */
typedef struct
{
    unsigned selected_device;
    unsigned cpol;
    unsigned cpha;
    unsigned period;
    unsigned accepting_new_transactions;
    unsigned num_slaves;
    port_t p_sclk;
    port_t p_mosi;
    port_t p_miso;
    port_t *p_ss;
} spi_ctx_t;

void spi_init(spi_ctx_t *, port_t p_sclk, port_t p_mosi, port_t p_miso, port_t p_ss[], const size_t num_slaves);

/* Transport - distributed */
void spi_distributed_client_init(spi_client_t *, spi_ctx_t *);

