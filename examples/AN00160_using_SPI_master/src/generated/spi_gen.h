#include "transport.h"
#include <xcore/chanend.h>
#pragma once

// Interface: spi
struct spi_client_ctx {
  struct rxc_client c;
  const struct spi_client_vt (*vt);
};
typedef struct spi_client_ctx (*spi_client_t);
struct spi_server_ctx {
  struct rxc_server s;
};
typedef struct spi_server_ctx (*spi_server_t);
struct spi_client_vt {
  void ((*begin_transaction))(spi_client_t, unsigned, unsigned, unsigned);
  void ((*end_transaction))(spi_client_t, unsigned);
  char ((*transfer8))(spi_client_t, char);
  unsigned ((*transfer32))(spi_client_t, unsigned);
};
inline void (client_spi_begin_transaction)(spi_client_t c, unsigned p0, unsigned p1, unsigned p2) {
  c->vt->begin_transaction(c, p0, p1, p2);
}
inline void (client_spi_end_transaction)(spi_client_t c, unsigned p0) {
  c->vt->end_transaction(c, p0);
}
inline char (client_spi_transfer8)(spi_client_t c, char p0) {
  return c->vt->transfer8(c, p0);
}
inline unsigned (client_spi_transfer32)(spi_client_t c, unsigned p0) {
  return c->vt->transfer32(c, p0);
}
extern const struct spi_client_vt spi_distributed_vt;
extern const struct spi_client_vt spi_remote_vt;
typedef struct  {
  struct spi_client_ctx c;
  struct spi_server_ctx s;
} spi_link_context_t;
static inline void (spi_distributed_transport_init)(spi_link_context_t (*ctx), struct rxc_shared_server (*srv)) {
  ctx->c = (struct spi_client_ctx) { { { (void*)&srv->sctx } }, &spi_distributed_vt };
  rxc_init_client_identification_block(&ctx->c.c.cib);
  ctx->s = (struct spi_server_ctx) { { (void*)&ctx->c.c.cib } };
}
typedef struct  {
  spi_client_t client;
  spi_server_t server;
} spi_handles_t;
static inline spi_handles_t (spi_distributed_link_ctor)(spi_link_context_t (*ctx), struct rxc_shared_server (*srv)) {
  spi_distributed_transport_init(ctx, srv);
  return (spi_handles_t) { &ctx->c , &ctx->s };
}
static inline void (spi_distributed_transport_fini)(spi_link_context_t (*ctx)) {
  
}
static inline void (spi_distributed_link_dtor)(spi_link_context_t (*ctx)) {
  spi_distributed_transport_fini(ctx);
}
static inline void (spi_remote_client_init)(struct spi_client_ctx (*cli), chanend_t underlying) {
  *cli = (struct spi_client_ctx) { { { (void*)underlying } }, &spi_remote_vt };
}
static inline void (spi_remote_server_init)(struct spi_server_ctx (*srv), chanend_t underlying) {
  *srv = (struct spi_server_ctx) { { (void*)underlying } };
}
static inline spi_handles_t (spi_remote_link_ctor)(spi_link_context_t (*ctx)) {
  chanend_t underlying = chanend_alloc();
  spi_remote_client_init(&ctx->c, underlying);
  spi_remote_server_init(&ctx->s, underlying);
  return (spi_handles_t) { &ctx->c, &ctx->s };
}
static inline void (spi_remote_client_dtor)(struct spi_client_ctx (*cli)) {
  
}
static inline void (spi_remote_server_dtor)(struct spi_server_ctx (*srv)) {
  
}
static inline void (spi_remote_link_dtor)(spi_link_context_t (*ctx)) {
  spi_remote_server_dtor(&ctx->s);
  spi_remote_client_dtor(&ctx->c);
  chanend_free((chanend_t)ctx->s.s.sctx);
}
