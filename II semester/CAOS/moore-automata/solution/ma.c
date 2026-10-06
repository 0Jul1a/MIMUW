// Zadanie 1
// Autor: Julia Stasiak

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>       
#include <stdbool.h>
#include "ma.h"

#define NUM_OF_BITS 64

typedef struct input_connection input_connection_t;
typedef struct output_connection_list output_connection_list_t;

// Struktura opisująca połączenie wejścia automatu.
struct input_connection {
   
    moore_t *connected_automat;           // Automat, z którego wyjścia będziemy pobierać wartość. 
    size_t out_bit_num;                   // Konkretny bit sygnału wyjściowego ww. automatu.
    
};

// Struktura opisująca połączenie wyjścia automatu.
struct output_connection_list {
    
    moore_t *connected_automat;          // Automat, którego wejście jest połączone z tym wyjściem.
    size_t in_bit_num;                   // Konkretny bit sygnału wejściowego ww. automatu.
    output_connection_list_t *next;
};

struct moore {
   
    uint64_t *input;
    uint64_t *output;
    uint64_t *state;

    size_t input_size;
    size_t output_size;
    size_t state_size;

    output_function_t output_function;
    transition_function_t transition_function;

    input_connection_t *input_connections;
    output_connection_list_t **output_connections;              

};

void free_output_connection_list(output_connection_list_t *connection_list_to_be_freed) {

    if (!connection_list_to_be_freed)
        return;

    output_connection_list_t *curr = connection_list_to_be_freed;

    while (curr) {
        
        output_connection_list_t *tmp = curr->next;
        free(curr);
        curr = tmp;

    }
}

void free_output_connections(moore_t *automats_output_connections_to_be_freed) {

    for (size_t i = 0; i < automats_output_connections_to_be_freed->output_size; i++) 
        free_output_connection_list(automats_output_connections_to_be_freed->output_connections[i]);

    free(automats_output_connections_to_be_freed->output_connections);
    
}

output_connection_list_t *create_new_node(moore_t *automat, size_t bit_num) {

    output_connection_list_t *new_node = malloc(sizeof(output_connection_list_t));

    if (!new_node)
        return NULL;

    new_node->connected_automat = automat;
    new_node->in_bit_num = bit_num;
    new_node->next = NULL;

    return new_node;

}

// Dodajemy na początek listy nowy węzeł.
int add_element(output_connection_list_t **connection_list, moore_t *automat_to_add, size_t bit_num) {
    
    output_connection_list_t *new_node = create_new_node(automat_to_add, bit_num);

    if (!new_node) {
        errno = ENOMEM;
        return -1;
    }

    new_node->next = *connection_list;
    *connection_list = new_node;

    return 0;

}

void remove_element(output_connection_list_t **connection_list, moore_t *automat_to_remove, size_t bit) {

    if (!connection_list) {
        return;
    }
    
    output_connection_list_t *curr = *connection_list;
    output_connection_list_t *prev = NULL;

    while (curr) {

        // Sprawdzamy, czy usuwane jest właściwe połączenie, ponieważ automat (automat_to_remove) może być podłączony kilkukrotnie
        // do tego samego bitu na wyjściu danego automatu (curr->connected_automat), ale z innymi bitami na swoim wejściu.
        if (curr->connected_automat == automat_to_remove && curr->in_bit_num == bit) {

            if (!prev) {                                        // Szukany węzeł do usunięcia jest na początku listy.
                *connection_list = curr->next;
            } 
            else {
                prev->next = curr->next;
            }
            free(curr);
            return;
        }

        prev = curr;
        curr = curr->next;
    }
    
}

size_t get_size(size_t num) {
    return num / NUM_OF_BITS + (num % NUM_OF_BITS != 0 ? 1 : 0);
}

void id_function(uint64_t *output, uint64_t const *state, size_t m, size_t s) {
    (void)s;
    size_t size_m = get_size(m);
    memcpy(output, state, size_m * sizeof(uint64_t));
}

moore_t * ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, 
    output_function_t y, uint64_t const *q) {

    if (m == 0 || s == 0 || !t ||!y || !q) {
        errno = EINVAL;
        return NULL;
    }

    moore_t *new = (moore_t *)calloc(1, sizeof(moore_t));

    if (!new) {
        errno = ENOMEM;
        return NULL;
    }

    size_t input_size = get_size(n);
    size_t output_size = get_size(m);
    size_t state_size = get_size(s);

    new->input = calloc(input_size, sizeof(uint64_t));
    new->output = calloc(output_size, sizeof(uint64_t));
    new->state = calloc(state_size, sizeof(uint64_t));

    new->input_connections = calloc(n, sizeof(input_connection_t));
    new->output_connections = calloc(m, sizeof(output_connection_list_t *));

    if (!new->input || !new->output || !new->state
            || !new->input_connections || !new->output_connections) {

        errno = ENOMEM;
        free(new->input);
        free(new->output);
        free(new->state);
        free(new->input_connections);
        free(new->output_connections);
        free(new);
        return NULL;
    }
    
    new->input_size = n;
    new->output_size = m;
    new->state_size = s;

    new->transition_function = t;
    new->output_function = y;

    memcpy(new->state, q, state_size * sizeof(uint64_t));

    new->output_function(new->output, new->state, m, s);
    return new;

}

moore_t * ma_create_simple(size_t n, size_t s, transition_function_t t) {

    if (!t || s == 0) {
        errno = EINVAL;
        return NULL;        
    }

    size_t state_size = get_size(s);

    uint64_t *q = calloc(state_size, sizeof(uint64_t));

    if (!q) {
        errno = ENOMEM;
        return NULL;
    }
        
    moore_t *new = ma_create_full(n, s, s, t, id_function, q);
    free(q);

    if (!new) {
        errno = ENOMEM;
        return NULL;
    }

    return new;

}

uint64_t const * ma_get_output(moore_t const *a) {

    if (!a) {
        errno = EINVAL;
        return NULL;
    }

    return a->output;

}

int ma_set_input(moore_t *a, uint64_t const *input) {

    if (!a || !a->input || !input || a->input_size == 0) {
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < a->input_size; i++) {
        if (!a->input_connections[i].connected_automat) {       // Automat nie ma połączenia na tym bicie, więc można ustawić bit z 'input'.
            if (input[i / NUM_OF_BITS] & ((uint64_t)1 << (i % NUM_OF_BITS)))      // Sprawdzamy, czy odpowiedni bit w 'input' jest ustawiony.
                a->input[i / NUM_OF_BITS] |= (uint64_t)1 << (i % NUM_OF_BITS);    // Jeśli tak, to ustawiamy również na wejściu automatu.
            else                                                // Jeśli nie, to ustawiamy 0.
                a->input[i / NUM_OF_BITS] &= ~((uint64_t)1 << (i % NUM_OF_BITS)); 
        }
    }
        
    return 0;
}

int ma_set_state(moore_t *a, uint64_t const *state) {

    if (!a || !state) {
        errno = EINVAL;
        return -1;
    }
    size_t state_size = get_size(a->state_size);
    memcpy(a->state, state, state_size * sizeof(uint64_t));  // Kopiujemy do stanu automatu 'state'.
    a->output_function(a->output, a->state, a->output_size, a->state_size); // Aktualizujemy wartości na wyjściu.
    return 0;

}

int ma_disconnect(moore_t *a_in, size_t in, size_t num) {

    if (!a_in || num == 0 || in + num > a_in->input_size || in > a_in->input_size) {
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < num; i++) {
        
        size_t in_bits = in + i;

        if (in_bits >= a_in->input_size) {                          // Wyszliśmy poza zakres, więc ustawiamy 'errno' na EINVAL.
            errno = EINVAL;
            return -1;
        }

        if (a_in->input_connections[in_bits].connected_automat) {   // Odłączamy automat, jeśli był podłączony na tym bicie.
            
            moore_t *other_side_of_conection = a_in->input_connections[in_bits].connected_automat;  
            size_t out_bit = a_in->input_connections[in_bits].out_bit_num;

            remove_element(&other_side_of_conection->output_connections[out_bit], a_in, in_bits);   // Usuwamy drugą część połączenia.
            
            a_in->input_connections[in_bits].connected_automat = NULL;
            a_in->input_connections[in_bits].out_bit_num = 0;

        }
    }
    return 0;
}

// Jako argument przyjmujemy listę z połączenami na danym bicie, na wyjściu jakiegoś automatu, którą chcemy usunąć.
void remove_all_connected_outputs(output_connection_list_t **connection_list) {
    
    if (!connection_list)
        return;

    output_connection_list_t *curr = *connection_list;
    while (curr) {
        output_connection_list_t *next = curr->next;

        if (curr->connected_automat) {                                     // Jest połączenie, więc musimy odłączyć.
    
            moore_t *automat_to_disconnect = curr->connected_automat;
            size_t bit_from_automat = curr->in_bit_num;
            curr = curr->next;            

            ma_disconnect(automat_to_disconnect, bit_from_automat, 1);     // Wywołujemy ma_disconnect, ponieważ usuwa ono od razu obie strony połączenia.
            
        }
        curr = next;
    }
}

void ma_delete(moore_t *a) {

    if (!a) return;

     ma_disconnect(a, 0, a->input_size);            // Rozłączamy wszystkie wejścia.

     for (size_t i = 0; i < a->output_size; i++) {  // Rozłączamy wszystkie wyjścia.
       
         remove_all_connected_outputs(&a->output_connections[i]);

     }

    free_output_connections(a);
    free(a->input);
    free(a->output);
    free(a->state);
    free(a->input_connections);
    free(a);
    
}

int ma_connect(moore_t *a_in, size_t in, moore_t *a_out, size_t out, size_t num) {

    if (!a_in || !a_out || num == 0 || in + num > a_in->input_size ||
            out + num > a_out->output_size || in > a_in->input_size || out > a_out->output_size) {
        errno = EINVAL;
        return -1;
    }

    for (size_t i = 0; i < num; i++) {
        size_t in_bits = in + i;
        size_t out_bits = out + i;

        if (in_bits >= a_in->input_size || out_bits >= a_out->output_size) {
            errno = EINVAL;
            return -1;
        }

        if (a_in->input_connections[in_bits].connected_automat) {                       // Jeśli automat był już połączony, to trzeba rozłączyć.
            ma_disconnect(a_in, in_bits, 1);
        }
                
        if (add_element(&a_out->output_connections[out_bits], a_in, in_bits) == -1) {   // Jeśli dodanie automatu nie powiodło się, to odłączamy te połączenia, które już podłączyliśmy.

            for(size_t j = 0; j < i; j++)
                ma_disconnect(a_in, in + j, 1);            
            
            errno = ENOMEM;
            return -1;
        }

        a_in->input_connections[in_bits].connected_automat = a_out;
        a_in->input_connections[in_bits].out_bit_num = out_bits;


    }
    return 0;
}

int update_connected_inputs(moore_t *at[], size_t num) {

    for (size_t i = 0; i < num; i++) {
        
        moore_t *a = at[i];

        if (!a) {
            return -1;
        }
        
        // Idąc pętlą po liczbie sygnałów wejściowych sprawdzamy, czy jest połączenie i wówczas aktualizujemy wartość wejścia na tym bicie.
        for (size_t j = 0; j < a->input_size ; j++) {       
            
            if (a->input_connections[j].connected_automat) {    
                
                moore_t *source = a->input_connections[j].connected_automat;                    // Z jakiego automatu pobieramy nową wartość.
                size_t bit_in_source = a->input_connections[j].out_bit_num;                     // Z jakiego dokładnie bitu ją pobieramy.
                
                if (source->output[bit_in_source / NUM_OF_BITS] & ((uint64_t)1 << (bit_in_source % NUM_OF_BITS))) 
                    a->input[j / NUM_OF_BITS] |= (uint64_t)1 << (j % NUM_OF_BITS);   
                else                                                      
                    a->input[j / NUM_OF_BITS] &= ~((uint64_t)1 << (j % NUM_OF_BITS)); 
            }
        }
    }
    return 0;
}

int ma_step(moore_t *at[], size_t num) {

    if (!at || num == 0) {
        errno = EINVAL;
        return -1;
    }

    // Aktualizujemy wszystkie wejścia, które mają aktywne połączenie przed wykonaniem aktualizacji stanów.
    if (update_connected_inputs(at, num) == -1) {
        errno = EINVAL;
        return -1;
    }

    // Alokujemy pamięć na nowe stany.
    uint64_t **next_states = calloc(num, sizeof(uint64_t*));
    if (!next_states) {
        errno = ENOMEM;
        return -1;
    }

    for (size_t i = 0; i < num; i++) {

        size_t state_size = get_size(at[i]->state_size);
        next_states[i] = calloc(state_size, sizeof(uint64_t));

        if (!next_states[i]) {

            for (size_t j = 0; j < i; j++) 
                free(next_states[j]);

            free(next_states);
            errno = ENOMEM;
            return -1;
        }

    }

    // Aktualizujemy stany i wyjścia automatów.
    for (size_t i = 0; i < num; i++) {
        
        moore_t *a = at[i];
        if (!a) {
            errno = EINVAL;
            return -1;
        }

        size_t state_size = get_size(a->state_size);

        a->transition_function(next_states[i], a->input, a->state, a->input_size, a->state_size);
        memcpy(a->state, next_states[i], state_size * sizeof(uint64_t));

        a->output_function(a->output, a->state, a->output_size, a->state_size);
        free(next_states[i]);

    }
    
    free(next_states);

    return 0;

}

