#include <xs1.h>
#include <stdio.h>
#include <print.h>
#include <xcore/hwtimer.h>

#include "xassert.h"
#include "spi_xcmm_internal.h"

void get_mode_bits(spi_mode_t mode, unsigned *cpol, unsigned *cpha); // from spi_sync_simple.xc

/* This replaces the SPI server implementation i.e. all the cases in the select */

void spi_init(spi_ctx_t *ctx,
        port_t p_sclk,
        port_t p_mosi,
        port_t p_miso,
        port_t p_ss[],
        const size_t num_slaves)
{
    port_start_buffered(p_sclk, 32);
    for(int i = 0; i < num_slaves; i++)
        port_enable(p_ss[i]);
    port_start_buffered(p_mosi, 32);
    port_start_buffered(p_miso, 32);

    if(p_miso != 0)
    {
        port_set_sample_falling_edge(p_miso);
    }

    if(p_mosi != 0)
        port_out(p_mosi, 0xffffffff);

    ctx->selected_device = 0;
    ctx->accepting_new_transactions = 1;
    ctx->num_slaves = num_slaves;

    ctx->p_sclk = p_sclk;
    ctx->p_mosi = p_mosi;
    ctx->p_miso = p_miso;
    ctx->p_ss = &p_ss[0];
}

void spi_begin_transaction(spi_ctx_t *ctx, unsigned device_index, unsigned speed_in_khz, spi_mode_t mode)
{
    get_mode_bits(mode, &ctx->cpol, &ctx->cpha);

    xassert(device_index < ctx->num_slaves);

    port_sync(ctx->p_sclk);

    // Wait for the chip deassert time if need be
    if(device_index == ctx->selected_device)
        port_sync(ctx->p_ss[ctx->selected_device]);

    // Set the expected clock idle state on the clock port
    if(ctx->cpol)
        port_out_part_word(ctx->p_sclk, 1, ctx->cpol);

    port_sync(ctx->p_sclk);

    // Calculate the clock period from the speed_in_khz
    ctx->period = (XS1_TIMER_KHZ + speed_in_khz - 1)/speed_in_khz;//round up

    // Lock the begin transaction
    ctx->accepting_new_transactions = 0;

    // Do a slave select
    ctx->selected_device = device_index;
    port_out(ctx->p_ss[ctx->selected_device], 0);
}

void spi_end_transaction(spi_ctx_t *ctx, uint32_t ss_deassert_time)
{
    uint32_t time;

    // Unlock the transaction
    ctx->accepting_new_transactions = 1;

    if(ctx->cpol)
        port_out_part_word(ctx->p_sclk, 1, ctx->cpol);

    port_sync(ctx->p_sclk);

    port_out(ctx->p_ss[ctx->selected_device], 1);
    time = (uint32_t)  port_get_trigger_time(ctx->p_ss[ctx->selected_device]);

    //TODO should this be allowed? (0.6ms max without it)
    if(ss_deassert_time > 0xffff)
    {
        hwtimer_t t;
        t = hwtimer_alloc();
        hwtimer_delay(t, ss_deassert_time&0xffff0000);
    }

    time += ss_deassert_time;

    port_out_at_time(ctx->p_ss[ctx->selected_device], (port_timestamp_t) time, 1);
}

/* From spi_sync_simple.xc */
uint8_t transfer8_sync_zero_clkblk(
        port_t sclk,
        port_t mosi,
        port_t miso,
        uint8_t data, const unsigned period,
        const unsigned cpol, const unsigned cpha);

uint8_t spi_transfer8(spi_ctx_t *ctx, uint8_t data)
{
    uint8_t r = transfer8_sync_zero_clkblk(ctx->p_sclk, ctx->p_mosi, ctx->p_miso, data, ctx->period,
            ctx->cpol, ctx->cpha);
    return r;
}

/* From spi_sync_simple.xc */
uint32_t transfer32_sync_zero_clkblk(
        port_t sclk,
        port_t mosi,
        port_t miso,
        uint32_t data, const unsigned period,
        const unsigned cpol, const unsigned cpha);

uint32_t spi_transfer32(spi_ctx_t *ctx, uint32_t data)
{
    uint32_t r = transfer32_sync_zero_clkblk(ctx->p_sclk, ctx->p_mosi, ctx->p_miso, data, ctx->period,
            ctx->cpol, ctx->cpha);
    return r;
}
