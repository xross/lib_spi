
#include <stdio.h>
#include <platform.h>

#include <print.h>

#include <xcore/parallel.h>
#include <xcore/channel.h>
#include <xcore/hwtimer.h>

#include "spi_xcmm.h" // main lib_spi API include

port_t p_sclk  = WIFI_CLK;
port_t p_ss[1] = {WIFI_CS_N};
port_t p_miso  = WIFI_MISO;
port_t p_mosi  = WIFI_MOSI;
port_t p_rstn  = WIFI_WUP_RST_N;

void spi_client(const spi_client_t *spi)
{
    port_enable(p_rstn);
    port_out(p_rstn, 0x2); //Take out of reset and wait

    hwtimer_t t;
    t = hwtimer_alloc();
    hwtimer_delay(t, 200000);

    spi_client_begin_transaction(spi, 0, 1000, SPI_MODE_1);

    uint32_t addr = 0;
    uint32_t command = 0x8002 | (addr << 12); //Read command
    while(1)
    {
        uint8_t val = spi_client_transfer8(spi, command >> 8);
        val = spi_client_transfer8(spi, command & 0xff);
        uint32_t reg;
        reg = spi_client_transfer32(spi, 0x00);
        spi_client_end_transaction(spi, 0);
        printhexln(reg << 16 | reg >> 16);
    }
}

DECLARE_JOB(spi_server, (remote_link_t, port_t, port_t, port_t, port_t *, const size_t));
DECLARE_JOB(spi_client, (const spi_client_t *));

int main(void)
{
    channel_t api_chan = chan_alloc();
    spi_client_t client;
    spi_remote_client_init(&client, api_chan.end_a);

    PAR_JOBS(
        PJOB(spi_server, (api_chan.end_b, p_sclk, p_mosi, p_miso, p_ss, 1)),
        PJOB(spi_client, (&client))
    );

    chan_free(api_chan);
    return 0;
}

