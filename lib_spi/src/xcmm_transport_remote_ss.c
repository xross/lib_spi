#include <xcore/assert.h>
#include <xcore/port.h>

#include "spi_xcmm_internal.h"
#include "transport_remote_utils.h"

void spi_server(remote_link_t c,
        port_t p_sclk,
        port_t p_mosi,
        port_t p_miso,
        port_t p_ss[],
        const size_t num_slaves
        )
{
    spi_ctx_t ctx;
    spi_init(&ctx, p_sclk, p_mosi, p_miso, p_ss, num_slaves);

    while (1)
    {
        switch (server_await_transaction(c))
        {
            // TODO guarded by accepting_new_transactions
            case SPI_TAG_BEGIN_TRANSACTION: /* void spi_begin_transaction(spi_ctx_t *, int); */
            {
                int device_index = 0;
                int speed_in_khz = 0;
                spi_mode_t mode;
                server_deserialise_i(c, &device_index);
                server_deserialise_i(c, &speed_in_khz);
                server_deserialise_i(c, (int*)&mode);
                spi_begin_transaction(&ctx, device_index, speed_in_khz, mode);
                server_return_v(c);
                break;
            }
            case SPI_TAG_END_TRANSACTION: /* int spi_end_transaction(spi_ctx_t *, uint32_t ss_deassert_time); */
            {
                uint32_t ss_deassert_time;
                server_deserialise_i(c, (int *) &ss_deassert_time);
                spi_end_transaction(&ctx, ss_deassert_time);
                server_return_v(c);
                break;
            }
            case SPI_TAG_TRANSFER8: /* void spi_transfer8(spi_ctx_t *, uint8_t data); */
            {
                uint8_t data;
                server_deserialise_c(c, &data);
                uint8_t r = spi_transfer8(&ctx, data);
                server_return_c(c, r);
                break;
            }
            case SPI_TAG_TRANSFER32: /* void spi_transfer32(spi_ctx_t *, unsigned, short, char); */
            {
                uint32_t data;
                server_deserialise_i(c, (int *) &data);
                uint32_t r = spi_transfer32(&ctx, data);
                server_return_i(c, r);
                break;
            }

            default:
                xassert(0 && "Bad method tag");
        }
    }
}
