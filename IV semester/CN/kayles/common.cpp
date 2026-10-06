#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <inttypes.h>
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "common.h"

// Serwer ma porty od 0, a klient od 1, dlatego ustawiamy wartość "client" na odpowiednio 0/1.
// Funkcja z zajęć.
uint16_t read_port(char const *string, int client) {
    char *endptr;
    errno = 0;
    long port = strtol(string, &endptr, 10);
    if (errno != 0 || *endptr != 0  || port < client || port > UINT16_MAX) {
        fatal("%s is not a valid port number", string);
    }
    return (uint16_t) port;
}

// Sprawdzenie poprawności podanego układu pionów.
bool validate_pawn_row(const char *pawn_row) {
    if (!pawn_row)
        return false;

    size_t len = strlen(pawn_row);
    // Sprawdzamy, czy jest odpowiednia długość.
    if (len < MIN_SIZE_PAWN_ROW || len > MAX_SIZE_PAWN_ROW)
        return false;

    // Sprawdzamy, czy pierwszy i ostatni pion jest ustawiony.
    if (pawn_row[0] != '1' || pawn_row[len - 1] != '1')
        return false;

    // Sprawdzamy, czy wszystkie pozostałe piony mają wartość 0/1.
    for (size_t i = 1; i < len; i++) {
        if (pawn_row[i] != '1' && pawn_row[i] != '0')
            return false;
    }
    return true;
}

// Sprawdzenie poprawności wartości timeout, jeśli poprawna to przypisujemy wartość z argumentu do timeout.
bool validate_timeout(char const *string, int *timeout) {
    if (!string || *string == '\0')
        return false;

    char *endptr;
    errno = 0;
    long timeout_long = strtol(string, &endptr, 10);

    // Sprawdzamy, czy nie było overflow, nie ma dodatkowych znaków oprócz liczby oraz czy liczba jest z odpowiedniego przedziału.
    if (errno != 0 || *endptr != '\0' || timeout_long < 1 || timeout_long > TIMEOUT_MAX) {
        fatal("%s is not a valid timeout", string);
    }

    *timeout = (int)timeout_long;
    return true;
}

// Funkcja pomocnicza do obliczenia ilości segmentów "x/" oraz odpowiedniego przypisania tych segmentów do msg.
int split_msg(char *string, char *msg[], int max_parts) {
    int count = 0;
    char *split = strtok(string, "/");

    while (split != NULL && count < max_parts) {
        msg[count++] = split;
        split = strtok(NULL, "/");
    }

    return count;
}

// Wyciągamy argument konwertując go na uint32_t.
bool parse_32(const char *string, uint32_t *s) {
    if (!string || *string == '\0')
        return false;

    char *endptr;
    errno = 0;
    long val = strtol(string, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || val < 0 || val > UINT32_MAX)
        return false;

    *s = (uint32_t)val;
    return true;
}

// Wyciągamy argument konwertując go na uint8_t.
bool parse_8(const char *string, uint8_t *s) {
    if (!string || *string == '\0')
        return false;

    char *endptr;
    errno = 0;
    long val = strtol(string, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || val < 0 || val > UINT8_MAX )
        return false;

    *s = (uint8_t)val;
    return true;
}

// Sprawdzamy poprawność podanej wiadomości z poleceniem do gry zgodnie z wytycznymi z polecenia.
bool validate_message(char *string) {

    // Specjalne przypadki błędów.
    if (!string || strlen(string) == 0 || strlen(string) >= BUFFER_SIZE)
        return false;

    if (strstr(string, "//") != NULL || string[0] == '/' ||string[strlen(string) - 1] == '/' )
        return false;

    char tmp[BUFFER_SIZE];
    strncpy(tmp, string, BUFFER_SIZE);
    tmp[BUFFER_SIZE - 1] = '\0';

    char *msg[MAX_MSG_SIZE];
    int count = split_msg(tmp, msg, MAX_MSG_SIZE);

    // Jeśli podzielono na więcej niż 4 kawałki, to wiadomość jest niepoprawna.
    if (count > 4)
        return false;

    uint8_t msg_type, pawn;
    uint32_t player_id, game_id;

    // Sprawdzamy poprawność przypadku bazowego dla wszystkich komunikatów, czyli pozyskanie msg_type i player_id.
    if (count < 2 || !parse_8(msg[0], &msg_type) || !parse_32(msg[1], &player_id))
        return false;

    // player_id to wartość większa lub równa 1, więc musimy to osobno sprawdzić.
    if (player_id < 1)
        return false;

    switch (msg_type) {
        case MSG_JOIN:
            // Jeśli już doszliśmy tutaj, to jedyny warunek, to odpowiednia wartość count.
            return count == 2;

        case MSG_MOVE_1:
        case MSG_MOVE_2: {
            // Sprawdzamy, czy poprawna ilość segmentów oraz, czy nie było błędu przy konwersji game_id i pawn.
            if (count != 4 || !parse_32(msg[2], &game_id) || !parse_8(msg[3], &pawn))
                return false;
            return true;
        }

        case MSG_KEEP_ALIVE:
        case MSG_GIVE_UP: {
            // Sprawdzamy, czy mamy poprawną liczbę argumentów i poprawną wartość game_id.
            if (count != 3 || !parse_32(msg[2], &game_id))
                return false;
            return true;
        }

        default:
            return false;
    }

}

// Sprawdzenie poprawności adresu.
bool validate_address(char const *string) {
    if (!string || *string == '\0')
        return false;

    struct addrinfo hints = {}, *res;
    hints.ai_family = AF_INET;              // IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(string, NULL, &hints, &res) != 0) {
        return false;
    }

    freeaddrinfo(res);
    return true;
}

// Funkcja z zajęć.
struct sockaddr_in get_server_address(char const *host, uint16_t port) {
    struct addrinfo hints{};
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *address_result;
    int errcode = getaddrinfo(host, NULL, &hints, &address_result);
    if (errcode != 0) {
        fatal("getaddrinfo: %s", gai_strerror(errcode));
    }

    struct sockaddr_in send_address{};
    send_address.sin_family = AF_INET;   // IPv4
    send_address.sin_addr.s_addr =       // IP address
            ((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr;
    send_address.sin_port = htons(port); // port from the command line

    freeaddrinfo(address_result);

    return send_address;
}


