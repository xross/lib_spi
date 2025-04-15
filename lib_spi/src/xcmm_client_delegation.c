#include "spi_xcmm_internal.h"

void spi_client_begin_transaction(const spi_client_t *c,
        unsigned device_index, unsigned speed_in_khz, spi_mode_t mode)
{
    c->vt->spi_begin_transaction((void *)(c->ctx), device_index, speed_in_khz, mode);
}

void spi_client_end_transaction(const spi_client_t *c, uint32_t ss_deassert_time)
{
    c->vt->spi_end_transaction((void *)(c->ctx), ss_deassert_time);
}

uint8_t spi_client_transfer8(const spi_client_t *c, uint8_t data)
{
    return c->vt->spi_transfer8((void *)(c->ctx), data);
}

uint32_t spi_client_transfer32(const spi_client_t *c, uint32_t data)
{
    return c->vt->spi_transfer32((void *)(c->ctx), data);
}
