#ifndef COBRA_TCP_CLIENT_H
#define COBRA_TCP_CLIENT_H

typedef struct tcp_client_t tcp_client_t;

#ifdef COBRA_TCP_CLIENT_PRIVATE
struct tcp_client_t {

};
#endif

tcp_client_t *tcp_client_create();
void tcp_client_destroy(tcp_client_t *client);

#endif //COBRA_TCP_CLIENT_H
