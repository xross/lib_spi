
#include <stdio.h>
#include <platform.h>

#include <print.h>

#include <xcore/parallel.h>
#include <xcore/channel.h>
#include <xcore/hwtimer.h>

#include "spi_xcmm_transport.h" // main lib_spi API include. Will evenually be "spi.h"

#define NUM_SLAVES (1)

port_t p_sclk  = WIFI_CLK;
port_t p_ss[1] = {WIFI_CS_N};
port_t p_miso  = WIFI_MISO;
port_t p_mosi  = WIFI_MOSI;
port_t p_rstn  = WIFI_WUP_RST_N;

void spi_client(spi_client_t spi)
{
    uint32_t addr = 0;
    uint32_t command = 0x8002 | (addr << 12); //Read command

    port_enable(p_rstn);
    port_out(p_rstn, 0x2); //Take out of reset and wait

    hwtimer_t t;
    t = hwtimer_alloc();
    hwtimer_delay(t, 200000);

    for(int i = 0; i < 10000000; i++)
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


DECLARE_JOB(spi_server_remote, (spi_server_t, port_t, port_t, port_t, port_t *, const size_t));
DECLARE_JOB(spi_client, (spi_client_t));


int main(void)
{
    xm_os_enable_for_all_cores();
    struct xm_os_control_block cb;
    xm_os_init(&cb);
#if (REMOTE)
    /* Remote */
    printstrln("Remote");
    chanend_t api_chan = chanend_alloc();

    spi_server_t srv = { (void *) api_chan };

    struct rxc_client cli_storage = { (void *)api_chan, NULL, &rxc_transport_remote_shared.cvt };
    spi_client_t cli = &cli_storage;

    PAR_JOBS(
        PJOB(spi_client, (cli)),
        PJOB(spi_server_remote, (srv, p_sclk, p_mosi, p_miso, p_ss, NUM_SLAVES))
        );

    //chanend_free(api_chan);
#else
    printstrln("Distributed");
    long long unsigned server_stack[128+1];
    struct rxc_shared_server *srv_ctx = alloca(RXC_SHARED_SERVER_SIZE(1));

    rxc_init_shared_server(srv_ctx, 1, spi_server_distributed, &server_stack[127]);
    spi_server_t srv = { &srv_ctx->clients[0] };
    struct rxc_client cli_storage = { srv_ctx, &srv_ctx->clients[0], &rxc_transport_distributed_shared_with_client_exclusion.cvt };
    spi_client_t cli = &cli_storage;

    xm_os_start_shared_context(&srv_ctx->sctx, &srv);
    spi_client(cli);
#endif

    xm_os_fini();
    return 0;
}

