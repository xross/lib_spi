
#include <stdio.h>
#include <stdlib.h>
#include <print.h>
#include <platform.h>
#include <xcore/parallel.h>
#include <xcore/channel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include "transport.h"
#include "xm_os.h"

port_t p_sclk  = WIFI_CLK;
port_t p_ss[1] = {WIFI_CS_N};
port_t p_miso  = WIFI_MISO;
port_t p_mosi  = WIFI_MOSI;
port_t p_rstn  = WIFI_WUP_RST_N;

#include "spi_xcmm_transport.h" // main lib_spi API include. Will evenually be "spi.h"

#pragma stackfunction 1024
void spi_client(spi_client_t spi, size_t n)
{
    uint32_t addr = 10;
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
        client_spi_begin_transaction(spi, 0, 1000, SPI_MODE_1);

        uint8_t val = client_spi_transfer8(spi, command >> 8);

        val = client_spi_transfer8(spi, command & 0xff);
        uint32_t reg = client_spi_transfer32(spi, 0x00);

        client_spi_end_transaction(spi, 0);

        printhexln(reg << 16 | reg >> 16);

        hwtimer_delay(t, 200000);
    }

#if 1
    client_spi_set(spi, 0);
    printf("Api state: %d\n", client_spi_get(spi));

    client_spi_set(spi, 2);
    printf("Api state: %d\n", client_spi_get(spi));

    client_spi_triple(spi);
    printf("Api state: %d\n", client_spi_get(spi));

    client_spi_add_all(spi, 0x0f0000, 0x0f00, 0x0f);
    printf("Api state: %d\n", client_spi_get(spi));

    client_spi_triple(spi);
    printf("Api state: %d\n", client_spi_get(spi));

    client_spi_stop(spi);
#endif

}

DECLARE_JOB(spi_client, (spi_client_t, size_t));
DECLARE_JOB(spi_server_remote, (const spi_server_args_t*));

void main_remote(spi_server_args_t * args)
{
    spi_link_context_t transport;
    spi_handles_t link = spi_remote_link_ctor(&transport);

    //spi_server_args_t args = { link.server };
    args->srv = link.server;

    PAR_JOBS(
        PJOB(spi_client, (link.client, 0)),
        PJOB(spi_server_remote, (args)));

    spi_remote_link_dtor(&transport);

}

void main_distributed(spi_server_args_t * args)
{
    long long unsigned server_stack[256 + 1];
    struct rxc_shared_server srv_ctx;
    rxc_init_shared_server(&srv_ctx, spi_server_distributed, &server_stack[127]);

    spi_link_context_t transport;
    spi_handles_t link = spi_distributed_link_ctor(&transport, &srv_ctx);

    //spi_server_args_t args = { link.server };
    args->srv = link.server;
    rxc_start_shared_server(&srv_ctx, args);
    spi_client(link.client,0);

    spi_distributed_link_dtor(&transport);
}

int main(void)
{
    struct xm_os_control_block cb;
    xm_os_init(&cb);

    spi_server_args_t args = {
        .srv = NULL,
        .p_sclk = p_sclk,
        .p_mosi = p_mosi,
        .p_miso = p_miso,
        .p_ss = p_ss,
        .num_slaves = NUM_SLAVES
    };

#if REMOTE
    main_remote(&args);
#else
    main_distributed(&args);
#endif

    xm_os_fini();
}



#if 0

#if NUM_CLIENTS == 1
DECLARE_JOB(spi_server_remote, (const spi_server_args_t*, spi_server_params_t*));
#else
DECLARE_JOB(spi_server_remote, (spi_server_t *, size_t, spi_server_params_t));
#endif
DECLARE_JOB(spi_client, (spi_client_t, size_t));

#pragma stackfunction 5000
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
        printstrln("GO");
        while(1);
        client_spi_begin_transaction(spi, 0, 1000, SPI_MODE_1);
        printstrln("DONE");

        uint8_t val = client_spi_transfer8(spi, command >> 8);

        val = client_spi_transfer8(spi, command & 0xff);
        uint32_t reg = client_spi_transfer32(spi, 0x00);

        client_spi_end_transaction(spi, 0);

        printhexln(reg << 16 | reg >> 16);

        hwtimer_delay(t, 200000);
    }
    xm_os_fini();
}

void main_remote(spi_server_params_t params)
{
    printstrln("Remote");


#if NUM_CLIENTS == 1

    spi_link_context_t transport;

    // Allocates channel
    spi_handles_t link = spi_remote_link_ctor(&transport);
    spi_server_args_t args = { link.server };

    PAR_JOBS(
        PJOB(spi_client, (link.client)),
        PJOB(spi_server_remote, (&args, &params))
        );

    spi_remote_link_dtor(&transport);

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

#if 0
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

#endif

int main(void)
{

    //xm_os_enable_for_all_cores();
    struct xm_os_control_block cb;
    xm_os_init(&cb);

    spi_server_params_t params = {
        .p_sclk = p_sclk,
        .p_mosi = p_mosi,
        .p_miso = p_miso,
        .p_ss = p_ss,
        .num_slaves = NUM_SLAVES
    };

    //if(REMOTE)
        main_remote(params);
    //else
       // main_distributed(params);

    // Does this free the chanend for the remote version?
    //xm_os_fini();

    return 0;
}
#endif
