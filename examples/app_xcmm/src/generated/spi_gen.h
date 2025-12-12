#include "rxc_request_types.h"
#include "rxc_client_identification.h"
#include "rxc_shared_server.h"
#include <xcore/chanend.h>
#pragma once

// Interface: spi
struct spi_client_ctx {
  struct rxc_client xif_c;
  const struct xif_u_spi_client_vt (*xif_vt);
};
typedef struct spi_client_ctx (*spi_client_t);
struct spi_server_ctx {
  struct rxc_server s;
};
typedef struct spi_server_ctx (*spi_server_t);
struct xif_u_spi_client_vt {
  void ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_set"))) xif_u_set))(spi_client_t, int);
  int ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_get"))) xif_u_get))(spi_client_t);
  void ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_triple"))) xif_u_triple))(spi_client_t);
  void ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_add_all"))) xif_u_add_all))(spi_client_t, unsigned, short, char);
  void ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_stop"))) xif_u_stop))(spi_client_t);
  void ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_begin_transaction"))) xif_u_begin_transaction))(spi_client_t, unsigned, unsigned, unsigned);
  void ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_end_transaction"))) xif_u_end_transaction))(spi_client_t, unsigned);
  char ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_transfer8"))) xif_u_transfer8))(spi_client_t, char);
  unsigned ((*__attribute__((__fptrgroup__("xif_u_spi_cvt_group_transfer32"))) xif_u_transfer32))(spi_client_t, unsigned);
};
inline void (client_spi_set)(spi_client_t  xif_c, int  xif_p0) {
  xif_c->xif_vt->xif_u_set(xif_c, xif_p0);
}
inline int (client_spi_get)(spi_client_t  xif_c) {
  return xif_c->xif_vt->xif_u_get(xif_c);
}
inline void (client_spi_triple)(spi_client_t  xif_c) {
  xif_c->xif_vt->xif_u_triple(xif_c);
}
inline void (client_spi_add_all)(spi_client_t  xif_c, unsigned  xif_p0, short  xif_p1, char  xif_p2) {
  xif_c->xif_vt->xif_u_add_all(xif_c, xif_p0, xif_p1, xif_p2);
}
inline void (client_spi_stop)(spi_client_t  xif_c) {
  xif_c->xif_vt->xif_u_stop(xif_c);
}
inline void (client_spi_begin_transaction)(spi_client_t  xif_c, unsigned  xif_p0, unsigned  xif_p1, unsigned  xif_p2) {
  xif_c->xif_vt->xif_u_begin_transaction(xif_c, xif_p0, xif_p1, xif_p2);
}
inline void (client_spi_end_transaction)(spi_client_t  xif_c, unsigned  xif_p0) {
  xif_c->xif_vt->xif_u_end_transaction(xif_c, xif_p0);
}
inline char (client_spi_transfer8)(spi_client_t  xif_c, char  xif_p0) {
  return xif_c->xif_vt->xif_u_transfer8(xif_c, xif_p0);
}
inline unsigned (client_spi_transfer32)(spi_client_t  xif_c, unsigned  xif_p0) {
  return xif_c->xif_vt->xif_u_transfer32(xif_c, xif_p0);
}
extern const struct xif_u_spi_client_vt spi_distributed_vt;
extern const struct xif_u_spi_client_vt spi_remote_vt;
typedef struct  {
  struct spi_client_ctx xif_c;
  struct spi_server_ctx xif_s;
} spi_link_context_t;
static inline void (spi_distributed_transport_init)(spi_link_context_t (* xif_ctx), struct rxc_shared_server (* xif_srv)) {
  xif_ctx->xif_c.xif_c.base.cctx = (void*)&xif_srv->sctx;
  xif_ctx->xif_c.xif_vt = &spi_distributed_vt;
  rxc_init_client_identification_block(&xif_ctx->xif_c.xif_c.cib);
  xif_ctx->xif_s.s.sctx = (void*)&xif_ctx->xif_c.xif_c.cib;
}
typedef struct  {
  spi_client_t client;
  spi_server_t server;
} spi_handles_t;
static inline spi_handles_t (spi_distributed_link_ctor)(spi_link_context_t (* xif_ctx), struct rxc_shared_server (* xif_srv)) {
  spi_distributed_transport_init(xif_ctx, xif_srv);
  return (spi_handles_t) { &xif_ctx->xif_c , &xif_ctx->xif_s };
}
static inline void (spi_distributed_transport_fini)(spi_link_context_t (*__attribute__((unused)) xif_ctx)) {
  
}
static inline void (spi_distributed_link_dtor)(spi_link_context_t (* xif_ctx)) {
  spi_distributed_transport_fini(xif_ctx);
}
static inline void (spi_remote_client_init)(struct spi_client_ctx (* xif_cli), chanend_t  xif_underlying) {
  xif_cli->xif_c.base.cctx = (void*)xif_underlying;
  xif_cli->xif_vt = &spi_remote_vt;
}
static inline void (spi_remote_server_init)(struct spi_server_ctx (* xif_srv), chanend_t  xif_underlying) {
  xif_srv->s.sctx = (void*)xif_underlying;
}
static inline spi_handles_t (spi_remote_link_ctor)(spi_link_context_t (* xif_ctx)) {
  chanend_t xif_underlying = chanend_alloc();
  spi_remote_client_init(&xif_ctx->xif_c, xif_underlying);
  spi_remote_server_init(&xif_ctx->xif_s, xif_underlying);
  return (spi_handles_t) { &xif_ctx->xif_c, &xif_ctx->xif_s };
}
static inline void (spi_remote_client_dtor)(struct spi_client_ctx (*__attribute__((unused)) xif_cli)) {
  
}
static inline void (spi_remote_server_dtor)(struct spi_server_ctx (*__attribute__((unused)) xif_srv)) {
  
}
static inline void (spi_remote_link_dtor)(spi_link_context_t (* xif_ctx)) {
  spi_remote_server_dtor(&xif_ctx->xif_s);
  spi_remote_client_dtor(&xif_ctx->xif_c);
  chanend_free((chanend_t)xif_ctx->xif_s.s.sctx);
}
