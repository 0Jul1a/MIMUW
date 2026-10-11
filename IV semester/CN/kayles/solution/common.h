#ifndef COMMON_H
#define COMMON_H

#define BUFFER_SIZE 64
#define MAX_SIZE_PAWN_ROW 256               // Maksymalna ilość pionków.
#define MIN_SIZE_PAWN_ROW 1
#define TIMEOUT_MAX       99
// Maksymalna wielkość wiadomości podanej w argumencie (msg_type + player_id + game_id + pawn + 1 (na wykrycie dodatkowych błędnych wartości)).
#define MAX_MSG_SIZE      5
#define STATUS_INDEX 12                                            // Numer indexu, pod którym niezależnie od rodzaju wiadomości jest status.

#define MSG_JOIN 0
#define MSG_MOVE_1 1
#define MSG_MOVE_2 2
#define MSG_KEEP_ALIVE 3
#define MSG_GIVE_UP 4
#define MSG_GAME_STATE 5
#define MSG_WRONG_MSG 255

uint16_t read_port(char const *string, int client);
bool validate_pawn_row(const char *pawn_row);
bool validate_timeout(char const *string, int *timeout);
int split_msg(char *string, char *msg[], int max_parts);
bool parse_32(const char *string, uint32_t *s);
bool parse_8(const char *string, uint8_t *s);
bool validate_message(char *string);
bool validate_address(char const *string);
struct sockaddr_in get_server_address(char const *host, uint16_t port);

#endif

