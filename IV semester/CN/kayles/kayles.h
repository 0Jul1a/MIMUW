#ifndef KAYLES_H
#define KAYLES_H

#include <cstdint>
#include <ctime>

#define WAITING_FOR_OPPONENT 0
#define TURN_A 1
#define TURN_B 2
#define WIN_A 3
#define WIN_B 4
// Wiemy, że maksymalnie może być 256 pionów, więc maksymalny rozmiar tablicy uint8_t pionów to: (256 pionów / 8) + 1.
#define MAX_PAWN_ROW_SIZE 33
typedef struct {

    uint32_t game_id;
    uint32_t player_a_id;
    uint32_t player_b_id;
    uint8_t status;
    uint8_t max_pawn;
    uint8_t pawn_row[MAX_PAWN_ROW_SIZE];

    bool finished;

    struct timespec end_time;
    struct timespec last_activity_a;
    struct timespec last_activity_b;

}game_state_t;

// Zaczynamy rozgrywkę, tworzymy planszę na podstawie napisu z parametru podanego w -r.
void init_game(game_state_t *game, const char *pawn_row, uint32_t player_a_id);

void add_player_b(game_state_t *game, uint32_t player_b_id);

void move(game_state_t *game, uint32_t player_id, uint8_t pawn, bool two_pawns);

bool move_one(game_state_t *game, uint32_t player_id, uint8_t pawn);

bool move_two(game_state_t *game, uint32_t player_id, uint8_t pawn);

// Sprawdzamy, czy mozna wykonać ruch (czy piony istnieją, czy jest tura gracza).
bool is_move_legal(game_state_t *game, uint32_t player_id, uint8_t pawn, bool two_pawns);

// Sprawdzamy stan gry, czy jest to już koniec rozgrywki.
bool check_win(game_state_t *game);

// Sprawdzamy, czy jest pion.
bool get_pawn(game_state_t *game, uint8_t pawn);

// Funkcja do wyciągnięcia wartości piona do wypisania w kliencie.
bool get_pawn_to_print(uint8_t *pawn_row, uint8_t pawn);

void remove_pawn(game_state_t *game, uint8_t pawn);

// Poddanie się gracza, ustawienie odpowiedniego statusu.
void give_up(game_state_t *game, uint32_t player_id);

#endif

