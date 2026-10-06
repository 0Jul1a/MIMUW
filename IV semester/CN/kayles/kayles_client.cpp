#include <iostream>
#include "kayles.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include "err.h"
#include "common.h"

using namespace std;

// Tworzymy odpowiednią wiadomość do wysłania.
// Zwracamy długość stworzonej wiadomości, a do new_created_msg zapisujemy odpowiednio wartości tej wiadomości.
ssize_t create_msg(char *msg_s, uint8_t *new_created_msg) {
    char *msg[MAX_MSG_SIZE];
    int count = split_msg(msg_s, msg, MAX_MSG_SIZE);      // Obliczamy ile dostaliśmy argumentów (max. 5, jeśli za dużo).

    // Minimalnie 2 argumenty: msg_type, player_id.
    // Maksymalnie 4 argumenty: msg_type, player_id, game_id, pawn.
    if (count < 2 || count > 4)
        return -1;

    uint8_t msg_type, pawn;
    uint32_t player_id, game_id;

    // Wyciągamy msg_type oraz player_id.
    if (!parse_8(msg[0], &msg_type) || !parse_32(msg[1], &player_id))
        return -1;

    // msg_type niepoprawny.
    if (msg_type > MSG_GIVE_UP)
        return -1;

    // Zapisujemy msg_type w nowej, stworzonej wiadomości.
    ssize_t offset = 0;
    new_created_msg[offset++] = msg_type;

    uint32_t player_a_id = htonl(player_id);
    memcpy(new_created_msg + offset, &player_a_id, sizeof(player_a_id));

    offset += sizeof(player_a_id);

    // Zależnie od rodzaju msg_type parsujemy i zapisujemy odpowiednie argumenty (game_id, pawn).
    if (msg_type == MSG_JOIN)
        return offset;              // Jeśli MSG_JOIN to już zwracamy, bo player_id i msg_type już ustawione.

    // Dla reszty msg_type musi być co najmniej 3 argumenty: poprawna wartość game_id.
    if (count < 3)
        return -1;
    if (!parse_32(msg[2], &game_id))
        return -1;

    // Konwertujemy i zapisujemy wartość game_id.
    uint32_t game_a_id = htonl(game_id);
    memcpy(new_created_msg + offset, &game_a_id, sizeof(game_a_id));
    offset += sizeof(game_a_id);

    // Jeśli mamy podane niżej msg_type, to możemy już zwrócić poprawnie stworzoną wiadomość i jej długość.
    if (msg_type == MSG_KEEP_ALIVE || msg_type == MSG_GIVE_UP) {
        return offset;
    }

    // msg_type == któryś z move, musimy sprawdzić ilość argumentów oraz odebrać jeszcze pawn i dodać do wiadomości.
    if (msg_type == MSG_MOVE_1 || msg_type == MSG_MOVE_2) {
        if (count < 4)
            return -1;
        if (!parse_8(msg[3], &pawn))
            return -1;
        new_created_msg[offset++] = pawn;
        return offset;
    }
    return -1;

}

// Funkcja do wypisania planszy odebranej od serwera w poprawnej wiadomości.
void print_good_msg( uint8_t msg[]) {
    uint32_t game_id, player_a_id, player_b_id;
    uint8_t status, max_pawn;

    memcpy(&game_id, msg , sizeof(game_id));
    memcpy(&player_a_id, msg + 4, sizeof(player_a_id));
    memcpy(&player_b_id, msg + 8, sizeof(player_b_id));

    game_id = ntohl(game_id);
    player_a_id = ntohl(player_a_id);
    player_b_id = ntohl(player_b_id);
    status = msg[STATUS_INDEX];
    max_pawn = msg[13];

    size_t pawn_row_size = (max_pawn / 8)  +1;
    vector<uint8_t> pawn_row(pawn_row_size, 0);
    memcpy(pawn_row.data(), msg + 14, pawn_row_size);

    printf("game_id: %u\n", game_id);
    printf("player_a_id: %u\n", player_a_id);
    printf("player_b_id: %u\n", player_b_id);
    printf("status: %u\n", status);
    printf("max_pawn: %u\n", max_pawn);
    printf("pawn_row: ");

    for (int i = 0; i <= max_pawn; i++)
        printf("%d", get_pawn_to_print(pawn_row.data(), i));
    printf("\n");

}

// Funkcja do wypisania odpowiednich wartości błędu po dostaniu odpowiedzi od serwera MSG_WRONG_MSG.
void print_wrong_msg(uint8_t msg[]) {
    uint8_t status = msg[STATUS_INDEX];
    uint8_t error_index = msg[14];
    printf("ERROR\n");
    printf("Status: %d\n", status);
    printf("Error index: %d\n", error_index);
}

int main(int argc, char *argv[]) {

    char *address = {nullptr}, *message = {nullptr};
    uint16_t port = 0;
    int client_timeout, opt;
    char message_cpy[BUFFER_SIZE];
    bool received_a = false, received_p = false, received_m = false, received_t = false;
    // Pobieramy argumenty i sprawdzamy ich poprawność.
    while ((opt = getopt(argc, argv, "a:p:m:t:")) != -1) {
        switch (opt) {
            case 'a':
                if (!validate_address(optarg))
                    fatal("address failed");

                address = optarg;
                received_a = true;
                break;

            case 'p':
                port = read_port(optarg, 1);
                received_p = true;
                break;

            case 'm':
                strncpy(message_cpy, optarg, BUFFER_SIZE);
                message_cpy[BUFFER_SIZE - 1] = '\0';

                if (!validate_message(message_cpy))
                    fatal("message failed");

                received_m = true;
                break;

            case 't':
                if (!validate_timeout(optarg, &client_timeout))
                    fatal("timeout failed");

                received_t = true;
                break;

            default:
                fatal("invalid argument");
                break;

        }
    }

    // Nie dostaliśmy wszystkich wymaganych argumentów.
    if (!received_p || !received_a || !received_m || !received_t ) {
        fatal("Missing arguments");
    }

    message = message_cpy;
    sockaddr_in server_address = get_server_address(address, port);

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        syserr("cannot create a socket");
    }

    // Tworzymy wiadomość.
    uint8_t msg[BUFFER_SIZE];
    ssize_t msg_size = create_msg(message, msg);

    if (msg_size == -1) {
        syserr("cannot create a message, wrong arguments");
    }

    // Wysyłamy wiadomość z ruchem do serwera.
    ssize_t sent_len = sendto(socket_fd, msg, msg_size, 0, (struct sockaddr *) &server_address, sizeof(server_address));
    if (sent_len < 0) {
        syserr("cannot send message");
    }

    memset(msg, 0, sizeof(msg));
    struct sockaddr_in receive_address{};
    socklen_t address_length = (socklen_t) sizeof(receive_address);

    // Ustawiamy timeout na ten podany w argumencie.
    struct timeval timeout;
    timeout.tv_sec = client_timeout;
    timeout.tv_usec = 0;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
         syserr("setsockopt failed");
    }

    ssize_t received_length = recvfrom(socket_fd, msg, BUFFER_SIZE, 0,
                                           (struct sockaddr *) &receive_address, &address_length);
    // Sprawdzamy, czy wystąpił błąd podczas odbierania i czy to kwestia timeoutu.
    if (received_length < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("client timed out");
        } else {
            syserr("receive fail");
        }
    }

    // Odebrano poprawnie wiadomość zwrotną.
    if (received_length > 0) {

        // W msg[STATUS_INDEX] niezależnie od wiadomości jest status, STATUS == 255 to MSG_WRONG_MSG/
        if (msg[STATUS_INDEX] == MSG_WRONG_MSG)
            print_wrong_msg(msg);
        else
            print_good_msg(msg);
    }

    close(socket_fd);
    return 0;
}

