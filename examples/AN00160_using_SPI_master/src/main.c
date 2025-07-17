
#include <stdio.h>
#include <stdlib.h>
#include <platform.h>

#include <print.h>

#include <xcore/parallel.h>
#include <xcore/channel.h>
#include <xcore/hwtimer.h>

#include "spi_xcmm_transport.h" // main lib_spi API include. Will evenually be "spi.h"

port_t p_sclk  = WIFI_CLK;
port_t p_ss[1] = {WIFI_CS_N};
port_t p_miso  = WIFI_MISO;
port_t p_mosi  = WIFI_MOSI;
port_t p_rstn  = WIFI_WUP_RST_N;

#if NUM_CLIENTS == 1
DECLARE_JOB(spi_server_remote, (spi_server_t, spi_server_params_t));
#else
DECLARE_JOB(spi_server_remote, (spi_server_t *, size_t, spi_server_params_t));
#endif
DECLARE_JOB(spi_client, (spi_client_t, size_t));

void spi_client(spi_client_t spi, size_t n)
{
    uint32_t addr = 0;
    uint32_t command = 0x8002 | (addr << 12); //Read command

    if (n == 0)
    {
        port_enable(p_rstn);
        port_out(p_rstn, 0x2); //Take out of reset and wait
    }

    hwtimer_t t;
    t = hwtimer_alloc();
    hwtimer_delay(t, 300000);

    for(int i = 0; i < 2; i++)
    {
        spi_client_begin_transaction(spi, 0, 1000, SPI_MODE_1);

        uint8_t val = spi_client_transfer8(spi, command >> 8);

        val = spi_client_transfer8(spi, command & 0xff);
        uint32_t reg = spi_client_transfer32(spi, 0x00);

        spi_client_end_transaction(spi, 0);

        printhexln(reg << 16 | reg >> 16);

        hwtimer_delay(t, 200000);
    }
}

void main_remote(spi_server_params_t params)
{
    printstrln("Remote");

#if NUM_CLIENTS == 1
    chanend_t api_chan = chanend_alloc();

    spi_server_t srv = { (void *) api_chan };

    struct rxc_client cli_storage = { (void *)api_chan, NULL, &rxc_transport_remote_shared.cvt };
    spi_client_t cli = &cli_storage;

    PAR_JOBS(
        PJOB(spi_client, (cli, 0)),
        PJOB(spi_server_remote, (srv, params))
        );

    chanend_free((chanend_t)srv.sctx);

#else
    spi_server_t srv[NUM_CLIENTS];
    struct rxc_client client_storage[NUM_CLIENTS];
    for (size_t i = 0; i < NUM_CLIENTS; i += 1)
    {
        chanend_t api_chan = chanend_alloc();
        srv[i].sctx = (void *)api_chan;
        client_storage[i] = (struct rxc_client) { (void *)api_chan, NULL, &rxc_transport_remote_shared.cvt };
    }

    /* Note, NUM_CLIENTS not respected here */
    PAR_JOBS(
        PJOB(spi_client, (&client_storage[0], 0)),
        PJOB(spi_client, (&client_storage[1], 1)),
        PJOB(spi_client, (&client_storage[2], 2)),
        PJOB(spi_server_remote, (srv, NUM_CLIENTS, params))
        );

    for (size_t i = 0; i < NUM_CLIENTS; i++)
        chanend_free((chanend_t)srv[i].sctx);
#endif
}

void main_distributed(spi_server_params_t params)
{
    printstrln("Distributed");
    long long unsigned server_stack[128+1];
    struct rxc_shared_server *srv_ctx = alloca(RXC_SHARED_SERVER_SIZE(NUM_CLIENTS));
    rxc_init_shared_server(srv_ctx, NUM_CLIENTS, spi_server_distributed, &server_stack[127]);

#if NUM_CLIENTS == 1
    spi_server_t srv = { &srv_ctx->clients[0]};
    struct rxc_client cli_storage = { srv_ctx, &srv_ctx->clients[0], &rxc_transport_distributed_shared_with_client_exclusion.cvt };
    spi_client_t cli = &cli_storage;

    spi_server_wrapper_t d = { &srv, &params};

    xm_os_start_shared_context(&srv_ctx->sctx, &d);
    spi_client(cli, 0);
#else
    struct rxc_client client_storage[NUM_CLIENTS];
    spi_server_t server_storage[NUM_CLIENTS];
    for (size_t i = 0; i < NUM_CLIENTS; i += 1)
    {
        client_storage[i] = (struct rxc_client){ srv_ctx, &srv_ctx->clients[i], &rxc_transport_distributed_shared_with_client_exclusion.cvt };
        server_storage[i] = (spi_server_t){ &srv_ctx->clients[i] };
    }

    struct rxc_server_handle_wrapper srvs = { NUM_CLIENTS, &server_storage[0] };

    spi_server_wrapper_t d = {&srvs, &params};

    xm_os_start_shared_context(&srv_ctx->sctx, &d);

    /* Note, NUM_CLIENTS not respected here */
    PAR_JOBS(
        PJOB(spi_client, (&client_storage[0], 0)),
        PJOB(spi_client, (&client_storage[1], 1)),
        PJOB(spi_client, (&client_storage[2], 2)));
#endif
}

int main(void)
{
    xm_os_enable_for_all_cores();
    struct xm_os_control_block cb;
    xm_os_init(&cb);

    spi_server_params_t params = {
        .p_sclk = p_sclk,
        .p_mosi = p_mosi,
        .p_miso = p_miso,
        .p_ss = p_ss,
        .num_slaves = NUM_SLAVES
    };

    if(REMOTE)
        main_remote(params);
    else
        main_distributed(params);

    xm_os_fini();
    return 0;
}

