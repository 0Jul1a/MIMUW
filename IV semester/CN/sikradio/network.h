#ifndef NETWORK_H
#define NETWORK_H

#include "sikradio.h"

// Domyślne wartości numerów portów.
#define HTTPS_PORT  443
#define HTTP_PORT   80

// Statusy oznaczające jakiekolwiek przekierowanie.
#define MULTIPLE_CHOICES     300
#define MOVED_PERMANENTLY    301
#define MOVED_TEMPORARILY    302
#define SEE_OTHER            303
#define REDIRECTED           307
#define PERMANENT_REDIRECTED 308

ssize_t writen_with_proper_encr(const void *vptr, size_t n, connection_t conn);
ssize_t read_with_proper_encr(connection_t conn, char *buf, size_t len);

uint16_t read_port(char const *string);
struct sockaddr_storage get_server_address(char const *host, uint16_t port, int ip_preference);

connection_t connect_to_server(url_t &url, config_t &config);
void close_connection(connection_t &conn);
connection_t reconnect(url_t &url, config_t &config, stream_state_t &stream_state, connection_t &conn);
void send_http_request(url_t &url, config_t &config, connection_t &conn);

void clean_stream_state(stream_state_t &stream_state);

#endif

