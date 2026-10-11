#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <csignal>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "err.h"
#include "network.h"
#include "sikradio.h"

// Funkcja wypisująca odpowiedni komunikat w zależności od wartości verbosity.
void log_msg(int verbosity, const char *msg, const config_t& config) {
    if (config.verbosity >= verbosity)
        fprintf(stderr, "%s\n", msg);
}

// Rozbijanie podanego adresu url na: schemat, host, port i ścieżkę.
url_t parse_url(const std::string& url, config_t &config) {
    url_t res;

    // Szukamy http/https, czyli miejsca, gdzie mamy "://".
    size_t scheme_end = url.find("://");
    if (scheme_end == std::string::npos) {
        log_msg(CRITICAL_ERRORS_LOGS, "missing scheme in url", config);
        exit(1);
    }

    res.scheme = url.substr(0, scheme_end);

    size_t host_begin = scheme_end + 3;                                 // Wartość 3, ponieważ "://" to 3 znaki, a o tyle musimy się przesunąć.
    size_t path_begin = url.find('/', host_begin);

    std::string host;

    // Oddzielamy hosta od ścieżki, jeśli nie mamy "/", to zakładamy domyślną wartość ścieżki: "/".
    if (path_begin != std::string::npos) {
        host = url.substr(host_begin, path_begin - host_begin);
        res.path = url.substr(path_begin);
    }
    else {
        host = url.substr(host_begin);
        res.path = "/";
    }

    // Oddzielna obsługa adresów IPv6, które są otoczone w nawiasy kwadratowe.
    if (!host.empty() && host[0] == '[') {
        size_t end = host.find(']');
        if (end == std::string::npos) {
            log_msg(CRITICAL_ERRORS_LOGS, "invalid host in url", config);
            exit(1);
        }
        res.host = host.substr(1, end - 1);                       // Usuwamy nawiasy z przodu i tyłu i pobieramy wartość.
        if (end + 1 < host.size() && host[end + 1] == ':') {            // Sprawdzamy, czy po "]" mamy zdefiniowany port.
            res.port = read_port(host.substr(end + 2).c_str());
        }
        else {                                                          // Jeśli nie mamy, to ustawiamy odpowiedni port domyślny.
            if (res.scheme == "https") {
                res.port = HTTPS_PORT;
            }
            else if (res.scheme == "http") {
                res.port = HTTP_PORT;
            }
            else {
                log_msg(CRITICAL_ERRORS_LOGS, "wrong scheme in url", config);
                exit(1);
            }
        }
    }
    else { // Obsługa standardowych adresów i domen.
        // Szukamy, czy podano port.
        size_t port = host.find(':');

        if (port != std::string::npos) {                                // Wyciągamy wartość host i port.
            res.host = host.substr(0, port);
            res.port = read_port(host.substr(port + 1).c_str());
        }
        else {                                                          // Nie ma podanego portu, ustawiamy domyślne.
            res.host = host;
            if (res.scheme == "https")
                res.port = HTTPS_PORT;
            else if (res.scheme == "http")
                res.port = HTTP_PORT;
            else {
                log_msg(CRITICAL_ERRORS_LOGS, "unsupported scheme in url", config);
                exit(1);
            }
        }
    }

    return res;
}

// Sprawdzenie poprawności wartości argumentu, jeśli poprawny to przypisujemy wartość z argumentu do arg_to_save.
bool validate_arg(char const *string, int *arg_to_save, int min_lim, int max_lim) {
    if (!string || *string == '\0')
        return false;

    char *endptr;
    errno = 0;
    long arg = strtol(string, &endptr, 10);

    // Sprawdzamy, czy nie było overflow, nie ma dodatkowych znaków oprócz liczby oraz czy liczba jest z odpowiedniego przedziału.
    if (errno != 0 || *endptr != '\0' || arg < min_lim || arg > max_lim) {
        fatal("%s is not a valid timeout", string);
    }

    *arg_to_save = (int) arg;
    return true;
}

// Obsługa przekierowań HTTP (statusy 3xx). Wyciągamy nagłówek "location", parsujemy ten nowy url i łączymy się na nowo.
void redirect(std::string &header, connection_t &new_conn, url_t &url, config_t &config, stream_state_t &stream_state) {
    // Dla bezpieczeństwa zamiana wszystkich liter na małe.
    std::string header_lower = header;
    std::transform(header_lower.begin(), header_lower.end(), header_lower.begin(), tolower);

    size_t loc = header_lower.find("location:");

    if (loc == std::string::npos) {
        log_msg(CRITICAL_ERRORS_LOGS, "Redirect status, but can not find location", config);
        exit(1);
    }

    size_t start = loc + LOCATION_LEN;

    // Pomijamy ewentualne znaki białe między ":", a nowym url.
    while (start < header.size() && (header[start] == ' ' || header[start] == ':')) {
        start++;
    }

    size_t end = header.find("\r\n", start);
    std::string new_url = header.substr(start, end - start);

    // Aktualizujemy nowy url i łączymy się ponownie.
    url = parse_url(new_url, config);
    new_conn = reconnect(url, config, stream_state, new_conn);
}

// Wyciągnięcie "cookies" z odpowiedzi serwera i zapisanie ich do "config" w celu ich późniejszego odesłania.
void get_cookies(std::string &header, config_t &config) {
    std::string header_lower = header;
    std::transform(header_lower.begin(), header_lower.end(), header_lower.begin(), tolower);
    size_t pos = 0;

    // Może być więcej niż jeden nagłówek z cookies.
    while ((pos = header_lower.find("set-cookie:", pos)) != std::string::npos) {
        size_t start = pos + SET_COOKIE_LEN;
        while (start < header.size() && (header[start] == ' ' || header[start] == ':')) {
            start++;
        }

        size_t end = header.find("\r\n", start);
        if (end == std::string::npos) {
            end = header.size();
        }

        std::string cookie = header.substr(start, end - start);

        // Bierzemy tylko wartości "cookies", bez innych parametrów, które są oddzielone średnikiem.
        size_t semic_pos = cookie.find(';');
        if (semic_pos != std::string::npos) {
            cookie = cookie.substr(0, semic_pos);
        }

        // Nadpisujemy stare wartości.
        size_t eq_pos = cookie.find('=');
        if (eq_pos != std::string::npos) {
            std::string name = cookie.substr(0, eq_pos);
            std::string value = cookie.substr(eq_pos + 1);
            config.cookies[name] = value;
        } else {
            config.cookies[cookie] = "";
        }

        pos = end;
    }
}

// Parsujemy otrzymany nagłówek. Zwracamy true, jeśli cały nagłówek został dobrze przetworzony i nie było redirectu.
bool process_header(char *&buff_ptr, ssize_t &bytes_left, stream_state_t &stream_state, config_t &config, connection_t &conn, url_t &url) {
    stream_state.header.append(buff_ptr, bytes_left);

    // Szukamy końca nagłówka.
    size_t header_pos_end = stream_state.header.find("\r\n\r\n");
    size_t sep_len = 4;
    if (header_pos_end == std::string::npos) {
        header_pos_end = stream_state.header.find("\n\n");
        sep_len = 2;
    }

    // Nie znaleziono pustej linii, czyli nie mamy całego nagłówka, więc musimy czekać na reszte danych.
    if (header_pos_end == std::string::npos) {
        return false;
    }
    stream_state.header_done = true;

    std::string header_str = stream_state.header.substr(0, header_pos_end + sep_len);
    log_msg(COMMUNICATION_LOGS, header_str.c_str(), config);
    get_cookies(stream_state.header, config);
    int status = 0;

    // Pobieramy pierwszą linijkę nagłówka, kod odpowiedzi.
    if (sscanf(stream_state.header.c_str(), "HTTP/%*s %d", &status) != 1
            && sscanf(stream_state.header.c_str(), "ICY %d", &status) != 1) {

        fprintf(stderr, "failed to parse HTTP request\n");
        exit(1);
    }

    // Mamy przekierowanie.
    if (status == MOVED_PERMANENTLY || status == MOVED_TEMPORARILY || status == REDIRECTED
        || status == SEE_OTHER || status == PERMANENT_REDIRECTED || status == MULTIPLE_CHOICES ) {
        redirect(stream_state.header, conn, url, config, stream_state);
        return false;                           // Mamy nowe połączenie, czyli nowy nagłówek do odczytania, dlatego false.
    }


    if (status != 200) {
        log_msg(CRITICAL_ERRORS_LOGS, "Server did not return status 200 OK", config);
        exit(1);
    }

    // Jeśli została ustawiona opcja multiplex, to szukamy nagłówka "icy-metaint:", aby wiedzieć co ile bajtów audio są metadane
    if (config.multiplex) {
        std::string header_lower = stream_state.header;
        std::transform(header_lower.begin(), header_lower.end(), header_lower.begin(), tolower);
        size_t pos = header_lower.find("icy-metaint:");

        if (pos == std::string::npos) {
            log_msg(NONCRITICAL_ERRORS_LOGS, "Server did not return icy_metaint:", config);
            config.multiplex = false;           // Błąd, serwer nie wspiera metadanych.
        }
        else {
            size_t start = pos + ICY_METAINT_LEN;
            size_t end = stream_state.header.find("\r\n", start);
            std::string data = stream_state.header.substr(start, end - start);
            stream_state.icy = atoi(data.c_str());
            stream_state.bytes_until_metadata = stream_state.icy;       // Ustawiamy początkowy licznik bajtów do przeskoczenia.
        }
    }

    // Obliczamy ile bajtów nagłówka już mieliśmy przed obecną, nową porcją danych.
    size_t already_before = stream_state.header.size() - bytes_left;

    // W którym miejscu obecnego bufora zaczynają się właściwe dane.
    ssize_t body_offset_in_current = 0;

    // Jeśli koniec nagłówka wypada wewnątrz nowo odczytanych danych, to musimy obliczyć gdzie zaczęły się dane.
    if (header_pos_end + sep_len > already_before)
        body_offset_in_current = (header_pos_end + sep_len) - already_before;
    else
        body_offset_in_current = 0;

    // Przesuwamy wskaźnik bufora tak, aby omijał nagłówek i wskazywał prosto na audio
    // oraz redukujemy liczbę dostępnych bajtów o te, które były jeszcze w nagłówku.
    buff_ptr +=  body_offset_in_current;
    bytes_left -= body_offset_in_current;
    return true;
}

// Przechwytujemy dane i obsługujemy audio, a jeśli trzeba to metadane.
void process_data(char* buff_ptr, ssize_t &bytes_left, stream_state_t &stream_state, config_t &config) {
    while (bytes_left > 0) {

        // Jeśli nie musimy przetwarzać metadanych, to od razu możemy przetworzyć audio.
        if (!config.multiplex) {
            write(STDOUT_FILENO, buff_ptr, bytes_left);
            bytes_left = 0;
            break;
        }

        // Czytamy dane audio.
        if (!stream_state.reading_metadata) {

            // Jeszcze możliwość odczytu audio.
            if (stream_state.bytes_until_metadata > 0) {

                // Nie możemy zapisać więcej niż zostało do kolejnego bloku metadanych.
                size_t to_write = std::min((size_t) bytes_left, stream_state.bytes_until_metadata);
                write(STDOUT_FILENO, buff_ptr, to_write);

                buff_ptr += to_write;
                bytes_left -= to_write;
                stream_state.bytes_until_metadata -= to_write;
            }

            // Jeśli bytes_until_metadata == 0 oraz dalej mamy dane do odczytu, to dostaliśmy teraz informację o długości metadanych.
            if (stream_state.bytes_until_metadata == 0 && bytes_left > 0) {
                uint8_t len = (uint8_t) *buff_ptr;
                stream_state.metadata_size_to_read = len * 16;
                buff_ptr++;
                bytes_left--;
                stream_state.reading_metadata = true;
                stream_state.metadata.clear();

                // Dostaliśmy pusty blok.
                if (stream_state.metadata_size_to_read == 0) {
                    stream_state.reading_metadata = false;
                    stream_state.bytes_until_metadata = stream_state.icy;   // Resetujemy licznik i wracamy do odczytu audio.
                }
            }
        }
        else {
            // Odczytujemy metadane.
            size_t to_read = std::min((size_t) bytes_left,stream_state.metadata_size_to_read - stream_state.metadata.size());
            stream_state.metadata.append(buff_ptr, to_read);
            buff_ptr += to_read;
            bytes_left -= to_read;

            // Skończyliśmy przetwarzać cały blok metadanych.
            if (stream_state.metadata.size() == stream_state.metadata_size_to_read) {

                // Przetworzyliśmy metadane, więc je wypisujemy.
                if (!stream_state.metadata.empty()) {
                    fprintf(stderr, "%s\n", stream_state.metadata.c_str());
                }

                // Koniec czytania metadanych, ustawiamy odpowiednie wartości i wracamy do odczytu audio.
                stream_state.reading_metadata = false;
                stream_state.bytes_until_metadata = stream_state.icy;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    config_t config;
    int opt;
    bool received_url = false, ip_v4_flag = false, ip_v6_flag = false;

    while ((opt = getopt(argc, argv, "u:mt:46v:q")) != -1) {
        switch (opt) {
            case 'u':
                config.url = optarg;
                received_url = true;
                break;

            case 'm':
                config.multiplex = true;
                break;

            case 't': {
                if (!validate_arg(optarg, &config.timeout, 100, 100000))
                    fatal("%s is not a valid timeout", optarg);
                break;
            }
            case '4':
                ip_v4_flag = true;
                break;
            case '6':
                ip_v6_flag = true;
                break;
            case 'v': {
                if (!validate_arg(optarg, &config.verbosity, 0, 4))
                    fatal("%s is not a valid verbosity", optarg);
                break;
            }
            case 'q':
                config.verbosity = 0;
                break;
            default:
                fatal("Unknown option '%c'", optopt);
                break;
        }
    }

    // Nie dostaliśmy wymaganego argumentu.
    if (!received_url)
        fatal("Missing required argument");

    // Określamy wersje protokołu IP.
    if ((ip_v4_flag && ip_v6_flag) || (!ip_v4_flag && !ip_v6_flag)) {
        config.ip_version = AF_UNSPEC;
    }
    else {
        if (ip_v4_flag)
            config.ip_version = AF_INET;
        if (ip_v6_flag)
            config.ip_version = AF_INET6;
    }

    // Parsujemy uzyskany url i łączymy się z serwerem, a następnie wysyłamy do serwera HTTP GET.
    url_t url = parse_url(config.url, config);
    connection_t conn = connect_to_server(url, config);
    config.original_url = url;
    send_http_request(url, config, conn);

    struct pollfd poll_descriptors[2];
    poll_descriptors[0].fd = STDIN_FILENO;      // Nasłuch klawiatury dla wykrycia polecenia "quit".
    poll_descriptors[0].events = POLLIN;
    poll_descriptors[1].fd = conn.fd;           // Nasłuch danych z serwera.
    poll_descriptors[1].events = POLLIN;

    char input[BUFFER_SIZE];
    std::string stdin_buffer;
    stream_state_t stream_state;
    clean_stream_state(stream_state);

    while (true) {

        // Timeout z argumentu (-t) dotyczy tylko odbierania danych.
        int timeout = stream_state.header_done ? config.timeout : -1;
        int poll_status = poll(poll_descriptors, 2, timeout);

        if (poll_status == -1 ) {
            if (errno == EINTR) {
                log_msg(DEBUG_INFO_LOGS, "interrupted system call", config);
                continue;
            }
            else {
                log_msg(CRITICAL_ERRORS_LOGS, "poll", config);
                exit(1);
            }
        }

        // Obsługa timeout.
        if (poll_status == 0) {
            log_msg(COMMUNICATION_LOGS, "data receiving timeout", config);
            config.cookies.clear();
            url = config.original_url;      // Powrót do oryginalnego url.
            conn = reconnect(url, config, stream_state, conn);
            poll_descriptors[1].fd = conn.fd;
            continue;
        }

        // Czytamy z klawiatury, jeśli "quit" kończymy.
        if (poll_descriptors[0].fd != -1 && (poll_descriptors[0].revents & (POLLIN | POLLHUP))) {
            ssize_t bytes_read = read(STDIN_FILENO, input, sizeof(input) - 1);
            if (bytes_read > 0) {
                stdin_buffer.append(input, bytes_read);

                // Czekamy na quit w dowolnym momencie.
                if (stdin_buffer.find("quit\n") != std::string::npos) {
                    close_connection(conn);
                    exit(0);
                }

            }
            else if (bytes_read == 0) {   // EOF na klawiaturze, musimy ustawić wartość deskryptora na -1.
                log_msg(COMMUNICATION_LOGS, "Interrupted keybord", config);
                poll_descriptors[0].fd = -1;
            }
        }

        // Czytamy z serwera.
        if (poll_descriptors[1].revents & (POLLIN | POLLHUP)) {
            char buff[BUFFER_SIZE];
            ssize_t bytes = read_with_proper_encr(conn, buff, sizeof(buff));

            if (bytes < 0) {
                if (errno == EINTR) {
                    continue;
                }

                log_msg(CRITICAL_ERRORS_LOGS, "cannot receive HTTP request", config);
                exit(1);
            }

            // Serwer zamknął połączenie.
            if (bytes == 0) {
                log_msg(COMMUNICATION_LOGS, "Server closed connection", config);
                close_connection(conn);

                // Serwer zamknął połączenie przed końcem wysłania nagłówka, błąd.
                if (!stream_state.header_done)
                    exit(1);

                // Serwer przerwał w trakcie czytania metadanych.
                if (stream_state.reading_metadata)
                    exit(1);

                exit(0);
            }

            char *buff_ptr = buff;
            ssize_t bytes_left = bytes;

            // Jeśli nie skończyliśmy parsować otrzymanego nagłówka, to przekazujemy nowe dane, do kontynuowania parsowania.
            if (!stream_state.header_done) {
                bool header_status = process_header(buff_ptr, bytes_left, stream_state, config, conn, url);
                poll_descriptors[1].fd = conn.fd;

                // Nadal nie przetworzyliśmy całego nagłówka.
                if (!header_status) {
                    continue;
                }
            }

            // Nagłówek jest gotowy i przetworzony, odbieramy dane.
            if (stream_state.header_done && bytes_left > 0) {
                process_data(buff_ptr, bytes_left, stream_state, config);
            }

        }
    }
    close_connection(conn);
    return 0;

}

