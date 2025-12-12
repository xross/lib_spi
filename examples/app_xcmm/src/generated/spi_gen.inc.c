#include "rxc_request_types.h"
#include <xcore/assert.h>
#include "rxc_server_support.h"
#include "rxc_client_identification.h"
#include "rxc_shared_server.h"
#include <xcore/chanend.h>
#include "rxc_client_support.h"
#include <stdlib.h>
#include <stddef.h>
#include "rxc_transport_shared.h"
#include "rxc_transport_remote.h"
#include "spi_gen.h"
// Interface: spi
// Tag type
enum spi_tag {
  spi_tag_set = 0,
  spi_tag_get = 1,
  spi_tag_triple = 2,
  spi_tag_add_all = 3,
  spi_tag_stop = 4,
  spi_tag_begin_transaction = 5,
  spi_tag_end_transaction = 6,
  spi_tag_transfer8 = 7,
  spi_tag_transfer32 = 8
};
static inline enum spi_tag (spi_valid_tag)(rxc_request_context_t  xif_rctx) {
  const enum spi_tag xif_tag = (enum spi_tag)rxc_get_tag(xif_rctx);
  xassert(xif_tag <= (rxc_request_tag_t)spi_tag_transfer32 && "Unknown tag type!");
  return xif_tag;
}
// Authoritative client wrapper definitions
void (client_spi_set)(spi_client_t, int);
int (client_spi_get)(spi_client_t);
void (client_spi_triple)(spi_client_t);
void (client_spi_add_all)(spi_client_t, unsigned, short, char);
void (client_spi_stop)(spi_client_t);
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
struct xif_u_spi_rbuf_set {
  struct  {
    int xif_p0;
  } xif_req;
};
struct xif_u_spi_rbuf_get {
  struct  {
    int xif_v0;
  } xif_res;
};
struct xif_u_spi_rbuf_add_all {
  struct  {
    unsigned xif_p0;
    short xif_p1;
    char xif_p2;
  } xif_req;
};
struct xif_u_spi_rbuf_begin_transaction {
  struct  {
    unsigned xif_p0;
    unsigned xif_p1;
    unsigned xif_p2;
  } xif_req;
};
struct xif_u_spi_rbuf_end_transaction {
  struct  {
    unsigned xif_p0;
  } xif_req;
};
struct xif_u_spi_rbuf_transfer8 {
  struct  {
    char xif_p0;
  } xif_req;
  struct  {
    char xif_v0;
  } xif_res;
};
struct xif_u_spi_rbuf_transfer32 {
  struct  {
    unsigned xif_p0;
  } xif_req;
  struct  {
    unsigned xif_v0;
  } xif_res;
};
// Generic client functions
__attribute__((always_inline))
static void (xif_u_spi_set_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli, int  xif_p0) {
  struct xif_u_spi_rbuf_set xif_buf;
  xif_buf.xif_req.xif_p0 = xif_p0;
  struct rxc_request_envelope *xif_r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct xif_u_spi_rbuf_set));
  rxc_init_request_envelope(xif_r, spi_tag_set, &xif_buf, sizeof(xif_buf.xif_req));
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, sizeof(xif_buf.xif_req), 0);
  rxc_get_response(&xif_buf, xif_r, sizeof(xif_buf.xif_req), 0);
}
__attribute__((always_inline))
static int (xif_u_spi_get_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli) {
  struct xif_u_spi_rbuf_get xif_buf;
  struct rxc_request_envelope *xif_r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct xif_u_spi_rbuf_get));
  rxc_init_request_envelope(xif_r, spi_tag_get, &xif_buf, offsetof(struct xif_u_spi_rbuf_get, xif_res));
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, offsetof(struct xif_u_spi_rbuf_get, xif_res), sizeof(xif_buf.xif_res));
  rxc_get_response(&xif_buf, xif_r, offsetof(struct xif_u_spi_rbuf_get, xif_res), sizeof(xif_buf.xif_res));
  return xif_buf.xif_res.xif_v0;
}
__attribute__((always_inline))
static void (xif_u_spi_triple_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli) {
  struct rxc_request_envelope *xif_r = alloca(RXC_EMPTY_ENVELOPE_SIZE);
  rxc_init_request_envelope(xif_r, spi_tag_triple, NULL, 0);
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, 0, 0);
  rxc_get_response(NULL, xif_r, 0, 0);
}
__attribute__((always_inline))
static void (xif_u_spi_add_all_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli, unsigned  xif_p0, short  xif_p1, char  xif_p2) {
  struct xif_u_spi_rbuf_add_all xif_buf;
  xif_buf.xif_req.xif_p0 = xif_p0;
  xif_buf.xif_req.xif_p1 = xif_p1;
  xif_buf.xif_req.xif_p2 = xif_p2;
  struct rxc_request_envelope *xif_r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct xif_u_spi_rbuf_add_all));
  rxc_init_request_envelope(xif_r, spi_tag_add_all, &xif_buf, sizeof(xif_buf.xif_req));
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, sizeof(xif_buf.xif_req), 0);
  rxc_get_response(&xif_buf, xif_r, sizeof(xif_buf.xif_req), 0);
}
__attribute__((always_inline))
static void (xif_u_spi_stop_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli) {
  struct rxc_request_envelope *xif_r = alloca(RXC_EMPTY_ENVELOPE_SIZE);
  rxc_init_request_envelope(xif_r, spi_tag_stop, NULL, 0);
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, 0, 0);
  rxc_get_response(NULL, xif_r, 0, 0);
}
__attribute__((always_inline))
static void (xif_u_spi_begin_transaction_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli, unsigned  xif_p0, unsigned  xif_p1, unsigned  xif_p2) {
  struct xif_u_spi_rbuf_begin_transaction xif_buf;
  xif_buf.xif_req.xif_p0 = xif_p0;
  xif_buf.xif_req.xif_p1 = xif_p1;
  xif_buf.xif_req.xif_p2 = xif_p2;
  struct rxc_request_envelope *xif_r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct xif_u_spi_rbuf_begin_transaction));
  rxc_init_request_envelope(xif_r, spi_tag_begin_transaction, &xif_buf, sizeof(xif_buf.xif_req));
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, sizeof(xif_buf.xif_req), 0);
  rxc_get_response(&xif_buf, xif_r, sizeof(xif_buf.xif_req), 0);
}
__attribute__((always_inline))
static void (xif_u_spi_end_transaction_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli, unsigned  xif_p0) {
  struct xif_u_spi_rbuf_end_transaction xif_buf;
  xif_buf.xif_req.xif_p0 = xif_p0;
  struct rxc_request_envelope *xif_r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct xif_u_spi_rbuf_end_transaction));
  rxc_init_request_envelope(xif_r, spi_tag_end_transaction, &xif_buf, sizeof(xif_buf.xif_req));
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, sizeof(xif_buf.xif_req), 0);
  rxc_get_response(&xif_buf, xif_r, sizeof(xif_buf.xif_req), 0);
}
__attribute__((always_inline))
static char (xif_u_spi_transfer8_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli, char  xif_p0) {
  struct xif_u_spi_rbuf_transfer8 xif_buf;
  xif_buf.xif_req.xif_p0 = xif_p0;
  struct rxc_request_envelope *xif_r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct xif_u_spi_rbuf_transfer8));
  rxc_init_request_envelope(xif_r, spi_tag_transfer8, &xif_buf, offsetof(struct xif_u_spi_rbuf_transfer8, xif_res));
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, offsetof(struct xif_u_spi_rbuf_transfer8, xif_res), sizeof(xif_buf.xif_res));
  rxc_get_response(&xif_buf, xif_r, offsetof(struct xif_u_spi_rbuf_transfer8, xif_res), sizeof(xif_buf.xif_res));
  return xif_buf.xif_res.xif_v0;
}
__attribute__((always_inline))
static unsigned (xif_u_spi_transfer32_client_impl)(const struct rxc_client_vt (* xif_cvt), spi_client_t  xif_cli, unsigned  xif_p0) {
  struct xif_u_spi_rbuf_transfer32 xif_buf;
  xif_buf.xif_req.xif_p0 = xif_p0;
  struct rxc_request_envelope *xif_r = alloca(RXC_REQUEST_ENVELOPE_SIZE(struct xif_u_spi_rbuf_transfer32));
  rxc_init_request_envelope(xif_r, spi_tag_transfer32, &xif_buf, offsetof(struct xif_u_spi_rbuf_transfer32, xif_res));
  xif_cvt->request(&xif_cli->xif_c.base, xif_r, offsetof(struct xif_u_spi_rbuf_transfer32, xif_res), sizeof(xif_buf.xif_res));
  rxc_get_response(&xif_buf, xif_r, offsetof(struct xif_u_spi_rbuf_transfer32, xif_res), sizeof(xif_buf.xif_res));
  return xif_buf.xif_res.xif_v0;
}
// Specialised client functions
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_set")))
static void (xif_u_spi_set_distributed)(spi_client_t  xif_c, int  xif_p0) {
  return xif_u_spi_set_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c, xif_p0);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_get")))
static int (xif_u_spi_get_distributed)(spi_client_t  xif_c) {
  return xif_u_spi_get_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_triple")))
static void (xif_u_spi_triple_distributed)(spi_client_t  xif_c) {
  return xif_u_spi_triple_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_add_all")))
static void (xif_u_spi_add_all_distributed)(spi_client_t  xif_c, unsigned  xif_p0, short  xif_p1, char  xif_p2) {
  return xif_u_spi_add_all_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c, xif_p0, xif_p1, xif_p2);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_stop")))
static void (xif_u_spi_stop_distributed)(spi_client_t  xif_c) {
  return xif_u_spi_stop_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_begin_transaction")))
static void (xif_u_spi_begin_transaction_distributed)(spi_client_t  xif_c, unsigned  xif_p0, unsigned  xif_p1, unsigned  xif_p2) {
  return xif_u_spi_begin_transaction_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c, xif_p0, xif_p1, xif_p2);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_end_transaction")))
static void (xif_u_spi_end_transaction_distributed)(spi_client_t  xif_c, unsigned  xif_p0) {
  return xif_u_spi_end_transaction_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c, xif_p0);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_transfer8")))
static char (xif_u_spi_transfer8_distributed)(spi_client_t  xif_c, char  xif_p0) {
  return xif_u_spi_transfer8_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c, xif_p0);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_transfer32")))
static unsigned (xif_u_spi_transfer32_distributed)(spi_client_t  xif_c, unsigned  xif_p0) {
  return xif_u_spi_transfer32_client_impl(&rxc_transport_distributed_shared_with_client_exclusion.cvt, xif_c, xif_p0);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_set")))
static void (xif_u_spi_set_remote)(spi_client_t  xif_c, int  xif_p0) {
  return xif_u_spi_set_client_impl(&rxc_transport_remote_shared.cvt, xif_c, xif_p0);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_get")))
static int (xif_u_spi_get_remote)(spi_client_t  xif_c) {
  return xif_u_spi_get_client_impl(&rxc_transport_remote_shared.cvt, xif_c);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_triple")))
static void (xif_u_spi_triple_remote)(spi_client_t  xif_c) {
  return xif_u_spi_triple_client_impl(&rxc_transport_remote_shared.cvt, xif_c);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_add_all")))
static void (xif_u_spi_add_all_remote)(spi_client_t  xif_c, unsigned  xif_p0, short  xif_p1, char  xif_p2) {
  return xif_u_spi_add_all_client_impl(&rxc_transport_remote_shared.cvt, xif_c, xif_p0, xif_p1, xif_p2);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_stop")))
static void (xif_u_spi_stop_remote)(spi_client_t  xif_c) {
  return xif_u_spi_stop_client_impl(&rxc_transport_remote_shared.cvt, xif_c);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_begin_transaction")))
static void (xif_u_spi_begin_transaction_remote)(spi_client_t  xif_c, unsigned  xif_p0, unsigned  xif_p1, unsigned  xif_p2) {
  return xif_u_spi_begin_transaction_client_impl(&rxc_transport_remote_shared.cvt, xif_c, xif_p0, xif_p1, xif_p2);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_end_transaction")))
static void (xif_u_spi_end_transaction_remote)(spi_client_t  xif_c, unsigned  xif_p0) {
  return xif_u_spi_end_transaction_client_impl(&rxc_transport_remote_shared.cvt, xif_c, xif_p0);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_transfer8")))
static char (xif_u_spi_transfer8_remote)(spi_client_t  xif_c, char  xif_p0) {
  return xif_u_spi_transfer8_client_impl(&rxc_transport_remote_shared.cvt, xif_c, xif_p0);
}
__attribute__((__fptrgroup__("xif_u_spi_cvt_group_transfer32")))
static unsigned (xif_u_spi_transfer32_remote)(spi_client_t  xif_c, unsigned  xif_p0) {
  return xif_u_spi_transfer32_client_impl(&rxc_transport_remote_shared.cvt, xif_c, xif_p0);
}
// VTable definitions
const struct xif_u_spi_client_vt spi_distributed_vt = {
  xif_u_spi_set_distributed,
  xif_u_spi_get_distributed,
  xif_u_spi_triple_distributed,
  xif_u_spi_add_all_distributed,
  xif_u_spi_stop_distributed,
  xif_u_spi_begin_transaction_distributed,
  xif_u_spi_end_transaction_distributed,
  xif_u_spi_transfer8_distributed,
  xif_u_spi_transfer32_distributed
};
const struct xif_u_spi_client_vt spi_remote_vt = {
  xif_u_spi_set_remote,
  xif_u_spi_get_remote,
  xif_u_spi_triple_remote,
  xif_u_spi_add_all_remote,
  xif_u_spi_stop_remote,
  xif_u_spi_begin_transaction_remote,
  xif_u_spi_end_transaction_remote,
  xif_u_spi_transfer8_remote,
  xif_u_spi_transfer32_remote
};
// Server payload helpers
static inline void (spi_get_args_set)(rxc_request_context_t  xif_rctx, int (* xif_p0)) {
  struct xif_u_spi_rbuf_set xif_buf;
  xif_rctx.vt->get_request_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, sizeof(xif_buf.xif_req));
  *xif_p0 = xif_buf.xif_req.xif_p0;
}
static inline void (spi_get_args_get)(rxc_request_context_t __attribute__((unused)) xif_rctx) {
  
}
static inline void (spi_get_args_triple)(rxc_request_context_t __attribute__((unused)) xif_rctx) {
  
}
static inline void (spi_get_args_add_all)(rxc_request_context_t  xif_rctx, unsigned (* xif_p0), short (* xif_p1), char (* xif_p2)) {
  struct xif_u_spi_rbuf_add_all xif_buf;
  xif_rctx.vt->get_request_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, sizeof(xif_buf.xif_req));
  *xif_p0 = xif_buf.xif_req.xif_p0;
  *xif_p1 = xif_buf.xif_req.xif_p1;
  *xif_p2 = xif_buf.xif_req.xif_p2;
}
static inline void (spi_get_args_stop)(rxc_request_context_t __attribute__((unused)) xif_rctx) {
  
}
static inline void (spi_get_args_begin_transaction)(rxc_request_context_t  xif_rctx, unsigned (* xif_p0), unsigned (* xif_p1), unsigned (* xif_p2)) {
  struct xif_u_spi_rbuf_begin_transaction xif_buf;
  xif_rctx.vt->get_request_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, sizeof(xif_buf.xif_req));
  *xif_p0 = xif_buf.xif_req.xif_p0;
  *xif_p1 = xif_buf.xif_req.xif_p1;
  *xif_p2 = xif_buf.xif_req.xif_p2;
}
static inline void (spi_get_args_end_transaction)(rxc_request_context_t  xif_rctx, unsigned (* xif_p0)) {
  struct xif_u_spi_rbuf_end_transaction xif_buf;
  xif_rctx.vt->get_request_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, sizeof(xif_buf.xif_req));
  *xif_p0 = xif_buf.xif_req.xif_p0;
}
static inline void (spi_get_args_transfer8)(rxc_request_context_t  xif_rctx, char (* xif_p0)) {
  struct xif_u_spi_rbuf_transfer8 xif_buf;
  xif_rctx.vt->get_request_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, offsetof(struct xif_u_spi_rbuf_transfer8, xif_res));
  *xif_p0 = xif_buf.xif_req.xif_p0;
}
static inline void (spi_get_args_transfer32)(rxc_request_context_t  xif_rctx, unsigned (* xif_p0)) {
  struct xif_u_spi_rbuf_transfer32 xif_buf;
  xif_rctx.vt->get_request_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, offsetof(struct xif_u_spi_rbuf_transfer32, xif_res));
  *xif_p0 = xif_buf.xif_req.xif_p0;
}
static inline void (spi_set_response_set)(rxc_request_context_t  xif_rctx) {
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_get)(rxc_request_context_t  xif_rctx, int  xif_v0) {
  struct xif_u_spi_rbuf_get xif_buf;
  xif_buf.xif_res.xif_v0 = xif_v0;
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, offsetof(struct xif_u_spi_rbuf_get, xif_res), sizeof(xif_buf.xif_res));
}
static inline void (spi_set_response_triple)(rxc_request_context_t  xif_rctx) {
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_add_all)(rxc_request_context_t  xif_rctx) {
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_stop)(rxc_request_context_t  xif_rctx) {
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_begin_transaction)(rxc_request_context_t  xif_rctx) {
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_end_transaction)(rxc_request_context_t  xif_rctx) {
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, NULL, 0, 0);
}
static inline void (spi_set_response_transfer8)(rxc_request_context_t  xif_rctx, char  xif_v0) {
  struct xif_u_spi_rbuf_transfer8 xif_buf;
  xif_buf.xif_res.xif_v0 = xif_v0;
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, offsetof(struct xif_u_spi_rbuf_transfer8, xif_res), sizeof(xif_buf.xif_res));
}
static inline void (spi_set_response_transfer32)(rxc_request_context_t  xif_rctx, unsigned  xif_v0) {
  struct xif_u_spi_rbuf_transfer32 xif_buf;
  xif_buf.xif_res.xif_v0 = xif_v0;
  xif_rctx.vt->set_response_bytes(xif_rctx.srv, xif_rctx.req, &xif_buf, offsetof(struct xif_u_spi_rbuf_transfer32, xif_res), sizeof(xif_buf.xif_res));
}
