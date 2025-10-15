#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcore/port.h>
#include <xcore/assert.h>
#include <xcore/hwtimer.h>

#include "spi_xcmm_internal.h"
#include "transport_remote_utils.h"

#if 0
void get_mode_bits(spi_mode_t mode, unsigned *cpol, unsigned *cpha); // from spi_sync_simple.xc

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

static void spi_init(
        spi_ctx_t *ctx,
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

/* Server implemetations of actual SPI - i.e. pin wiggling etc*/
static void spi_begin_transaction_impl(spi_ctx_t *ctx, unsigned device_index, unsigned speed_in_khz, spi_mode_t mode)
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

void spi_end_transaction_impl(spi_ctx_t *ctx, uint32_t ss_deassert_time)
{
    uint32_t time;

    // Unlock the transaction
    ctx->accepting_new_transactions = 1;

    if(ctx->cpol)
        port_out_part_word(ctx->p_sclk, 1, ctx->cpol);

    port_sync(ctx->p_sclk);

    port_out(ctx->p_ss[ctx->selected_device], 0xffffffff);
    time = (uint32_t)  port_get_trigger_time(ctx->p_ss[ctx->selected_device]);

    //TODO should this be allowed? (0.6ms max without it)
    if(ss_deassert_time > 0xffff)
    {
        hwtimer_t t;
        t = hwtimer_alloc();
        hwtimer_delay(t, ss_deassert_time&0xffff0000);
    }

    time += ss_deassert_time;

    port_out_at_time(ctx->p_ss[ctx->selected_device], (port_timestamp_t) time, 0xffffffff);
}

/* From spi_sync_simple.xc */
uint8_t transfer8_sync_zero_clkblk(
        port_t sclk,
        port_t mosi,
        port_t miso,
        uint8_t data, const unsigned period,
        const unsigned cpol, const unsigned cpha);

uint8_t spi_transfer8_impl(spi_ctx_t *ctx, uint8_t data)
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

uint32_t spi_transfer32_impl(spi_ctx_t *ctx, uint32_t data)
{
    uint32_t r = transfer32_sync_zero_clkblk(ctx->p_sclk, ctx->p_mosi, ctx->p_miso, data, ctx->period,
            ctx->cpol, ctx->cpha);
    return r;
}


/* Structures for requests and responses */
struct spi_rbuf_begin_transaction
{
    struct {
        int device_index;
        int speed_in_khz;
        spi_mode_t mode;
    } req;
};

struct spi_rbuf_end_transaction
{
    struct {
        uint32_t ss_deassert_time;
    } req;
};

struct spi_rbuf_transfer8
{
    struct {
        uint8_t data;
    } req;
    struct {
        uint8_t data;
    } res;
};
struct spi_rbuf_transfer32
{
    struct {
        uint32_t data;
    } req;
    struct {
        uint32_t data;
    } res;
};

/* Client API funciton */
#pragma stackfunction 5000
void spi_client_begin_transaction(spi_client_t cli, unsigned device_index, unsigned speed_in_khz, spi_mode_t mode)
{
    struct spi_rbuf_begin_transaction buf = {{device_index, speed_in_khz, mode}};
    struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_begin_transaction));
    rxc_init_request_envelope(r, SPI_TAG_BEGIN_TRANSACTION, &buf, sizeof(buf.req));
    cli->cvt->request(cli, r, sizeof(buf.req), 0);
    rxc_get_response(&buf, r, sizeof(buf.req), 0);
}

#pragma stackfunction 5000
void spi_client_end_transaction(spi_client_t cli, uint32_t ss_deassert_time)
{
    struct spi_rbuf_end_transaction buf = {{ss_deassert_time}};
    struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_end_transaction));
    rxc_init_request_envelope(r, SPI_TAG_END_TRANSACTION, &buf, sizeof(buf.req));
    cli->cvt->request(cli, r, sizeof(buf.req), 0);
    rxc_get_response(&buf, r, sizeof(buf.req), 0);
}

#pragma stackfunction 1000
uint8_t spi_client_transfer8(spi_client_t cli, uint8_t data)
{
    struct spi_rbuf_transfer8 buf = {{data}, {0xAA}};
    struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_transfer8));
    rxc_init_request_envelope(r, SPI_TAG_TRANSFER8, &buf, sizeof(buf));
    cli->cvt->request(cli, r, offsetof(struct spi_rbuf_transfer8, res), sizeof(buf.res));
    rxc_get_response(&buf, r, offsetof(struct spi_rbuf_transfer8, res), sizeof(buf.res));
    return buf.res.data;
}

#pragma stackfunction 1000
uint32_t spi_client_transfer32(spi_client_t cli, uint32_t data)
{
    struct spi_rbuf_transfer32 buf = {{data}, {0xBB}};
    struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_transfer32));
    rxc_init_request_envelope(r, SPI_TAG_TRANSFER32, &buf, sizeof(buf));
    cli->cvt->request(cli, r, offsetof(struct spi_rbuf_transfer32, res), sizeof(buf.res));
    rxc_get_response(&buf, r, offsetof(struct spi_rbuf_transfer32, res), sizeof(buf.res));
    return buf.res.data;
}

__attribute__((always_inline))
void spi_server_impl(
        const struct rxc_server_vt * vt,
#if NUM_CLIENTS > 1
        spi_server_t srv[],
#else
        spi_server_t srv,
#endif
        size_t num_clients,
        port_t p_sclk,
        port_t p_mosi,
        port_t p_miso,
        port_t p_ss[],
        const size_t num_slaves
        )
{
    /* TODO needed this context structure for previous implementation.
     * This can be removed now */
    spi_ctx_t ctx;
    spi_init(&ctx, p_sclk, p_mosi, p_miso, p_ss, num_slaves);

#if NUM_CLIENTS > 1
    const struct rxc_resource_array_descriptor servers = { num_clients, &srv[0] };
    vt->configure_await(&servers);
#endif

    // TODO use ctx.accepting_new_transactions
    int accepting_new_transactions = 1;

    while (1)
    {
        size_t c = 0;
#if NUM_CLIENTS == 1
        size_t idx;
        rxc_request_handle_t req = rxc_complete_and_await_any(vt, &srv, 1, &idx);
#else
        rxc_request_handle_t req = vt->complete_and_await(&servers, &c);
#endif
        switch (vt->get_tag(req))
        {
            case SPI_TAG_BEGIN_TRANSACTION:
            {
                int device_index = 0;
                int speed_in_khz = 0;
                spi_mode_t mode;

                xassert(c < NUM_CLIENTS);
                printf("server client %d: accepting: %d\n", (int) c, accepting_new_transactions);

                struct spi_rbuf_begin_transaction buf;

#if NUM_CLIENTS > 1
                if (!accepting_new_transactions)
                {
                    vt->defer_request(&srv[c], req);
                    continue;
                }
                accepting_new_transactions = 0;
#endif

#if NUM_CLIENTS > 1
                vt->get_request_bytes(&srv[c], req, &buf.req, sizeof(buf.req));
#else
                vt->get_request_bytes(&srv, req, &buf.req, sizeof(buf.req));
#endif

                device_index = buf.req.device_index;
                speed_in_khz = buf.req.speed_in_khz;
                mode = (spi_mode_t)buf.req.mode;

                printf("server: client %d: spi_begin_transaction_impl(%d, %d, %d)\n", c, device_index, speed_in_khz, mode);
                spi_begin_transaction_impl(&ctx, device_index, speed_in_khz, mode);

#if NUM_CLIENTS > 1
                vt->set_response_bytes(&srv[c], req, NULL, 0, 0);
#else
                vt->set_response_bytes(&srv, req, NULL, 0, 0);
#endif
                break;
            }
            case SPI_TAG_END_TRANSACTION:
            {
                uint32_t ss_deassert_time;

                struct spi_rbuf_end_transaction buf;
#if NUM_CLIENTS > 1
                vt->get_request_bytes(&srv[c], req, &buf.req, sizeof(buf.req));
#else
                vt->get_request_bytes(&srv, req, &buf.req, sizeof(buf.req));
#endif

                ss_deassert_time = buf.req.ss_deassert_time;
                printf("server: client: %d: spi_end_transaction_impl ss_deassert_time(%x)\n", c, (unsigned) ss_deassert_time);

                spi_end_transaction_impl(&ctx, ss_deassert_time);

#if NUM_CLIENTS > 1
                accepting_new_transactions = 1;
                vt->set_response_bytes(&srv[c], req, NULL, 0, 0);
#else
                vt->set_response_bytes(&srv, req, NULL, 0, 0);
#endif


                break;
            }
            case SPI_TAG_TRANSFER8:
            {
                uint8_t wrdata;
                uint8_t rddata;

                struct spi_rbuf_transfer8 buf;
#if NUM_CLIENTS > 1
                vt->get_request_bytes(&srv[c], req, &buf.req, sizeof(buf.req));
#else
                vt->get_request_bytes(&srv, req, &buf.req, sizeof(buf.req));
#endif

                wrdata = buf.req.data;

                printf("server: client %d: spi_transfer8 wrdata(%x)\n", c, wrdata);

                rddata =  spi_transfer8_impl(&ctx, wrdata);

                printf("server: client %d: spi_transfer8 rddata(%x)\n", c, rddata);

                buf.res.data = rddata;

#if NUM_CLIENTS > 1
                vt->set_response_bytes(&srv[c], req, &buf, offsetof(struct spi_rbuf_transfer8, res), sizeof(buf.res));
#else
                vt->set_response_bytes(&srv, req, &buf, offsetof(struct spi_rbuf_transfer8, res), sizeof(buf.res));
#endif
                break;
            }

            case SPI_TAG_TRANSFER32:
            {
                uint32_t wrdata;
                uint32_t rddata;

                struct spi_rbuf_transfer32 buf;
#if NUM_CLIENTS > 1
                vt->get_request_bytes(&srv[c], req, &buf.req, sizeof(buf.req));
#else
                vt->get_request_bytes(&srv, req, &buf.req, sizeof(buf.req));
#endif
                wrdata = buf.req.data;

                printf("server: client %d: spi_transfer32 wrdata(%x)\n", c, (unsigned) wrdata);

                rddata =  spi_transfer32_impl(&ctx, wrdata);

                printf("server: client %d: spi_transfer32 rddata(%x)\n", c, (unsigned) rddata);

                buf.res.data = rddata;
#if NUM_CLIENTS > 1
                vt->set_response_bytes(&srv[c], req, &buf, offsetof(struct spi_rbuf_transfer32, res), sizeof(buf.res));
#else
                vt->set_response_bytes(&srv, req, &buf, offsetof(struct spi_rbuf_transfer32, res), sizeof(buf.res));
#endif
                break;
            }
            default:
                xassert(0 && "Bad method tag");
        }
#if NUM_CLIENTS > 1
        vt->configure_await(&servers);
#endif
    }
}


#if NUM_CLIENTS == 1
void spi_server_remote(spi_server_t srv, spi_server_params_t params)
{
    /* Note, num_clients param unused */
    spi_server_impl(&rxc_transport_remote_shared.svt, srv, 1, params.p_sclk,
            params.p_mosi, params.p_miso, params.p_ss, params.num_slaves);
}
#else
void spi_server_remote(spi_server_t *srv, size_t num_clients, spi_server_params_t params)
{
    spi_server_impl(&rxc_transport_remote_shared.svt, srv, num_clients, params.p_sclk,
            params.p_mosi, params.p_miso, params.p_ss, params.num_slaves);
}
#endif


void spi_server_distributed(void * d)
{
#if NUM_CLIENTS == 1
    spi_server_wrapper_t w = *(spi_server_wrapper_t *)d;
    spi_server_t srv = *(w.srv);
    spi_server_params_t params = *(w.params);

    /* Note, num_clients param unused */
    spi_server_impl(&rxc_transport_distributed_shared_with_client_exclusion.svt, srv, 1, params.p_sclk,
        params.p_mosi, params.p_miso, params.p_ss, params.num_slaves);
#else
    spi_server_wrapper_t w = *(spi_server_wrapper_t *)d;
    struct rxc_server_handle_wrapper *handles = w.srvs;
    spi_server_params_t params = *(w.params);

    spi_server_impl(&rxc_transport_distributed_shared_with_client_exclusion.svt, handles->handles, handles->num, params.p_sclk,
        params.p_mosi, params.p_miso, params.p_ss, params.num_slaves);
#endif

}

#endif
