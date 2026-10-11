#include <iostream>
#include <unordered_map>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ctime>

#include "err.h"
#include "kayles.h"
#include "common.h"

using namespace std;

// Dla MSG_WRONG_MSG: 12 komunikatu + status (1) + error_index (1)
// Dla dobrego komunikatu: game_id (4) + player_a/b_id (4 + 4) + status (1) + max_pawn (1)
#define BASIC_MSG_LEN 14
#define ERROR 255
#define BASIC_ID_SIZE 4
#define MINIMAL_ARGS_LEN 5              // msg_type (1) + player_id (4)
#define MINIMAL_ARGS_LEN_REST_TYPE 9    // msg_type (1) + player_id (4) + game_id (4)
#define MINIMAL_ARGS_LEN_MOVE 10        // msg_type (1) + player_id (4) + game_id (4) + pawn (1)

uint32_t game_id_g = 0;
unordered_map<uint32_t, game_state_t> games;

int compare_timespec(struct timespec a, struct timespec b) {
    if (a.tv_sec < b.tv_sec)
        return -1;
    if (a.tv_sec > b.tv_sec)
        return 1;
    if (a.tv_nsec < b.tv_nsec)
        return -1;
    if (a.tv_nsec > b.tv_nsec)
        return 1;
    return 0;
}

void send_wrong_msg(int sock, sockaddr_in &client, const char *buff, ssize_t len, uint8_t error_index) {
    uint8_t resp[BASIC_MSG_LEN];
    memset(resp, 0, BASIC_MSG_LEN);
    size_t copy_len = (len > 12 ? 12 : len);
    memcpy(resp + 0, buff, copy_len);
    resp[STATUS_INDEX] = ERROR;
    resp[13] = error_index;
    sendto(sock, resp, BASIC_MSG_LEN, 0, (struct sockaddr *)&client, sizeof(client));
}

void send_state(int sock, sockaddr_in &client, uint32_t game_id) {
    uint8_t max_pawn = games[game_id].max_pawn;
    size_t pawn_row_size = (max_pawn / 8) + 1;

    uint8_t resp[BASIC_MSG_LEN + MAX_PAWN_ROW_SIZE] = {};

    uint32_t game_id_ = htonl(game_id);
    uint32_t player_a = htonl(games[game_id].player_a_id);
    uint32_t player_b = htonl(games[game_id].player_b_id);

    memcpy(resp + 0, &game_id_, BASIC_ID_SIZE);
    memcpy(resp + 4, &player_a, BASIC_ID_SIZE);
    memcpy(resp + 8, &player_b, BASIC_ID_SIZE);

    resp[STATUS_INDEX] = games[game_id].status;
    resp[13] = games[game_id].max_pawn;

    memcpy(resp + BASIC_MSG_LEN, games[game_id].pawn_row, pawn_row_size);
    sendto(sock, resp, BASIC_MSG_LEN + pawn_row_size, 0,
           (sockaddr*)&client, sizeof(client));

}

bool is_timeout(struct timespec start, struct timespec now, struct timespec server_timeout) {

    long diff_sec = now.tv_sec - start.tv_sec;
    long diff_nsec = now.tv_nsec - start.tv_nsec;

    if (diff_nsec < 0) {
        diff_sec -= 1;
        diff_nsec += 1000000000L;
    }
    if (diff_sec > server_timeout.tv_sec)
        return true;

    if (diff_sec == server_timeout.tv_sec && diff_nsec > server_timeout.tv_nsec)
        return true;

    return false;
}

void update_activity(game_state_t &game,uint32_t player_id, struct timespec now) {
    if (game.player_a_id == player_id)
        game.last_activity_a = now;
    if (game.player_b_id == player_id)
        game.last_activity_b = now;
}

// Sprawdzamy, czy nie trzeba usunąć lub zakończyć jakiejś rozgrywki.
void check_timeouts_and_clean_games(struct timespec now, struct timespec server_timeout) {
    for (auto it = games.begin(); it != games.end();) {

        game_state_t &game = it->second;
        bool to_delete = false;

        if (game.status == WAITING_FOR_OPPONENT) {
            if(is_timeout(game.last_activity_a, now, server_timeout))
                to_delete = true;
        }
        else if (!game.finished) {

            bool timeout_a = is_timeout(game.last_activity_a, now, server_timeout);
            bool timeout_b = is_timeout(game.last_activity_b, now, server_timeout);

            // Jeśli gracz A oraz B przekroczyli czas, to wygrywa ten, kto był aktywny ostatnio.
            if (timeout_a || timeout_b) {
                if (timeout_a && timeout_b) {
                    if (compare_timespec(game.last_activity_a, game.last_activity_b) > 0)
                        game.status = WIN_A;
                    else
                        game.status = WIN_B;
                }
                else if (timeout_a)
                    game.status = WIN_B;
                else if (timeout_b)
                    game.status = WIN_A;

                game.finished = true;
                game.end_time = now;
            }
        }
        else {
            if (is_timeout(game.end_time, now, server_timeout))
                to_delete = true;
        }
        if (to_delete)
            it = games.erase(it);
        else
            it++;
    }

}

void join(uint32_t player_id, uint32_t &game_id, struct timespec now, const char* pawn_row,
    bool &error, uint8_t &error_index, ssize_t received_len) {

    error = false;
    error_index = 0;
    if (received_len != MINIMAL_ARGS_LEN) {
        error = true;
        error_index = received_len;
        return;
    }
    for (auto &pair: games) {
        if (pair.second.player_b_id == 0 && !pair.second.finished) {
            game_id = pair.first;
            add_player_b(&pair.second, player_id);
            pair.second.last_activity_b = now;
            return;
        }
    }

    if (game_id_g >= UINT32_MAX) {
        error = true;
        error_index = 0;
        return;
    }

    init_game(&games[game_id_g], pawn_row, player_id);
    game_id = (game_id_g);
    games[game_id].last_activity_a = now;
    games[game_id].last_activity_b = now;
    game_id_g++;

}

bool parse_and_check_game(char *buffer, uint32_t &game_id_to_save, bool &error, uint8_t &error_index, uint32_t player_id) {
    uint32_t game_id = 0;
    memcpy(&game_id, buffer + 5, sizeof(game_id));
    game_id_to_save = ntohl(game_id);

    // Nie ma takiej rozgrywki.
    if (!games.contains(game_id_to_save)) {
        error = true;
        error_index = 5;
        return false;
    }
    // W tej rozgrywce nie uczestniczy podany gracz.
    if (games[game_id_to_save].player_a_id != player_id && games[game_id_to_save].player_b_id != player_id) {
        error = true;
        error_index = 8;
        return false;
    }
    return true;
}

void make_move(uint32_t player_id, uint32_t &game_id_to_save, struct timespec now,
    char *buffer, bool &error, uint8_t &error_index, ssize_t received_len, bool move_two) {

    uint32_t game_id = 0;
    error = false;
    error_index = 0;

    if (received_len != MINIMAL_ARGS_LEN_MOVE) {
        error = true;
        error_index = received_len;
        return;
    }
    if (!parse_and_check_game(buffer, game_id, error, error_index, player_id))
        return;

    if (games[game_id].finished) {
        game_id_to_save = game_id;
        return;
    }

    game_id_to_save = game_id;
    uint8_t pawn;

    memcpy(&pawn, buffer + 9, sizeof(pawn));

    game_state_t &game = games[game_id];
    update_activity(game, player_id, now);

    if (pawn <= game.max_pawn)
        move(&game, player_id, pawn, move_two);

    if (game.status == WIN_A || game.status == WIN_B) {
        game.finished = true;
        game.end_time = now;
    }
}

void parse_msg_type_and_player_id(char *buffer, uint32_t &player_id_to_save,
    uint8_t &msg_type_to_save, bool &error, uint8_t &error_index, ssize_t received_len) {

    uint32_t player_id = 0;
    if (received_len < MINIMAL_ARGS_LEN) {
        error = true;
        error_index = received_len; // za krótki
    }
    else {
        memcpy(&msg_type_to_save, buffer, sizeof(msg_type_to_save));
        memcpy(&player_id, buffer + 1, sizeof(player_id));
        player_id_to_save = ntohl(player_id);

        if (msg_type_to_save > MSG_GIVE_UP) {   // Nieznany komunikat.
            error = true;
            error_index = 0;
        }

        if (player_id_to_save == 0) {
            error = true;
            error_index = 1;
        }

    }

    if (!error && msg_type_to_save != MSG_JOIN) {
        if (received_len < MINIMAL_ARGS_LEN_REST_TYPE) {
            error = true;
            error_index = received_len;
        }
    }

}

int main(int argc, char *argv[]) {

    char *pawn_row = {nullptr}, *address = {nullptr};
    int timeout,opt;
    uint16_t port = 0;
    bool received_r = false, received_a = false, received_p = false, received_t = false;
    // Pobieramy argumenty i sprawdzamy ich poprawność.
    while ((opt = getopt(argc, argv, "r:a:p:t:")) != -1) {
        switch (opt) {
            case 'r':
                if (!validate_pawn_row(optarg))
                    fatal("pawn_row failed");

                pawn_row = optarg;
                received_r = true;
                break;

            case 'a':
                if (!validate_address(optarg))
                    fatal("address failed");

                address = optarg;
                received_a = true;
                break;

            case 'p':
                port = read_port(optarg, 0);
                received_p = true;
                break;

            case 't':
                if (!validate_timeout(optarg, &timeout))
                    fatal("timeout failed");
                received_t = true;
                break;

            default:
                fatal("invalid argument");
                break;

        }
    }
    // Nie dostaliśmy wszystkich wymaganych argumentów.
    if (!received_r || !received_a || !received_p || !received_t) {
        fatal("Missing arguments");
    }

    struct timespec server_timeout;
    server_timeout.tv_sec = timeout;
    server_timeout.tv_nsec = 0;

    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        syserr("can't create socket");
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        syserr("setsockopt failed");
    }

    sockaddr_in server_addr = get_server_address(address, port);
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        syserr("bind failed");
    }

    while (true) {

        char buffer[BUFFER_SIZE] = {};
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);

        ssize_t received_len = recvfrom(socket_fd, buffer, BUFFER_SIZE, 0,
                                        (struct sockaddr *) &client_addr,
                                        &client_addr_len);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        if (received_len < 0) {
             if (errno == EAGAIN || errno == EWOULDBLOCK) {
                 check_timeouts_and_clean_games(now, server_timeout);
                 continue;
             }

            syserr("recv failed");
         }

        uint32_t game_id = 0, player_id = 0;
        uint8_t msg_type = 0, error_index = 0;
        bool error = false;

        if (received_len >= 0) {
            parse_msg_type_and_player_id(buffer, player_id, msg_type, error, error_index, received_len);

            if (!error) {

                switch (msg_type) {
                    case MSG_JOIN: {
                        join(player_id, game_id, now, pawn_row, error, error_index, received_len);
                        break;
                    }

                    case MSG_MOVE_1: {
                        make_move(player_id, game_id, now, buffer, error, error_index, received_len, false);
                        break;
                    }

                    case MSG_MOVE_2: {
                        make_move(player_id, game_id, now, buffer, error, error_index, received_len, true);
                        break;
                    }

                    case MSG_KEEP_ALIVE: {
                        if (!parse_and_check_game(buffer, game_id, error, error_index, player_id))
                            break;

                        game_state_t &game = games[game_id];
                        if (!game.finished)
                            update_activity(game, player_id, now);
                        else
                            game.end_time = now;

                        break;
                    }

                    case MSG_GIVE_UP: {
                        if (!parse_and_check_game(buffer, game_id, error, error_index, player_id))
                            break;

                        game_state_t &game = games[game_id];

                        if (game.status == WIN_A || game.status == WIN_B)
                            break;

                        give_up(&game, player_id);
                        game.finished = true;
                        game.end_time = now;

                        break;
                    }

                    default:
                        error = true;
                        error_index = 0;
                        break;
                }
            }
        }
        check_timeouts_and_clean_games(now, server_timeout);
        if (error)
            send_wrong_msg(socket_fd, client_addr, buffer, received_len, error_index);
        else
            send_state(socket_fd, client_addr, game_id);

    }
    close(socket_fd);
    return 0;
}

