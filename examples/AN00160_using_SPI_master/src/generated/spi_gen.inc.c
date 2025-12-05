#include "transport.h"
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <stdlib.h>
#include <stddef.h>
#include "spi_gen.h"
// Interface: spi
// Tag type
enum spi_tag {
  spi_tag_begin_transaction = 0,
  spi_tag_end_transaction = 1,
  spi_tag_transfer8 = 2,
  spi_tag_transfer32 = 3
};
static inline enum spi_tag (spi_valid_tag)(rxc_request_context_t rctx) {
  const enum spi_tag tag = (enum spi_tag)rxc_get_tag(rctx);
  xassert(tag <= (rxc_request_tag_t)spi_tag_transfer32 && "Unknown tag type!");
  return tag;
}
// Authoritative client wrapper definitions
void (client_spi_begin_transaction)(spi_client_t, unsigned, unsigned, unsigned);
void (client_spi_end_transaction)(spi_client_t, unsigned);
char (client_spi_transfer8)(spi_client_t, char);
unsigned (client_spi_transfer32)(spi_client_t, unsigned);
// Authoritative transport construction helpers
void (spi_distributed_transport_init)(spi_link_context_t (*), struct rxc_shared_server (*));
spi_handles_t (spi_distributed_link_ctor)(spi_link_context_t (*), struct rxc_shared_server (*));
void (spi_distributed_transport_fini)(spi_link_context_t (*));
void (spi_distributed_link_dtor)(spi_link_context_t (*));
void (spi_remote_client_init)(struct spi_client_ctx (*), chanend_t);
void (spi_remote_server_init)(struct spi_server_ctx (*), chanend_t);
spi_handles_t (spi_remote_link_ctor)(spi_link_context_t (*));
void (spi_remote_client_dtor)(struct spi_client_ctx (*));
void (spi_remote_server_dtor)(struct spi_server_ctx (*));
void (spi_remote_link_dtor)(spi_link_context_t (*));
// Payload buffer types
struct spi_rbuf_begin_transaction {
  struct  {
    unsigned p0;
    unsigned p1;
    unsigned p2;
  } req;
};
struct spi_rbuf_end_transaction {
  struct  {
    unsigned p0;
  } req;
};
struct spi_rbuf_transfer8 {
  struct  {
    char p0;
  } req;
  struct  {
    char v0;
  } res;
};
struct spi_rbuf_transfer32 {
  struct  {
    unsigned p0;
  } req;
  struct  {
    unsigned v0;
  } res;
};
// Generic client functions
__attribute__((always_inline))
static void (client_spi_begin_transaction_impl)(const struct rxc_client_vt (*cvt), spi_client_t cli, unsigned p0, unsigned p1, unsigned p2) {
  struct spi_rbuf_begin_transaction buf = {{ p0, p1, p2 }};
  struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_begin_transaction));
  rxc_init_request_envelope(r, spi_tag_begin_transaction, &buf, sizeof(buf.req));
  cvt->request(&cli->c.base, r, sizeof(buf.req), 0);
  rxc_get_response(&buf, r, sizeof(buf.req), 0);
}
__attribute__((always_inline))
static void (client_spi_end_transaction_impl)(const struct rxc_client_vt (*cvt), spi_client_t cli, unsigned p0) {
  struct spi_rbuf_end_transaction buf = {{ p0 }};
  struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_end_transaction));
  rxc_init_request_envelope(r, spi_tag_end_transaction, &buf, sizeof(buf.req));
  cvt->request(&cli->c.base, r, sizeof(buf.req), 0);
  rxc_get_response(&buf, r, sizeof(buf.req), 0);
}
__attribute__((always_inline))
static char (client_spi_transfer8_impl)(const struct rxc_client_vt (*cvt), spi_client_t cli, char p0) {
  struct spi_rbuf_transfer8 buf = {{ p0 }};
  struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_transfer8));
  rxc_init_request_envelope(r, spi_tag_transfer8, &buf, offsetof(struct spi_rbuf_transfer8, res));
  cvt->request(&cli->c.base, r, offsetof(struct spi_rbuf_transfer8, res), sizeof(buf.res));
  rxc_get_response(&buf, r, offsetof(struct spi_rbuf_transfer8, res), sizeof(buf.res));
  return buf.res.v0;
}
__attribute__((always_inline))
static unsigned (client_spi_transfer32_impl)(const struct rxc_client_vt (*cvt), spi_client_t cli, unsigned p0) {
  struct spi_rbuf_transfer32 buf = {{ p0 }};
  struct rxc_request_envelope *r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct spi_rbuf_transfer32));
  rxc_init_request_envelope(r, spi_tag_transfer32, &buf, offsetof(struct spi_rbuf_transfer32, res));
  cvt->request(&cli->c.base, r, offsetof(struct spi_rbuf_transfer32, res), sizeof(buf.res));
  rxc_get_response(&buf, r, offsetof(struct spi_rbuf_transfer32, res), sizeof(buf.res));
  return buf.res.v0;
}
// Specialised client functions
static void (distributed_spi_begin_transaction)(spi_client_t c, unsigned p0, unsigned p1, unsigned p2) {
  return client_spi_begin_transaction_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, c, p0, p1, p2);
}
static void (distributed_spi_end_transaction)(spi_client_t c, unsigned p0) {
  return client_spi_end_transaction_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, c, p0);
}
static char (distributed_spi_transfer8)(spi_client_t c, char p0) {
  return client_spi_transfer8_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, c, p0);
}
static unsigned (distributed_spi_transfer32)(spi_client_t c, unsigned p0) {
  return client_spi_transfer32_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, c, p0);
}
static void (remote_spi_begin_transaction)(spi_client_t c, unsigned p0, unsigned p1, unsigned p2) {
  return client_spi_begin_transaction_impl(&rxc_transport_remote_shared.cvt, c, p0, p1, p2);
}
static void (remote_spi_end_transaction)(spi_client_t c, unsigned p0) {
  return client_spi_end_transaction_impl(&rxc_transport_remote_shared.cvt, c, p0);
}
static char (remote_spi_transfer8)(spi_client_t c, char p0) {
  return client_spi_transfer8_impl(&rxc_transport_remote_shared.cvt, c, p0);
}
static unsigned (remote_spi_transfer32)(spi_client_t c, unsigned p0) {
  return client_spi_transfer32_impl(&rxc_transport_remote_shared.cvt, c, p0);
}
// VTable definitions
const struct spi_client_vt spi_distributed_vt = {
  distributed_spi_begin_transaction,
  distributed_spi_end_transaction,
  distributed_spi_transfer8,
  distributed_spi_transfer32
};
const struct spi_client_vt spi_remote_vt = {
  remote_spi_begin_transaction,
  remote_spi_end_transaction,
  remote_spi_transfer8,
  remote_spi_transfer32
};
// Server payload helpers
static inline void (spi_get_args_begin_transaction)(rxc_request_context_t rctx, unsigned (*p0), unsigned (*p1), unsigned (*p2)) {
  struct spi_rbuf_begin_transaction buf;
  rctx.vt->get_request_bytes(rctx.srv, rctx.req, &buf, sizeof(buf.req));
  *p0 = buf.req.p0;
  *p1 = buf.req.p1;
  *p2 = buf.req.p2;
}
static inline void (spi_get_args_end_transaction)(rxc_request_context_t rctx, unsigned (*p0)) {
  struct spi_rbuf_end_transaction buf;
  rctx.vt->get_request_bytes(rctx.srv, rctx.req, &buf, sizeof(buf.req));
  *p0 = buf.req.p0;
}
static inline void (spi_get_args_transfer8)(rxc_request_context_t rctx, char (*p0)) {
  struct spi_rbuf_transfer8 buf;
  rctx.vt->get_request_bytes(rctx.srv, rctx.req, &buf, offsetof(struct spi_rbuf_transfer8, res));
  *p0 = buf.req.p0;
}
static inline void (spi_get_args_transfer32)(rxc_request_context_t rctx, unsigned (*p0)) {
  struct spi_rbuf_transfer32 buf;
  rctx.vt->get_request_bytes(rctx.srv, rctx.req, &buf, offsetof(struct spi_rbuf_transfer32, res));
  *p0 = buf.req.p0;
}
static inline void (spi_set_response_begin_transaction)(rxc_request_context_t rctx) {
  rctx.vt->set_response_bytes(rctx.srv, rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_end_transaction)(rxc_request_context_t rctx) {
  rctx.vt->set_response_bytes(rctx.srv, rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_transfer8)(rxc_request_context_t rctx, char v0) {
  struct spi_rbuf_transfer8 buf;
  buf.res.v0 = v0;
  rctx.vt->set_response_bytes(rctx.srv, rctx.req, &buf, offsetof(struct spi_rbuf_transfer8, res), sizeof(buf.res));
}
static inline void (spi_set_response_transfer32)(rxc_request_context_t rctx, unsigned v0) {
  struct spi_rbuf_transfer32 buf;
  buf.res.v0 = v0;
  rctx.vt->set_response_bytes(rctx.srv, rctx.req, &buf, offsetof(struct spi_rbuf_transfer32, res), sizeof(buf.res));
}
