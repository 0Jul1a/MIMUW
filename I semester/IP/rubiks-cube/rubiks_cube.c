/*
    Zadanie zaliczeniowe 2
    Autor: Julia Stasiak
    Data: 10.12.2024
 
    Kostke reprezentujemy za pomoca 6 tablic, kazda tablica oznacza jedna sciane NxN, bedziemy obracac sciany zamieniajac elmenty w odpowiednich wierszach i kolumnach
*/
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#ifndef N
#define N 5
#endif

// przypisujemy odpowiednie cyfry do liter oznaczajacych sciany, zeby pozniej wiedziec ktora tablice wziac
#define F 0
#define R 1
#define L 2
#define D 3
#define U 4
#define B 5

typedef int kostka[N][N];       // jedna sciana
typedef int kostka_cala[6][N][N];   // cala kostka
typedef int sekwencja[6][2][3]; // 3 rozne sekwencje obrotow dla scian: 1)przod-tyl, 2)gora-dol, 3)prawo-lewo
typedef int jakie_sciany_obrot[3][4]; // jakie maja byc obroty zdefiniowane, tzn jakie sciany sie ruszaja podczas obrotu
typedef int odwrotne[6];              // jaka jest przeciwlegla sciana do tej ktora teraz krecimy(przydaje sie kiedy odwracamt N scian)

// struktura opisujaca nasza kostke czyli sciany, sekwencje ruchow i jakie siany kiedy sie obracaja
typedef struct{
    kostka_cala sciany;
    sekwencja sekwencja_obrotow;
    jakie_sciany_obrot jakie_obroty;
    odwrotne sciany_odwrotne;
}calosc;

// przypisujemy odpowiednie sekwencje, zeby wiedziec jak obracac, tzn: sekwencje obrotow: xxxx => czy_orienacja_sciany, odwrotnie, pozycja:
// czy_orientacja_sciany: jakie indeksy bedziemy brali pod uwage poniewaz, np. odbracajac sciane lewa zmieniamy sciany korzystajac z innych indeksow niz obracajac prawa
// odwrotnie: w ktora strone obracamy, np. l bedziemy obracac w ta sama strone co r'
// pozycja: jak bedziemy szli w petli poniewaz to tez jest rozne w niektorych przypadkach, np. obracajac prawa strone, poniewaz wtedy sprawdzamy kolumny od prawej czyli od ostatniej kolumny i idziemy w lewo, tak samo w przypadku obracania dolu, wtedy patrzymy na ostatni wiersz i obracamy podana ilosc scian idac w gore
void przypisz_sekwencje(calosc *kostka_rubika) {
    int pom[6][2][3] = {
        {{1, 0, 0}, {1, 1, 1}},    // przod, przod_minus
        {{0, 1, 0}, {0, 0, 0}},   // prawy, prawy minus
        {{0, 0, 1}, {0, 1, 1}},   // lewy, lewy minus
        {{1, 1, 0}, {1, 0, 0}},   // dol, dol_minus
        {{1, 0, 1}, {1, 1, 1}},   // gora, gora_minus
        {{0, 0, 1}, {0, 1, 0}}   // tyl, tyl_minus
    };

    // przepisujemy teraz to wszystko do tablicy w strukturze calosc
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 3; k++) {
                kostka_rubika->sekwencja_obrotow[i][j][k] = pom[i][j][k];
            }
        }
    }
}

void przypisz_obrot(calosc *kostka_rubika) {
    int pom[3][4] = {
        {U, F, D, B},   // te sciany odwracaja sie gdy ruszamy sciane l/r
        {R, F, B, L},   // te sciany odwracaja sie gdy ruszamy sciane u/d
        {U, L, D, R}    // te sciany odwracaja sie gdy ruszamy sciane f/b
    };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            kostka_rubika->jakie_obroty[i][j] = pom[i][j];
        }
    }
    
    // przypisujemy przeciwlegle sciany(poniewaz one beda sie tez obracac kiedy bedziemy chcieli obrocic cala kostke)
    kostka_rubika->sciany_odwrotne[0] = B;
    kostka_rubika->sciany_odwrotne[1] = L;
    kostka_rubika->sciany_odwrotne[2] = R;
    kostka_rubika->sciany_odwrotne[3] = U;
    kostka_rubika->sciany_odwrotne[4] = D;
    kostka_rubika->sciany_odwrotne[5] = F;
}

// tworzenie pojedynczej sciany
void inicjalizuj_bok(kostka sciana, int kolor){
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            sciana[i][j] = kolor;
        }
    }
}

// tworzenie calej kostki
void inicjalizuj(calosc *kostka_rubika){

    // gora kostki
    inicjalizuj_bok(kostka_rubika->sciany[U], 0);

    // lewa sciana
    inicjalizuj_bok(kostka_rubika->sciany[L], 1);

    // przednia sciana
    inicjalizuj_bok(kostka_rubika->sciany[F], 2);

    // prawa sciana
    inicjalizuj_bok(kostka_rubika->sciany[R], 3);

    // tylnia sciana
    inicjalizuj_bok(kostka_rubika->sciany[B], 4);

    // dol kostki
    inicjalizuj_bok(kostka_rubika->sciany[D], 5);
}

void wypisz_bok(kostka sciana, int i){
    for (int j = 0; j < N; j++)
        {
            printf("%d", sciana[i][j]);
        }
}

// wypisywanie stanu kostki
void wypisz(calosc *kostka_rubika){
    printf("\n");
    
    for (int i = 0; i < N; i++)
    {
        // wypisanie N+1 spacji na lewo od gornej sciany
        for (int j = 0; j <= N; j++)
        {
            printf(" ");
        }
        wypisz_bok(kostka_rubika->sciany[U], i);
        printf("\n");
    }

    for (int i = 0; i < N; i++) {
        // wypisanie sciany: lewej, przedniej, prawej i tylniej
        wypisz_bok(kostka_rubika->sciany[L], i);
        printf("|");
        wypisz_bok(kostka_rubika->sciany[F], i);
        printf("|");
        wypisz_bok(kostka_rubika->sciany[R], i);
        printf("|");
        wypisz_bok(kostka_rubika->sciany[B], i);
        printf("\n");
    }

    // wypisanie N+1 spacji na lewo od dolnej sciany
    for(int i = 0; i< N; i ++){
        // wypisanie N+1 spacji z lewej strony dolnej sciany
        for (int j = 0; j <= N; j++)
        {
            printf(" ");
        }
        wypisz_bok(kostka_rubika->sciany[D], i);
        printf("\n");
    }
}

// obrot calej sciany zgodnie z ruchem wskazowek zegara
void obrot_prawo(kostka sciana){
    for (int i = 0; i < N / 2; i++) {   // idziemy petla po "wartswach" kostki gdzie dla i=0 mamy warstwe najbardziej zewnetrzna i pozniej idziemy "do srodka"
        for (int j = i; j < N - i - 1; j++) {   // idziemy petla po kolei zamieniajac 4 elementy w aktualnej warstwie az wszystkie sie nie zamienia
            int pom = sciana[i][j];     // lewy gorny
            sciana[i][j] = sciana[N - j - 1][i];    // lewy_gorny = lewy_dolny
            sciana[N - j - 1][i] = sciana[N - i - 1][N - j - 1];    // lewy_dolny = prawy_dolny
            sciana[N - i - 1][N - j - 1] = sciana[j][N - i - 1];    // prawy_dolny = prawy_gorny
            sciana[j][N - i - 1] = pom;                             // prawy_gorny = lewy_gorny
        }
    }
}

// obrot calej sciany w lewo poprzez przekrecenie 3 razy w prawo
void obrot_lewo(kostka sciana){
    for (int i = 0; i < 3; i++)
    {
        obrot_prawo(sciana);        
    }
}

// odwracamy(gora-dol) j-ta kolumne w podanej tablicy
void obroc_kolumne(kostka sciana, int j){
    for (int i = 0; i < N/2; i++)
        {
            int pom = sciana[i][j];
            sciana[i][j] = sciana[N - 1 - i][j];
            sciana[N - 1 - i][j] = pom;
        }
}

// odwracamy kolumny w podanych scianach
void zamien_kolumny(kostka sciana1, kostka sciana2, int ile_scian){
    for (int i = N - 1; i >= N - ile_scian; i--)
    {
        obroc_kolumne(sciana1, i);
    }
    for (int i = 0; i < ile_scian ; i++) {
        obroc_kolumne(sciana2, i);
    }
}

// obracanie scian f/b
void obrot_f_b(calosc *kostka_rubika, int ile_scian, int sciany[4], int ktora_sciana, int czy_minus){
    
    int czy_orientacja_sciany = kostka_rubika->sekwencja_obrotow[ktora_sciana][czy_minus][0];
    int odwrotnie = kostka_rubika->sekwencja_obrotow[ktora_sciana][czy_minus][1];
    int pozycja = kostka_rubika->sekwencja_obrotow[ktora_sciana][czy_minus][2];
    
    for (int i = 0; i < ile_scian; i++)
    {
        for (int j = 0; j < N; j++)
        {
            // mozna by zamienic uzywajac tylko jednej zmiennej pomocniczej ale dzieki uzyciu 4 mamy bardziej czytelniejszy podzial na sciany:
            int pom1 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[0]][N - 1 - i][j] : kostka_rubika->sciany[sciany[0]][i][j]; // sciana gorna
            int pom2 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[1]][N - 1 - j][N - 1 - i] : kostka_rubika->sciany[sciany[1]][N - 1 - j][i]; // sciana lewa
            int pom3 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[2]][i][N - 1 - j] : kostka_rubika->sciany[sciany[2]][N - 1 - i][N - 1 - j]; // sciana dolna
            int pom4 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[3]][j][i] : kostka_rubika->sciany[sciany[3]][j][N - 1 - i]; // sciana prawa
            
            if (odwrotnie && czy_orientacja_sciany && pozycja) { // przod minus, zamieniamy tak samo jak przod ale w druga strone
                kostka_rubika->sciany[sciany[0]][N - 1 - i][j] = pom4;
                kostka_rubika->sciany[sciany[1]][N - 1 - j][N - 1 - i] = pom1;
                kostka_rubika->sciany[sciany[2]][i][N - 1 - j] = pom2;
                kostka_rubika->sciany[sciany[3]][j][i] = pom3;
            } else if (odwrotnie && (!czy_orientacja_sciany) && (!pozycja)){ // tyl minus, tak jak tyl ale w druga strone, oczywiscie uwzgledniamy wszedzie ile warstw mamy zamienic
                kostka_rubika->sciany[sciany[0]][i][j] = pom2;
                kostka_rubika->sciany[sciany[1]][N - 1 - j][i] = pom3;
                kostka_rubika->sciany[sciany[2]][N - 1 - i][N - 1 - j] = pom4;
                kostka_rubika->sciany[sciany[3]][j][N - 1 - i] = pom1;
            } else if (!odwrotnie && czy_orientacja_sciany && !pozycja) // przod, zamieniamy sciany przy przedniej czyli N-1 kolumna sciany l, 1 kolumna sciany r, N-1 wiers sciany u i 1 wiersz sciany d
            {
                kostka_rubika->sciany[sciany[0]][N - 1 - i][j] = pom2;
                kostka_rubika->sciany[sciany[1]][N - 1 - j][N - 1 - i] = pom3;
                kostka_rubika->sciany[sciany[2]][i][N - 1 - j] = pom4;
                kostka_rubika->sciany[sciany[3]][j][i] = pom1;
            } else if(!odwrotnie && (!czy_orientacja_sciany) && pozycja){ // tyl, zamieniamy sciany przy tylniej czyli 1 kolumna sciany l, N-1 kolumna sciany r, N-1 wiersz sciany d oraz 1 wiersz sciany u
                kostka_rubika->sciany[sciany[0]][i][j] = pom4;
                kostka_rubika->sciany[sciany[1]][N - 1 - j][i] = pom1;
                kostka_rubika->sciany[sciany[2]][N - 1 - i][N - 1 - j] = pom2;
                kostka_rubika->sciany[sciany[3]][j][N - 1 - i] = pom3;
            }
        }
    }
}

// obracanie reszty scian
void obrot_reszta(calosc *kostka_rubika, int ile_scian, int sciany[4], int ktora_sciana, int czy_minus){
    int czy_orientacja_sciany = kostka_rubika->sekwencja_obrotow[ktora_sciana][czy_minus][0];
    int odwrotnie = kostka_rubika->sekwencja_obrotow[ktora_sciana][czy_minus][1];
    int pozycja = kostka_rubika->sekwencja_obrotow[ktora_sciana][czy_minus][2];
    
    // czy idziemy od ostatniej kolumny / wiersza
    int ustaw_pozycje_poczatkowa = pozycja ? 0 : N-1;
    int ustaw_pozycje_koncowa = pozycja ? ile_scian : N - ile_scian;
    int w_ktora_strone = pozycja ? 1 : -1;
    
    for (int j = ustaw_pozycje_poczatkowa; pozycja ? (j < ustaw_pozycje_koncowa) : (j >= ustaw_pozycje_koncowa); j += w_ktora_strone)
    {
        for (int i = 0; i < N; i++)
        {                                      // dla scian u i d:                      dla scian r i l:
            int pom1 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[0]][j][i] : kostka_rubika->sciany[sciany[0]][i][j]; // sciany r/u
            int pom2 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[1]][j][i] : kostka_rubika->sciany[sciany[1]][i][j]; // sciany f/f
            int pom3 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[2]][j][i] : kostka_rubika->sciany[sciany[2]][i][j]; // sciany b/d
            int pom4 = czy_orientacja_sciany ? kostka_rubika->sciany[sciany[3]][j][i] : kostka_rubika->sciany[sciany[3]][i][N - 1 - j]; // sciany l/b
            if (odwrotnie && czy_orientacja_sciany) {       // zamiana sciany dolnej lub  gora_minus(czyli o -90stopni)
                kostka_rubika->sciany[sciany[0]][j][i] = pom2;
                kostka_rubika->sciany[sciany[1]][j][i] = pom4;
                kostka_rubika->sciany[sciany[2]][j][i] = pom1;
                kostka_rubika->sciany[sciany[3]][j][i] = pom3;
            } else if(odwrotnie && (!czy_orientacja_sciany)) { // zamiana sciany prawej lub lewej_minus
                kostka_rubika->sciany[sciany[0]][i][j] = pom2;
                kostka_rubika->sciany[sciany[1]][i][j] = pom3;
                kostka_rubika->sciany[sciany[2]][i][j] = pom4;
                kostka_rubika->sciany[sciany[3]][i][N - 1 - j] = pom1;
            } else if ((!odwrotnie) && (!czy_orientacja_sciany)) // zamiana sciany prawej_minus lub lewej
            {
                kostka_rubika->sciany[sciany[0]][i][j] = pom4;
                kostka_rubika->sciany[sciany[1]][i][j] = pom1;
                kostka_rubika->sciany[sciany[2]][i][j] = pom2;
                kostka_rubika->sciany[sciany[3]][i][N - 1 - j] = pom3;
            } else if(!odwrotnie && czy_orientacja_sciany){   // zamiana sciany gornej lub dolnej_minus
                kostka_rubika->sciany[sciany[0]][j][i] = pom3;
                kostka_rubika->sciany[sciany[1]][j][i] = pom1;
                kostka_rubika->sciany[sciany[2]][j][i] = pom4;
                kostka_rubika->sciany[sciany[3]][j][i] = pom2;
            }
        }
    }
    
}

// wykonywanie wszystkich zamian podczas obracania kostka czyli zamiana odpowiednich scian i obrot samej sciany zamienianej
void zamien(calosc *kostka_rubika, int ile_scian, int ktora_sciana, int czy_minus, int ktore_obroty){

    // patrzymy jakie sciany beda nam sie obracac
    int sciana1 = kostka_rubika->jakie_obroty[ktore_obroty][0];
    int sciana2 = kostka_rubika->jakie_obroty[ktore_obroty][1];
    int sciana3 = kostka_rubika->jakie_obroty[ktore_obroty][2];
    int sciana4 = kostka_rubika->jakie_obroty[ktore_obroty][3];
    
    // zapisujemy w tablicy pomocniczej jakie sciany sie obracaja
    int sciany[4] = {sciana1, sciana2, sciana3, sciana4};
    
    if (ktora_sciana == F || ktora_sciana == B) {
        obrot_f_b(kostka_rubika, ile_scian, sciany, ktora_sciana, czy_minus);
    } else { 
        obrot_reszta(kostka_rubika, ile_scian, sciany, ktora_sciana, czy_minus);
    }
    // sprawdzamy czy mamy obrocic o -90 stopni, poniewaz wtedy nasza ktora_sciana obroci sie w inna strone niz przy obrocie o 90
    if (czy_minus) {
        obrot_lewo(kostka_rubika->sciany[ktora_sciana]);
        if (ile_scian == N) {     // jesli mamy obrocic tyle ile jest wszystkich scian to musimy odwrocic przeciwlegla w przeciwnym kierunku
            obrot_prawo(kostka_rubika->sciany[kostka_rubika->sciany_odwrotne[ktora_sciana]]);
        }
    } else {
        obrot_prawo(kostka_rubika->sciany[ktora_sciana]);
        if (ile_scian == N) {
            obrot_lewo(kostka_rubika->sciany[kostka_rubika->sciany_odwrotne[ktora_sciana]]);
        }
    }
}

// sprawdzamy jaka sciane wczytano i wykonujemy odpowiedni ruch, ktore_obrotu = 0 dla r/l, =1 dla u/d, =2 dla f/b
void sprawdz_jaki_ruch(char sciana, calosc *kostka_rubika, int ile_scian, int czy_obrot_minus){
            if (sciana == 'f') {
                zamien(kostka_rubika, ile_scian, F, czy_obrot_minus, 2);
            } else if (sciana == 'r') {
                zamien(kostka_rubika, ile_scian, R, czy_obrot_minus, 0);
                // w przypadku obracania sciany r kiedy bedzie zamiana sciany z dolnej na tylnia tylniej na gorna lub w przypadku obrotu o -90 tylniej na dolna i gornej na tylnia, to trzeba jeszcze obrocic lustrzanie wiersze
                if (czy_obrot_minus) {
                    zamien_kolumny(kostka_rubika->sciany[U], kostka_rubika->sciany[B], ile_scian);
                } else {
                    zamien_kolumny(kostka_rubika->sciany[D], kostka_rubika->sciany[B], ile_scian);
                }
            } else if(sciana == 'l'){
                zamien(kostka_rubika, ile_scian, L, czy_obrot_minus, 0);
                // tak samo jak w przypadku obrotu sciany r, tylko ze tutaj odbijamy lustrzanie wiersz sciany tylniej i dolnej lub w przypadku obrotu o -90 tylniej i gornej
                if (czy_obrot_minus) {
                    zamien_kolumny(kostka_rubika->sciany[B], kostka_rubika->sciany[D], ile_scian);
                } else {
                    zamien_kolumny(kostka_rubika->sciany[B], kostka_rubika->sciany[U], ile_scian);
                }
            } else if(sciana == 'd'){
                zamien(kostka_rubika, ile_scian, D, czy_obrot_minus, 1);
            } else if(sciana == 'u'){
                zamien(kostka_rubika, ile_scian, U, czy_obrot_minus, 1);
            } else if(sciana == 'b'){
                zamien(kostka_rubika, ile_scian, B, czy_obrot_minus, 2);
            }
}

// wczytujemy dane i sprawdzamy czy wczytano cyfre, jesli tak to dalej wczytujemy liczbe pozniej wczytujemy sciane i obrot oraz sprawdzamy czy wczytano obrot czy moze kolejna liczbe lub nowa linie lub znowu sciane
void wczytaj(calosc *kostka_rubika){
    int c;
    // ustawiamy domyslne wartosci dla liczby scian do obrotu, obrotu oraz sciany
    char sciana = '0';
    int liczba = 0;
    int obrot = 0;
    bool zakoncz = false;
    bool bylo = false;
    // wczytujemy znak po znaku dopoki nie bedzie '.'
    while (((c = getchar()) != '.') && (!zakoncz)) {
        while (c != '\n') {
            c = (char)c;
            // wczytalismy cyfre wiec sprawdzamy jaka to liczba
            if (isdigit(c)) {
                ungetc(c, stdin);
                scanf("%d", &liczba);
                c = getchar();
                sciana = (char)c;
                c = getchar();
                obrot = c;
            } else {
                liczba = 1; // nie wczytano zadnej cyfry/liczby wiec obracamy jedna sciane
                c = (char)c;
                if (isalpha(c)) { // wczytano sciane
                    sciana = (char)c;
                    c = getchar();
                    obrot = c;
                } else {
                    obrot = c;
                }
            }
            if (obrot == '"') {   // obrot o 180 czyli 2 razy o 90
                sprawdz_jaki_ruch(sciana, kostka_rubika, liczba, 0);
                sprawdz_jaki_ruch(sciana, kostka_rubika, liczba, 0);
            } else if(obrot == '\''){   //obrot = -90
                sprawdz_jaki_ruch(sciana, kostka_rubika, liczba, 1);
            } else if(obrot == '\n'){   // obrot = 90 i wypisujemy
                sprawdz_jaki_ruch(sciana, kostka_rubika, liczba, 0);
                wypisz(kostka_rubika);
                bylo = true;   // ustawiamy na true, zeby wyjsc z petli i zeby pozniej podwojnie nie wypisac kostki, poniewaz tutaj mamy nowa linie i wypisujemy
                break;
            } else if(isalpha(obrot)){  // wczytany obrot to litera czyli mamy obrot podstawowy = 90  i oddajemy znak zeby pozniej te litere wczytac i wykonac ruch
                sprawdz_jaki_ruch(sciana, kostka_rubika, liczba, 0);
                ungetc(c, stdin);
            } else if(isdigit(obrot)){  // wczytany odwrot jest cyfra czyli liczba scian do obrotu wiec wykonujemy obrot dla wczesnije podanych danych i obrotu = 90 oraz oddajemy znak zeby mozna bylo go wczytac i wykonac ruch w kolejnym obrocie petli
                sprawdz_jaki_ruch(sciana, kostka_rubika, liczba, 0);
                ungetc(c, stdin);
            } else if(obrot == '.') {    // wczytano "." w lini wiec konczymy caly program
                zakoncz = true;
                break;
            }
            // ustawiamy domyslne wartosci
            liczba = 0;
            obrot = 0;
            sciana = '0';
            c = getchar();
        }
        // nie wypisano wczesniej kostki i nie wczytano ".", a jest nowa linia wiec wypisujemy teraz
        if ((!bylo) && (!zakoncz)) {
            wypisz(kostka_rubika);
        }
        bylo = false;
    }
}

// wykonywanie calosci programu
int main(void){
    calosc *kostka_rubika = malloc(sizeof(calosc)); // dynamicznie alokujemy pamiec aby dzialalo gdy N bedzie duze
    przypisz_sekwencje(kostka_rubika); // ustawiamy dobra sekwencje obrotow
    przypisz_obrot(kostka_rubika);     // ustawiamy jakie sciany obracaja sie przy obrocie odpowiednich scian
    inicjalizuj(kostka_rubika);        // ustawiamy domyslne wartosci do kostki
    wczytaj(kostka_rubika);            // wczytujemy dane i wykonujemy program
    free(kostka_rubika);
}

