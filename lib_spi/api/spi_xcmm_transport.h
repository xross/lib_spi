#pragma once

#include <stdint.h>
#include <xcore/port.h>

#include "spi_gen.h"

/** This type indicates what mode an SPI component should use */
typedef enum spi_mode_t {
  SPI_MODE_0, /**< SPI Mode 0 - Polarity = 0, Clock Edge = 1 */
  SPI_MODE_1, /**< SPI Mode 1 - Polarity = 0, Clock Edge = 0 */
  SPI_MODE_2, /**< SPI Mode 2 - Polarity = 1, Clock Edge = 0 */
  SPI_MODE_3, /**< SPI Mode 3 - Polarity = 1, Clock Edge = 1 */
} spi_mode_t;

typedef struct {
    spi_server_t srv;
    port_t p_sclk;
    port_t p_mosi;
    port_t p_miso;
    port_t *p_ss;
    size_t num_slaves;
} spi_server_args_t;

#if 0
typedef struct spi_server_params_t
{
    port_t p_sclk;
    port_t p_mosi;
    port_t p_miso;
    port_t *p_ss;
    size_t num_slaves;
} spi_server_params_t;
#endif

#ifndef NUM_SLAVES
#define NUM_SLAVES (1)
#endif

void spi_server_remote(const spi_server_args_t *);


void spi_server_distributed(const void *);


#if 0

#include "spi_gen.h"

#include "transport.h"



#ifndef NUM_CLIENTS
#define NUM_CLIENTS (3)
#endif

#if NUM_SLAVES != 1
#error "NUM_SLAVES must be 1 for now"
#endif

// TODO Merge these two structs?

typedef struct
{
    spi_server_t srv;
} spi_server_args_t;





#if 0

typedef struct rxc_server spi_server_t;
typedef struct rxc_client *spi_client_t;

/* Wrapper for server parameters to be used in distributed mode */
typedef struct spi_server_wrapper_t
{
#if NUM_CLIENTS == 1
    spi_server_t *srv;
#else
    struct rxc_server_handle_wrapper *srvs;
#endif
    spi_server_params_t *params;
} spi_server_wrapper_t;

/* Client/API consumer functions */
void spi_client_begin_transaction(spi_client_t, unsigned device_index, unsigned speed_in_khz, spi_mode_t mode);
void spi_client_end_transaction(spi_client_t, uint32_t ss_deassert_time);
uint8_t spi_client_transfer8(spi_client_t, uint8_t data);
uint32_t spi_client_transfer32(spi_client_t, uint32_t data);

void spi_server(const struct rxc_server_vt *,
        spi_server_t srv,
        spi_server_params_t params
        );

#if NUM_CLIENTS == 1
void spi_server_remote(spi_server_t,
        spi_server_params_t params
        );
#else
void spi_server_remote(spi_server_t *,
        size_t num_clients,
        spi_server_params_t params
        );

#endif

void spi_server_distributed(void * d);

#endif


#endif
