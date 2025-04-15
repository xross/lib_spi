
#include "spi_xcmm_internal.h"

__attribute__((fptrgroup("client.spi_begin_transaction"))) static void spi_distributed_begin_transaction(void *ctx,
        unsigned device_index, unsigned speed_in_khz, spi_mode_t mode)
{
    spi_ctx_t * c = ctx;
    spi_begin_transaction(c, device_index, speed_in_khz, mode);
}

__attribute__((fptrgroup("client.spi_end_transaction"))) static void spi_distributed_end_transaction(void *ctx, uint32_t ss_deassert_time)
{
    spi_ctx_t * c = ctx;
    spi_end_transaction(c, ss_deassert_time);
}

__attribute__((fptrgroup("client.spi_transfer8"))) static uint8_t spi_distributed_transfer8(void *ctx, uint8_t data)
{
    spi_ctx_t * c = ctx;
    return spi_transfer8(c, data);
}

__attribute__((fptrgroup("client.spi_transfer32"))) static uint32_t spi_distributed_transfer32(void *ctx, uint32_t data)
{
    spi_ctx_t * c = ctx;
    return spi_transfer32(c, data);
}

static const spi_client_vtable_t spi_vtable_distributed = {
    spi_distributed_begin_transaction,
    spi_distributed_end_transaction,
    spi_distributed_transfer8,
    spi_distributed_transfer32};

void spi_distributed_client_init(spi_client_t *client, spi_ctx_t *ctx)
{
    client->vt = &spi_vtable_distributed;
    client->ctx = (uintptr_t)ctx;
    /* Do we want to call spi_init() here? */
}

