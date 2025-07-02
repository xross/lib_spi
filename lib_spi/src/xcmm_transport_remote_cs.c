#include "spi_xcmm_internal.h"
#include "transport_remote_utils.h"


#if 0
__attribute__((fptrgroup("client.spi_begin_transaction"))) static void spi_remote_begin_transaction(void *ctx,
        unsigned device_index, unsigned speed_in_khz, spi_mode_t mode)
{
    remote_link_t t = (remote_link_t)ctx;
    client_request_transaction(t, SPI_TAG_BEGIN_TRANSACTION);
    client_serialise_i(t, device_index);
    client_serialise_i(t, speed_in_khz);
    client_serialise_i(t, (uint32_t) mode);
    client_await_result_v(t);
}

__attribute__((fptrgroup("client.spi_end_transaction"))) static void spi_remote_end_transaction(void *ctx, uint32_t ss_deassert_time)
{
    remote_link_t t = (remote_link_t)ctx;
    client_request_transaction(t, SPI_TAG_END_TRANSACTION);
    client_serialise_i(t, ss_deassert_time);
    client_await_result_v(t);
}

__attribute__((fptrgroup("client.spi_transfer8"))) static uint8_t spi_remote_transfer8(void *ctx, uint8_t data)
{
    uint8_t r;
    remote_link_t t = (remote_link_t)ctx;
    client_request_transaction(t, SPI_TAG_TRANSFER8);
    client_serialise_c(t, data);
    client_await_result_c(t, &r);
    return r;
}

__attribute__((fptrgroup("client.spi_transfer32"))) static uint32_t spi_remote_transfer32(void *ctx, uint32_t data)
{
    uint32_t r;
    remote_link_t t = (remote_link_t)ctx;
    client_request_transaction(t, SPI_TAG_TRANSFER32);
    client_serialise_i(t, data);
    client_await_result_i(t, (int *) &r);
    return r;
}

static const spi_client_vtable_t spi_vtable_remote = {
    spi_remote_begin_transaction,
    spi_remote_end_transaction,
    spi_remote_transfer8,
    spi_remote_transfer32};

void spi_remote_client_init(spi_client_t *client, remote_link_t t)
{
    client->vt = &spi_vtable_remote;
    client->ctx = (uintptr_t)t;
}

#endif
