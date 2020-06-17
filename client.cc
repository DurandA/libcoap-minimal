/* minimal CoAP client
 *
 * Copyright (C) 2018 Olaf Bergmann <bergmann@tzi.org>
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <coap2/coap.h>

#include "common.hh"

int
main(void) {
  coap_context_t  *ctx = nullptr;
  coap_session_t *session = nullptr;
  coap_address_t dst;
  coap_pdu_t *pdu = nullptr;
  int result = EXIT_FAILURE;;

  coap_startup();
  coap_set_log_level(LOG_INFO);

  /* resolve destination address where server should be sent */
  if (resolve_address("localhost", "5683", &dst) < 0) {
    coap_log(LOG_CRIT, "failed to resolve address\n");
    goto finish;
  }

  /* create CoAP context and a client session */
  ctx = coap_new_context(nullptr);

  if (!ctx || !(session = coap_new_client_session(ctx, nullptr, &dst,
                                                  COAP_PROTO_UDP))) {
    coap_log(LOG_EMERG, "cannot create client session\n");
    goto finish;
  }

  /* coap_register_response_handler(ctx, response_handler); */
  coap_register_response_handler(ctx, [](auto, auto, auto,
                                         coap_pdu_t *received,
                                         auto) {
                                        coap_show_pdu(LOG_INFO, received);
                                      });
  /* construct CoAP message */
  pdu = coap_pdu_init(COAP_MESSAGE_CON,
                      COAP_REQUEST_GET,
                      0 /* message id */,
                      coap_session_max_pdu_size(session));
  if (!pdu) {
    coap_log( LOG_EMERG, "cannot create PDU\n" );
    goto finish;
  }

  /* add a Uri-Path option */
  coap_add_option(pdu, COAP_OPTION_OBSERVE, COAP_OBSERVE_ESTABLISH, NULL);
  coap_add_option(pdu, COAP_OPTION_URI_PATH, 4,
                  reinterpret_cast<const uint8_t *>("time"));

  /* and send the PDU */
  coap_send(session, pdu);

  while(1) {
  coap_run_once(ctx, 0);
  }

  result = EXIT_SUCCESS;
 finish:

  coap_session_release(session);
  coap_free_context(ctx);
  coap_cleanup();

  return result;
}
