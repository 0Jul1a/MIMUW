// place your solution here
#include "../common/plant.h"
#include "../common/err.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum state_t {
    TASK_ERROR,                     // Zadanie zakończyło się błędem.
    TASK_DONE,                      // Zadanie wykonano pomyślnie.
} state_t;

typedef struct task_state {

    task_t *task;                   // Zadanie
    state_t state;                  // Aktualny status zadania.
    int n_workers_finished;         // Ilu pracowników już wykonało zadanie.
    int station_id;                 // Na którym stanowisku wykonywane jest zadanie.
    int *workers;                   // Tablica z Id tych pracowników, którzy pracują przy zadaniu.
    bool finished;                  // Czy zadanie już wykonano całe.
    pthread_cond_t task_done;
    pthread_mutex_t task_mutex;

} task_state;

typedef struct worker_task {

    worker_t *worker;               // Oryginalna struktura pracownika.
    int index_in_task;              // Ktorym indeksem w zadaniu jesteśmy (potrzebna do wywołania work).
    task_state *current_task;       // Które zadanie wykonujemy aktualnie.

} worker_task;

typedef struct plant_t {

    bool destroyed;

    int n_workers;                  // Ilu wszystkich pracowników zgłosi się do pracy.
    worker_t **workers;
    bool *worker_busy;              // Informacja czy pracownik aktualnie wykonuje zadanie.
    int workers_added;              // Liczba pracowników, którzy już zgłosili się do pracy.
    int workers_busy;               // Liczba pracowników, którzy aktualnie pracują.

    int n_stations;                 // Ile jest wszystkich stanowisk w fabryce.
    int *station_capacity;          // Ich pojemność.
    bool *station_used;             // Czy dane stanowisko jest zajęte aktualnie.
    int stations_used;              // Ile stanowisk jest zajętych.
    int max_station_capacity;       // Pojemność stanowiska o największym capacity z wszystkich.

    task_state **tasks;
    int n_tasks;                    // Ile wszystkich zadań.
    task_state **pending_tasks;
    int n_pending_tasks;            // Ile zadań oczekuje na zrealizowanie.

    pthread_mutex_t mutex;

    pthread_cond_t manager_cond;
    pthread_t manager;              // "Menadżer" fabryki.

} plant_t;

static plant_t* plant;

// Funkcja wątku pracownika, która wykonuje swoją (pracownika) część zadania, zapisuje wynik pod odpowiednim indeksem w task->result,
// a następnie zgłasza się jako wykonany i wolny pracownik.
// Jeżeli jesteśmy ostatnim pracownikiem wykonującym zadanie, to musimy ustawić stan zadania na zrobiony oraz zwolnić stanowisko.
void* work_thread(void* data)
{
    // Wykonujemy pracę i zapisujemy jej wynik w "res".
    worker_task *w = (worker_task*)data;
    task_state *t = w->current_task;
    int res = w->worker->work(w->worker, w->current_task->task, w->index_in_task);

    // Zapisujemy wynik i zwiększamy liczbę skończonych pracowników.
    ASSERT_ZERO(pthread_mutex_lock(&t->task_mutex));
    t->task->results[w->index_in_task] = res;
    t->n_workers_finished++;
    bool last_worker = (t->n_workers_finished == t->task->capacity) ? true : false;             // Czy jesteśmy ostatnim pracownikiem.
    ASSERT_ZERO(pthread_mutex_unlock(&t->task_mutex));

    // Wykonaliśmy już naszą część zadania, więc zmieniamy status na wolny.
    ASSERT_ZERO(pthread_mutex_lock(&plant->mutex));
    plant->worker_busy[t->workers[w->index_in_task]] = false;
    plant->workers_busy--;

    // Jesteśmy ostatni więc musimy oznaczyć stanowisko jako wolne.
    if (last_worker) {
        plant->station_used[t->station_id] = false;
        plant->stations_used--;
    }
    ASSERT_ZERO(pthread_cond_broadcast(&plant->manager_cond));
    ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));

    // Jeśli jesteśmy ostatni to musimy ustawić status zadania i oznaczyć je jako skończony.
    if (last_worker) {
        ASSERT_ZERO(pthread_mutex_lock(&t->task_mutex));
        t->finished = true;
        t->state = TASK_DONE;
        ASSERT_ZERO(pthread_cond_broadcast(&t->task_done));
        ASSERT_ZERO(pthread_mutex_unlock(&t->task_mutex));
    }
    free(w);
    return NULL;
}

// Czas rozpoczęcia zadania, które zaczyna się najwcześniej.
time_t get_next_task_time()
{

    if (plant->n_pending_tasks == 0)            // Nie ma więcej zadań oczekujących.
        return -1;

    time_t min_next_time = plant->pending_tasks[0]->task->start;
    for (int i = 1; i < plant->n_pending_tasks; i++) {
        time_t task_time = plant->pending_tasks[i]->task->start;
        if (task_time < min_next_time)
            min_next_time = task_time;
    }
    return min_next_time;
}

// Funkcja wątku menadżera, która wykonuje wszystko, co jest związane z wykonaniem zadania: zarządza rozpoczynaniem zadań, rezerwuje stanowiska i pracowników
// oraz tworzy wątki dla pracowników, gdy zbierze się odpowiedni zespół.
void* manager_thread(void* p)
{
    ASSERT_ZERO(pthread_mutex_lock(&plant->mutex));

    while (!plant->destroyed || plant->n_pending_tasks > 0) {

        time_t now = time(NULL);

        bool cannot_work = (plant->n_pending_tasks == 0 || plant->stations_used == plant->n_stations ||
            plant->workers_busy == plant->workers_added);

        // Jeśli teraz nie mozna wykonac zadania, to musimy sprawdzić, czy to dlatego, że nie ma potrzebnych zasobów, czy dlatego
        // że jest za wcześnie (i nikt nas później nie da rady obudzić, bo już wszyscy pracownicy się zgłosili do pracy i wykonali zadanie)
        if (cannot_work) {
            time_t next_start = get_next_task_time();

            // // Jeśli zwrócono -1, to znaczy, że nie ma oczekujących zadań więc na pewno musimy "zawiesić" się ze względu na brak potrzebnych zasobów.
            if (next_start == -1)
                ASSERT_ZERO(pthread_cond_wait(&plant->manager_cond, &plant->mutex));
            else if (next_start > now && (plant->workers_added == plant->n_workers && plant->workers_busy == 0)) {
                struct timespec ts;
                ts.tv_sec = next_start;
                ts.tv_nsec = 0;
                ASSERT_ZERO(pthread_cond_timedwait(&plant->manager_cond, &plant->mutex, &ts));
            } else {
                ASSERT_ZERO(pthread_cond_wait(&plant->manager_cond, &plant->mutex));
            }
        }

        // Przetwarzamy wszystkie oczekujące zadania próbując przydzielić im pracowników oraz stanowisko.
        for (int i = 0; i < plant->n_pending_tasks; i++) {

            task_state *t = plant->pending_tasks[i];
            if (now < t->task->start)           // Jest za wcześnie na wykonanie zadania.
                continue;

            int station_id = -1;

            // Szukamy odpowiedniego stanowiska (sprawdzamy, czy jest wolne i czy ma dobry rozmiar).
            for (int s = 0; s < plant->n_stations; s++) {
                if (!plant->station_used[s] && plant->station_capacity[s] >= t->task->capacity) {
                    station_id = s;
                    break;
                }
            }

            // Nie ma odpowiedniego stanowiska, więc nie możemy wykonać teraz tego zadania,
            if (station_id == -1)
                continue;

            // Tablica pracowników, którzy będą przydzieleni do tego zadania.
            int *assigned_workers = calloc(t->task->capacity, sizeof(int));
            if (!assigned_workers)
                continue;

            // Ile pracowników przydzielono.
            int count = 0;
            // Przydzielamy wolnych pracowników do zadania zgodnie z wymaganiami.
            for (int w = 0; w < plant->workers_added; w++) {

                // Już przydzieliliśmy tylu pracowników ilu powinniśmy.
                if (count == t->task->capacity)
                    break;

                worker_t *worker = plant->workers[w];

                if (!worker)
                    continue;
                // Możemy przydzielić pracownika do zadania, kiedy nie jest zajęty i czas pracy się zgadza.
                if (!plant->worker_busy[w] && now >= worker->start && now < worker->end
                    && worker->end >= t->task->start) {
                    assigned_workers[count++] = w;
                    }
            }

            // Nie udało się przydzielić odpowiedniej liczby pracowników, sprawdzamy, czy w przyszłości się uda.
            if (count < t->task->capacity) {
                bool possible_in_the_future = false;

                // Jeszcze nie zgłosili się wszyscy pracownicy, więc nie można stwierdzić czy jest niewykonalne zadanie, czyli może się udać w przyszlości.
                if (plant->n_workers > plant->workers_added)
                    possible_in_the_future = true;
                else {
                    // Sprawdzamy, czy jest jakiś wolny pracownik, który jeszcze będzie mógł zgłosić się do zadania.
                    for (int w = 0; w < plant->n_workers; w++) {
                        if (plant->workers[w]) {
                            worker_t *worker = plant->workers[w];
                            if (plant->n_workers == plant->workers_added && !plant->worker_busy[w] && worker->end >= t->task->start) {
                                possible_in_the_future = true;
                                break;
                            }
                        }
                    }
                }
                // Nie jest mozliwe już wykonanie zadania nigdy.
                if (!possible_in_the_future) {
                    t->state = TASK_ERROR;
                    t->finished = true;
                    ASSERT_ZERO(pthread_cond_broadcast(&t->task_done));

                    // Usuwamy to zadanie z zadań oczekujących.
                    for (int j = i; j < plant->n_pending_tasks - 1; j++)
                        plant->pending_tasks[j] = plant->pending_tasks[j + 1];
                    plant->n_pending_tasks--;
                    i--;    // Poniewaz przesunęliśmy tablicę.
                }
                free(assigned_workers);
                continue;
            }

            // Wiemy, że mamy odpowiednią ilość pracowników i zajęte stanowisko, więc możemy już usunąć to zadanie z oczekujących.
            for (int j = i; j < plant->n_pending_tasks - 1; j++)
                plant->pending_tasks[j] = plant->pending_tasks[j + 1];
            plant->n_pending_tasks--;
            i--;    // Poniewaz przesunęliśmy tablicę.

            plant->station_used[station_id] = true;
            plant->stations_used++;
            // Ustawiamy wcześniej przydzielonych pracowników na zajętych.
            for (int j = 0; j < t->task->capacity; j++) {
                plant->worker_busy[assigned_workers[j]] = true;
                t->workers[j] = assigned_workers[j];
                plant->workers_busy++;
            }

            t->station_id = station_id;

            // Tworzymy wątki dla pracowników.
            for (int j = 0; j < t->task->capacity; j++) {

                worker_task *wt = malloc(sizeof(worker_task));
                if (!wt) {
                    continue;
                }

                wt->worker = plant->workers[t->workers[j]];

                if (!wt->worker) {
                    free(wt);
                    continue;
                }

                wt->index_in_task = j;
                wt->current_task = t;
                pthread_t worker_thread;
                ASSERT_ZERO(pthread_create(&worker_thread, NULL, work_thread, wt));
                ASSERT_ZERO(pthread_detach(worker_thread));
            }
            free(assigned_workers);
        }
    }

    ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
    return NULL;
}

int init_plant(int* stations, int n_stations, int n_workers)
{

    plant = calloc(1, sizeof(plant_t));

    if (!plant)
        return ERROR;

    plant->n_stations = n_stations;
    plant->stations_used = 0;
    plant->station_used = malloc(sizeof(bool) * n_stations);
    if (!plant->station_used)
        return ERROR;
    plant->station_capacity = malloc(sizeof(int) * n_stations);
    if (!plant->station_capacity)
        return ERROR;
    plant->max_station_capacity = -1;
    for (int i = 0; i < n_stations; i++) {
        plant->station_used[i] = false;
        plant->station_capacity[i] = stations[i];
        if (stations[i] > plant->max_station_capacity)
            plant->max_station_capacity = stations[i];
    }

    plant->n_workers = n_workers;
    plant->workers = (worker_t **)calloc(n_workers, sizeof(worker_t *));
    plant->worker_busy = (bool *)calloc(n_workers, sizeof(bool));
    plant->workers_added = 0;
    plant->workers_busy = 0;
    if (!plant->workers || !plant->worker_busy)
        return ERROR;
    for (int i = 0; i < n_workers; i++)
        plant->worker_busy[i] = false;

    plant->pending_tasks = NULL;
    plant->n_pending_tasks = 0;

    plant->tasks = NULL;
    plant->n_tasks = 0;
    ASSERT_ZERO(pthread_cond_init(&plant->manager_cond, NULL));
    ASSERT_ZERO(pthread_mutex_init(&plant->mutex, NULL));

    plant->destroyed = false;

    ASSERT_ZERO(pthread_create(&plant->manager, NULL, &manager_thread, (void *)plant));

    return PLANTOK;
}

int destroy_plant()
{

    ASSERT_ZERO(pthread_mutex_lock(&plant->mutex));

    plant->destroyed = true;
    ASSERT_ZERO(pthread_cond_broadcast(&plant->manager_cond));
    ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));

    ASSERT_ZERO(pthread_join(plant->manager, NULL));

    // Kończymy wszystkie zadania, które zostały dodane, ale możemy już stwierdzić, że nie będą nigdy możliwe do wykonania.
    for (int i = 0; i < plant->n_pending_tasks; i++) {
        task_state *t = plant->pending_tasks[i];
        bool possible_in_the_future = false;

        for (int w = 0; w < plant->n_workers; w++) {
            if (plant->workers[w]) {
                worker_t *worker = plant->workers[w];
                if (worker->end >= t->task->start) {
                    possible_in_the_future = true;
                    break;
                }
            }
        }

        if (!possible_in_the_future) {
            ASSERT_ZERO(pthread_mutex_lock(&t->task_mutex));
            t->state = TASK_ERROR;
            t->finished = true;
            for (int j = i; j < plant->n_pending_tasks - 1; j++)
                plant->pending_tasks[j] = plant->pending_tasks[j + 1];
            plant->n_pending_tasks--;
            i--;
            ASSERT_ZERO(pthread_cond_broadcast(&t->task_done));
            ASSERT_ZERO(pthread_mutex_unlock(&t->task_mutex));
        }
    }

    // Kończymy reszte zadań.
    for (int i = 0; i < plant->n_tasks; i++) {
        task_state *t = plant->tasks[i];
        ASSERT_ZERO(pthread_mutex_lock(&t->task_mutex));
        while (!t->finished)
            ASSERT_ZERO(pthread_cond_wait(&t->task_done, &t->task_mutex));
        ASSERT_ZERO(pthread_mutex_unlock(&t->task_mutex));
    }

    for (int i = 0; i < plant->n_tasks; i++) {
        if (plant->tasks[i]) {
            free(plant->tasks[i]->workers);
            ASSERT_ZERO(pthread_cond_destroy(&plant->tasks[i]->task_done));
            ASSERT_ZERO(pthread_mutex_destroy(&plant->tasks[i]->task_mutex));
            free(plant->tasks[i]);
        }
    }

    free(plant->tasks);
    free(plant->pending_tasks);
    free(plant->workers);
    free(plant->worker_busy);
    free(plant->station_capacity);
    free(plant->station_used);

    ASSERT_ZERO(pthread_mutex_destroy(&plant->mutex));
    ASSERT_ZERO(pthread_cond_destroy(&plant->manager_cond));

    free(plant);
    plant = NULL;

    return PLANTOK;

}

int add_worker(worker_t* w)
{
    ASSERT_ZERO(pthread_mutex_lock(&plant->mutex));

    if (plant->destroyed) {
        ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
        return ERROR;
    }

    // Jeśli już był dodany pracownik o takim ID to ignorujemy.
    for (int i = 0; i < plant->workers_added; i++) {
        if (plant->workers[i]->id == w->id) {
            ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
            return PLANTOK;
        }
    }

    plant->workers[plant->workers_added] = w;
    plant->workers_added++;

    ASSERT_ZERO(pthread_cond_broadcast(&plant->manager_cond));

    ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
    return PLANTOK;
}

int add_task(task_t* t)
{

    ASSERT_ZERO(pthread_mutex_lock(&plant->mutex));

    if (plant->destroyed) {
        ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
        return ERROR;
    }

    task_state *new_task = calloc(1, sizeof(task_state));
    if (!new_task) {
        ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
        return ERROR;
    }

    bool task_is_already_in_tasks = false;
    // Sprawdzamy, czy mamy takie zadanie w tablicy zadań, jeśli tak, to ignorujemy
    for (int i = 0; i < plant->n_tasks; i++) {
        if (t->id == plant->tasks[i]->task->id) {
            task_is_already_in_tasks = true;
            break;
        }
    }
    
    if (task_is_already_in_tasks) {
        free(new_task);
        ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
        return PLANTOK;
    }

    new_task->task = t;
    new_task->finished = false;
    new_task->n_workers_finished = 0;
    new_task->station_id = -1;
    new_task->workers = malloc(t->capacity * sizeof(int));
    for (int i = 0; i < t->capacity; i++)
        new_task->workers[i] = -1;

    ASSERT_ZERO(pthread_cond_init(&new_task->task_done, NULL));
    ASSERT_ZERO(pthread_mutex_init(&new_task->task_mutex, NULL));

    for (int i = 0; i < t->capacity; i++)
        new_task->task->results[i] = -1;

    // Zwiększamy tablicę wszystkich zadań i dodajemy nowe zadanie.
    task_state **tmp = realloc(plant->tasks, (plant->n_tasks + 1) * sizeof(task_state*));
    if (!tmp) {
        free(new_task->workers);
        free(new_task);
        ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
        return ERROR;
    }

    plant->tasks = tmp;
    plant->tasks[plant->n_tasks++] = new_task;

    // Sprawdzamy, czy zadanie jest niemożliwe do wykonania, tzn. już wiadomo, że mamy za malo pracowników lub stanowiska mają za małe capacity (aby nie dodawać niepotrzebnie do pending tasks)
    // max_station_capacity - największe stanowisko.
    if (t->capacity > plant->n_workers || t->capacity > plant->max_station_capacity) {
        ASSERT_ZERO(pthread_mutex_lock(&new_task->task_mutex));
        new_task->state = TASK_ERROR;
        new_task->finished = true;
        ASSERT_ZERO(pthread_cond_broadcast(&new_task->task_done));
        ASSERT_ZERO(pthread_mutex_unlock(&new_task->task_mutex));
    } else {    // Jeśli nie mozna stwierdzić, że niemożliwe, to dodajemy do oczekujących zadań.
        task_state **tmp_p = realloc(plant->pending_tasks, (plant->n_pending_tasks + 1) * sizeof(task_state*));
        if (!tmp_p) {
            ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
            return ERROR;
        }
        plant->pending_tasks = tmp_p;
        plant->pending_tasks[plant->n_pending_tasks++] = new_task;
    }

    ASSERT_ZERO(pthread_cond_broadcast(&plant->manager_cond));
    ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));
    return PLANTOK;
}

int collect_task(task_t* t)
{

    if (plant->destroyed) {
        return ERROR;
    }
    // Niewykonalne zadanie.
    if (plant->n_workers < t->capacity || plant->max_station_capacity < t->capacity) {
        return ERROR;
    }

    task_state *task_t = NULL;
    ASSERT_ZERO(pthread_mutex_lock(&plant->mutex));
    // Sprawdzamy, czy mamy takie zadanie w tablicy zadań, aby wiedzieć jaki wynik zwrócić.
    for (int i = 0; i < plant->n_tasks; i++) {
        if (t->id == plant->tasks[i]->task->id) {
            task_t = plant->tasks[i];
            break;
        }
    }
    ASSERT_ZERO(pthread_mutex_unlock(&plant->mutex));

    if (!task_t) {
        return ERROR;
    }

    ASSERT_ZERO(pthread_mutex_lock(&task_t->task_mutex));

    // Jeśli zadanie nie jest jeszcze ukończone to czekamy.
    while (!task_t->finished)
        ASSERT_ZERO(pthread_cond_wait(&task_t->task_done, &task_t->task_mutex));

    int ret = (task_t->state == TASK_ERROR ) ? ERROR : PLANTOK;

    ASSERT_ZERO(pthread_mutex_unlock(&task_t->task_mutex));

    return ret;
}
