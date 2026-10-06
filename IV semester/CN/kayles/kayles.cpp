#include <cmath>
#include <cstring>
#include <iostream>
#include <ostream>

#include "kayles.h"

void init_game(game_state_t *game, const char *pawn_row, uint32_t player_a_id) {
    game->player_a_id = player_a_id;
    game->player_b_id = 0;
    game->status = WAITING_FOR_OPPONENT;
    game->finished = false;

    size_t len = strlen(pawn_row);
    game->max_pawn = len - 1;          // Ponieważ piony numerowane są od 0.

    memset(game->pawn_row, 0, sizeof(game->pawn_row));

    for (size_t i = 0; i < len; i++) {
        if (pawn_row[i] == '1') {
            size_t byte_index = (i / 8);
            size_t bit_index = 7 - (i % 8);
            game->pawn_row[byte_index] |= (1 << bit_index);
        }
    }

}

void add_player_b(game_state_t *game, uint32_t player_b_id) {
    game->player_b_id = player_b_id;
    game->status = TURN_B;
}

void move(game_state_t *game, uint32_t player_id, uint8_t pawn, bool two_pawns) {

    if (game->status == TURN_A) {
        if (player_id != game->player_a_id)     // Jest kolej gracza A, ale ruch chce wykonać inny gracz -> ruch nielegalny.
            return;
    }
    else if (game->status == TURN_B) {          // Jest kolej gracza B, ale ruch chce wykonać inny gracz -> ruch nielegalny.
        if (player_id != game->player_b_id)
            return;
    }

    if (two_pawns) {                            // Chcemy wykonać ruch, który zbija 2 piony.
        if (!move_two(game, player_id, pawn))   // Sprawdzamy, czy ruch się udał (jeśli tak to jest on wykonywany w move_two).
            return;

        if (check_win(game))
            // Sprawdzamy, czy ruch sprawił, że gra się skończyła, jeśli tak to zwracamy, bo status jest już poprawnie ustawiony.
            return;

        game->status = 3 - game->status;        // TURN_A = 1, TURN_B = 2, więc 3 - status dla tych wartości poprawnie zmieni turę.
    }
    else {                                      // Chcemy wykonać ruch, który zbija 1 piona.
        if (!move_one(game, player_id, pawn))
            return;

        if (check_win(game))
            return;

        game->status = 3 - game->status;

    }
}

// Sprawdzenie legalności ruchu, jeśli legalny to zbicie podanego piona.
bool move_one(game_state_t *game, uint32_t player_id, uint8_t pawn) {
    if (!is_move_legal(game, player_id, pawn, false))       // Ruch nielegalny.
        return false;

    remove_pawn(game, pawn);                                          // Ruch legalny, wykonujemy zbicie piona.

    return true;
}

// Sprawdzenie legalności ruchu, jeśli legalny to zbicie piona oraz piona następnego.
bool move_two(game_state_t *game, uint32_t player_id, uint8_t pawn) {
    if (!is_move_legal(game, player_id, pawn, true))
        return false;

    remove_pawn(game, pawn);
    remove_pawn(game, pawn + 1);

    return true;
}

// Sprawdzamy, czy mozna wykonać ruch (czy piony istnieją, czy jest tura gracza).
bool is_move_legal(game_state_t *game, uint32_t player_id, uint8_t pawn, bool two_pawns) {
    int turn = game->status;
    uint32_t player;

    if (turn == TURN_A)             // Ustalamy id_gracza, którego jest teraz tura.
        player = game->player_a_id;
    else
        player = game->player_b_id;

    if (player != player_id)        // Nie jest to tura tego gracza, ruch nielegalny.
        return false;

    if (two_pawns) {                // Chcemy zbić 2 piony, sprawdzamy, czy można.
        bool can_move = get_pawn(game, pawn) == 1 && get_pawn(game, pawn + 1) == 1;
        return can_move;
    }
    // Sprawdzenie, czy można zbić 1 piona.
    bool can_move = get_pawn(game, pawn) == 1;
    return can_move;
}

// Sprawdzamy, czy gra zakończyła się zwycięstwem któregoś z graczy - sprawdzamy, czy wszystkie piony na planszy są równe 0.
bool check_win(game_state_t *game) {
    // <= ponieważ max_pawn oznacza maksymalny numer piona, czyli też trzeba go wliczyć.
    for (int i = 0; i <= game->max_pawn; i++) {
        if (get_pawn(game, i)) {    // Jeśli trafiliśmy na jakiś niezerowy pion, to jeszcze nie jest wygrana, więc kończymy zwracając false.
            return false;
        }
    }

    // Dotarliśmy do tego momentu, czyli wszystkie piony równe 0, sprawdzamy czyja tura, aby przypisać zwycięzcę.
    int winner = game->status;
    if (winner == TURN_A)
        game->status = WIN_A;
    else
        game->status = WIN_B;

    return true;

}

// Bierzemy piona i zwracamy jego wartość.
bool get_pawn(game_state_t *game, uint8_t pawn) {
    size_t index = pawn / 8;
    size_t bit_index = 7 - pawn % 8;
    return (game->pawn_row[index] & (1 << bit_index));
}

// Zwracamy piona z pawn row, a nie gry, do wypisania w kliencie.
bool get_pawn_to_print(uint8_t *pawn_row, uint8_t pawn) {
    size_t index = pawn / 8;
    size_t bit_index = 7 - pawn % 8;
    return (pawn_row[index] & (1 << bit_index));
}

void remove_pawn(game_state_t *game, uint8_t pawn) {
    size_t index = pawn / 8;
    size_t bit_index = 7 - pawn % 8;
    game->pawn_row[index] &= ~(1 << bit_index);
}

// Poddanie się, ustalenie, który gracz wygrywa.
void give_up(game_state_t *game, uint32_t player_id) {
    if (player_id == game->player_a_id && game->status == TURN_A ) {
            game->status = WIN_B;
    } else if (player_id == game->player_b_id && game->status == TURN_B) {
            game->status = WIN_A;
    }
}

