#ifndef SIKRADIO_H
#define SIKRADIO_H

#include <string>
#include <map>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 4096

// Wartości odpowiadające verbosity.
#define COMMUNICATION_LOGS      1
#define CRITICAL_ERRORS_LOGS    2
#define NONCRITICAL_ERRORS_LOGS 3
#define DEBUG_INFO_LOGS         4

// Długości odpowiednich słów.
#define ICY_METAINT_LEN 12              // icy-metaint:
#define SET_COOKIE_LEN  11              // set-cookie:
#define LOCATION_LEN    9               // location:

typedef struct {
    std::string scheme;
    std::string host;
    uint16_t port;
    std::string path;
} url_t;

typedef struct config_t {
    std::string url;
    url_t original_url;                  // Oryginalny url, potrzebny, przy reconnect po timeout.
    bool multiplex = false;
    int timeout = 5000;
    int ip_version = AF_UNSPEC;          // Wymuszenie IPv4/IPv6 bądź wybranie pierwszego dostępnego.
    int verbosity = 2;                   // Wartość z zakresu [0,4], domyślnie = 2, oznaczająca zakres informacji wypisywanych na standardowe wyjście diagnostyczne.
    std::map<std::string, std::string> cookies;
} config_t;

// Struktura opisująca aktualny stan przepływu danych.
typedef struct stream_state_t {

    bool header_done = false;            // Informacja, czy odebrano i przetworzono już cały nagłówek.
    std::string header;                  // Bufor do zapisu nagłówku.

    size_t icy = 0;                      // Wartość nagłówka icy-metaint: ile bajtów audio między metadanymi.
    size_t bytes_until_metadata = 0;     // Licznik pozostałych bajtów audio do wczytania przed kolejnym blokiem metadanych.
    bool reading_metadata = false;       // Informacja, czy aktualnie przetwarzamy metadane.
    size_t metadata_size_to_read = 0;    // Rozmiar obecnego bloku metadanych do odczytu.
    std::string metadata;                // Bufor do zapisu odbieranych metadanych.
} stream_state_t;

typedef struct {
    bool is_ssl;
    int fd;
    SSL_CTX *ctx;
    SSL *ssl;
} connection_t;

// Funkcja pomocnicza do wypisywania odpowiednich komunikatów, ze względu na verbosity na standardowe wyjście diagnostyczne.
void log_msg(int verbosity, const char *msg, const config_t& config);

#endif

