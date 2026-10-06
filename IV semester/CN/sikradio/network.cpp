#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "err.h"
#include "sikradio.h"
#include "network.h"

// Funkcja z zajęć, zmodyfikowana o dodanie dodatkowej obsługi SSL.
ssize_t writen_with_proper_encr(const void *vptr, size_t n, connection_t conn) {
    ssize_t nleft, nwritten;
    const char *ptr = (char*)vptr;               // Can't do pointer arithmetic on void*.
    nleft = n;
    while (nleft > 0) {
        if (conn.is_ssl) {
            nwritten = SSL_write(conn.ssl, ptr, nleft);
        }
        else {
            nwritten = write(conn.fd, ptr, nleft);
        }

        if (nwritten <= 0)
            return nwritten;  // error

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

// Funkcja z zajęć.
uint16_t read_port(char const *string) {
    char *endptr;
    errno = 0;
    unsigned long port = strtoul(string, &endptr, 10);
    if (errno != 0 || *endptr != 0 || port > UINT16_MAX) {
        fatal("%s is not a valid port number", string);
    }
    return (uint16_t) port;
}

// Funkcja get_server_address z zajęć, zmodyfikowana tak, aby obsługiwała IPv4 oraz IPv6 w zależności
// od tego, czy/jaka została podana informacja na temat wersji IP do obsłużenia.
// ip_preference: chcemy IPv4, IPv6 lub obojętnie który.
struct sockaddr_storage get_server_address(char const *host, uint16_t port, int ip_preference) {

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = ip_preference;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *address_result;
    int errcode = getaddrinfo(host, NULL, &hints, &address_result);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        exit(1);
    }

    struct sockaddr_storage send_address;
    memset(&send_address, 0, sizeof(send_address));

    // Kopiujemy pierwszy zwrócony przez getaddrinfo adres.
    memcpy(&send_address, address_result->ai_addr, address_result->ai_addrlen);

    // Ustawienie portu w zależności od przypisanego adresu.
    if (send_address.ss_family == AF_INET) {
        struct sockaddr_in *send_addres_v4 = (struct sockaddr_in *) &send_address;
        send_addres_v4->sin_port = htons(port);

    }
    else if (send_address.ss_family == AF_INET6) {
        struct sockaddr_in6 *send_addres_v6 = (struct sockaddr_in6 *) &send_address;
        send_addres_v6->sin6_port = htons(port);
    }

    freeaddrinfo(address_result);

    return send_address;
}

// Funkcja pomocnicza do wypisania informacji dotyczących połączenia.
void log_connection_info(url_t &url, sockaddr_storage &addr, config_t &config) {
    if (config.verbosity >= 1) {
        time_t now = time(NULL);
        char strftime_buf[64];
        strftime(strftime_buf, sizeof(strftime_buf), "%Y.%m.%d %H.%M.%S", localtime(&now));
        fprintf(stderr, "%s\n", strftime_buf);

        fprintf(stderr, "resolving name %s\n", url.host.c_str());
        if (addr.ss_family == AF_INET6) {
            char ipstr[INET6_ADDRSTRLEN];
            char portstr[INET6_ADDRSTRLEN];
            int res = getnameinfo((struct sockaddr*)&addr, sizeof(struct sockaddr_in6), ipstr, sizeof(ipstr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);
            if (res == 0)
                fprintf(stderr, "connecting to server [%s]:%s\n", ipstr, portstr);
        }
        else {
            char ipstr[INET_ADDRSTRLEN];
            char portstr[INET_ADDRSTRLEN];
            int res = getnameinfo((struct sockaddr*)&addr, sizeof(struct sockaddr_in), ipstr, sizeof(ipstr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);;
            if (res == 0)
                fprintf(stderr, "connecting to server %s:%s\n", ipstr, portstr);
        }
    }
}

// Tworzymy nowe połączenie z serwerem obsługując zarówno HTTP, jak i HTTPS.
connection_t connect_to_server(url_t &url, config_t &config) {
    struct sockaddr_storage addr = get_server_address(url.host.c_str(), url.port, config.ip_version);

    connection_t conn;
    conn.is_ssl = (url.scheme == "https");
    conn.ctx = NULL;
    conn.ssl = NULL;

    conn.fd = socket(addr.ss_family, SOCK_STREAM, 0);
    if (conn.fd < 0) {
        log_msg(CRITICAL_ERRORS_LOGS, "cannot create a socket", config);
        exit(1);
    }

    socklen_t addr_len = (addr.ss_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

    if (connect(conn.fd, (struct sockaddr *) &addr, addr_len) < 0) {
        log_msg(CRITICAL_ERRORS_LOGS, "cannot connect to the server", config);
        exit(1);
    }

    // Inicjalizacja SSL dla połączenia HTTPS.
    if (conn.is_ssl) {
        conn.ctx = SSL_CTX_new(TLS_client_method());
        if (conn.ctx == NULL) {
            log_msg(CRITICAL_ERRORS_LOGS, "cannot create SSL context", config);
            exit(1);
        }
        conn.ssl = SSL_new(conn.ctx);
        SSL_set_fd(conn.ssl, conn.fd);

        SSL_set_tlsext_host_name(conn.ssl, url.host.c_str());

        if (SSL_connect(conn.ssl) <= 0) {
            log_msg(CRITICAL_ERRORS_LOGS, "cannot connect to the server", config);
            exit(1);
        }
    }

    log_connection_info(url, addr, config);

    return conn;
}

// Przywrócenie wartości domyślnych dotyczących aktualnego stanu przepływu danych.
void clean_stream_state(stream_state_t &stream_state) {
    stream_state.header_done = false;
    stream_state.header.clear();
    stream_state.icy = 0;
    stream_state.bytes_until_metadata = 0;
    stream_state.reading_metadata = false;
    stream_state.metadata_size_to_read = 0;
    stream_state.metadata.clear();
}

// Generuje i wysyła żądanie HTTP GET do serwera.
void send_http_request(url_t &url, config_t &config, connection_t &conn) {
    char header_host[BUFFER_SIZE];

    // Jeśli w hoście mamy ":", to jest to IPv6.
    bool is_ip_v6 = (url.host.find(":") != std::string::npos);

    // Budujemy nowy nagłówek Host, dla adresów IPv6 muszą być w nawiasach.
    if (is_ip_v6)
        snprintf(header_host, sizeof(header_host), "[%s]", url.host.c_str());
    else
        snprintf(header_host, sizeof(header_host), "%s", url.host.c_str());

    // Dodajemy wartości wcześniejszych cookies do wysłania żądania.
    std::string cookies = "";
    if (!config.cookies.empty()) {
        cookies += "Cookie: " ;
        bool first = true;
        for (auto& cookie : config.cookies) {
            if (!first)
                cookies += "; ";
            cookies += cookie.first;
            if (!cookie.second.empty()) {
                cookies += "=" + cookie.second;
            }
            first = false;
        }
        cookies += "\r\n";

    }

    char request[BUFFER_SIZE];
    int req_len = snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\n"
                            "Host: %s\r\n"
                            "Connection: Keep-Alive\r\n"
                            "%s"
                            "%s"
                            "\r\n", url.path.c_str(), header_host, cookies.c_str(), config.multiplex ? "Icy-MetaData: 1\r\n" : "");

    log_msg(COMMUNICATION_LOGS, request, config);
    writen_with_proper_encr(request, req_len, conn);
}

// Zamykamy połączenie (odpowiednio, gdy mamy SSL lub nie mamy) i zwalniamy zasoby.
void close_connection(connection_t &conn) {
    if (conn.is_ssl) {
        if (conn.ssl) {
            SSL_shutdown(conn.ssl);
            SSL_free(conn.ssl);
            conn.ssl = NULL;
        }
        if (conn.ctx) {
            SSL_CTX_free(conn.ctx);
            conn.ctx = NULL;
        }
    }
    if (conn.fd >= 0) {
        close(conn.fd);
        conn.fd = -1;
    }
}

// Zamykamy stare połączenie, resetujemy stan i łączymy się na nowo.
connection_t reconnect(url_t &url, config_t &config, stream_state_t &stream_state, connection_t &conn) {
    close_connection(conn);
    clean_stream_state(stream_state);
    connection_t new_conn = connect_to_server(url, config);
    send_http_request(url, config, new_conn);
    return new_conn;
}

// Odbiór danych niezależnie od tego, czy mamy szyfrowanie, czy nie.
ssize_t read_with_proper_encr(connection_t conn, char *buf, size_t len) {
    if (conn.is_ssl) {
        return SSL_read(conn.ssl, buf, len);
    }
    else {
        return read(conn.fd, buf, len);
    }
}

