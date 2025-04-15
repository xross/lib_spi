#pragma once

#include "spi_xcmm.h"

#include "remote_transport_support.h"

enum spi_transport_remote_tag_t
{
    SPI_TAG_BEGIN_TRANSACTION,
    SPI_TAG_END_TRANSACTION,
    SPI_TAG_TRANSFER8,
    SPI_TAG_TRANSFER32,
};

/* xccm_server_implementaiton.c */
void spi_init(spi_ctx_t *, port_t p_sclk, port_t p_mosi, port_t p_miso, port_t p_ss[], const size_t num_slaves);

void spi_begin_transaction(spi_ctx_t *ctx, unsigned device_index, unsigned speed_in_khz, spi_mode_t mode);
void spi_end_transaction(spi_ctx_t *, uint32_t ss_deassert_time);
uint8_t spi_transfer8(spi_ctx_t *, uint8_t data);
uint32_t spi_transfer32(spi_ctx_t *, uint32_t data);


