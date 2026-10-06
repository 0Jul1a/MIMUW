/*
    Zadanie zaliczeniowe 3
    Autor: Julia Stasiak
    Data: 07.01.2025
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#define ILE_LITER 26    // taka wielkosc musi miec tablica zawierajaca informacje o skrzynkach, ktore sa od a do z
#define ZAMIEN_INDEKS 97 // stala ktora bedziemy odejmowac zeby moc dostac indeks dla skrzynki tzn a - 0, b - 1 itp

// gdzie jest gracz i skrzynki oraz jaka maja wartosc
typedef struct
{
    int wartosc;
    int x;  // wiersz
    int y;  // kolumna
}Pole;

// dane potrzebne do odtworzenia poprzedniego wygladu planszy
typedef struct 
{
    int gracz_wartosc; 
    int x_gracza;  
    int y_gracza; 
    int skrzynia_wartosc;
    int x_skrzyni;
    int y_skrzyni;
}Zapisane_polecenia;

typedef struct
{
    int wiersze;
    int *kolumny;           // tablica gdzie dla i-tej wartosci kolumny[i] to ile jest kolumn w i-tym wierszu
    int **pola_na_planszy;  // tablica dwuwymiarowa, ktora gracz podaje na poczatku i pozniej jest modyfikowana
    Pole gracz;             // gdzie aktualnie jest gracz i jego wartosc
    Pole skrzynki[ILE_LITER];      // gdzie sa skrzynki
}Plansza;

// powiekszanie wielkosci jesli mamy za malo miejsca zaalokowanego
int wiecej(int n) {
    return n / 2 * 3 + 2;
}

void wypisz_plansze(Plansza *plansza){
    for (int i = 0; i < plansza->wiersze; i++)
    {
        for (int j = 0; j < plansza->kolumny[i]; j++)
        {
            printf("%c", (char)plansza->pola_na_planszy[i][j]);
        }
        printf("\n");
    }
}

Plansza *inicjalizuj_plansze(int ile_wierszy){
    Plansza *plansza = (Plansza *)malloc(sizeof(Plansza));
    plansza->wiersze = ile_wierszy;
    plansza->kolumny = (int *)malloc(sizeof(int) * (size_t)ile_wierszy);
    plansza->pola_na_planszy = (int **)malloc(sizeof(sizeof(int)) * (size_t)ile_wierszy);
    return plansza;
}

// aktualizujemy informacje na temat gracza 
void aktualizuj_gracza(Plansza *plansza, int wartosc, int x, int y){
    plansza->gracz.wartosc = wartosc;
    plansza->gracz.x = x;
    plansza->gracz.y = y;
}

// aktualizujemy dane na temat naszej skrzyni
void aktualizuj_skrzynie(Plansza *plansza, int wartosc, int x, int y, int indeks){
    plansza->skrzynki[indeks].wartosc = wartosc;
    plansza->skrzynki[indeks].x = x;
    plansza->skrzynki[indeks].y = y;
}

// zapisujemy gdzie byla skrzynia i gracz przed poleceniem zeby pozniej to odwtorzyc
void przypisz_wartosci_poprzednie(int *wartosc, int *x, int *y, Zapisane_polecenia *polecenia, int indeks, bool gracz){
    if (gracz)
    {
        *wartosc = polecenia[indeks].gracz_wartosc;
        *x = polecenia[indeks].x_gracza;
        *y = polecenia[indeks].y_gracza;
    } else {
        *wartosc = polecenia[indeks].skrzynia_wartosc;
        *x = polecenia[indeks].x_skrzyni;
        *y = polecenia[indeks].y_skrzyni;
    }
}

// zapamietujemy gdzie jest teraz skrzynia i gracz zeby pozniej zamienic to na pola domyslne
void przypisz_wartosci_z_planszy(Plansza *plansza, int *wartosc, int *x, int *y, int indeks){
    if (indeks == -1)
    {
        *wartosc = plansza->gracz.wartosc;
        *x = plansza->gracz.x;
        *y = plansza->gracz.y;
    } else {
        *wartosc = plansza->skrzynki[indeks].wartosc;
        *x = plansza->skrzynki[indeks].x;
        *y = plansza->skrzynki[indeks].y;
    }
}

// kiedy mozna ruszyc skrzynie to zapisujemy gdzie sie znajdowala oraz gracz przed przesunieciem
void dopisz_polecenie(Zapisane_polecenia *polecenia, Plansza *plansza, int indeks, int indeks_skrzyni){
    polecenia[indeks].gracz_wartosc = plansza->gracz.wartosc;
    polecenia[indeks].x_gracza = plansza->gracz.x;
    polecenia[indeks].y_gracza = plansza->gracz.y;
    polecenia[indeks].skrzynia_wartosc = plansza->skrzynki[indeks_skrzyni].wartosc;
    polecenia[indeks].x_skrzyni = plansza->skrzynki[indeks_skrzyni].x;
    polecenia[indeks].y_skrzyni = plansza->skrzynki[indeks_skrzyni].y;
}

// wczytujemy plansze i pierwszy pusty wiersz i wypisujemy raz 
Plansza *wczytaj_plansze(void){
    int wczytaj_znak;
    int indeks = 0;
    int ile_wierszy = 0, ile_kolumn = 0, maks_wierszy = 2, maks_kolumn = 2; 
    bool byl_pusty_wiersz = false, dwa_puste_wiersze = false;   // przestajemy wczytywac kiedy bedzie pusty wiersz po wczytaniu planszy 
    Plansza *plansza = inicjalizuj_plansze(maks_wierszy);
    wczytaj_znak = getchar();
    // wczytujemy wartosci i zapamietujemy ile jest wierszy i kolumn w wierszu
    while (!dwa_puste_wiersze)
    {
        if (wczytaj_znak == '\n')
        {
            if (byl_pusty_wiersz)   // juz byl jeden pusty wiersz czyli koniec wcztywania
            {
                dwa_puste_wiersze = true;
                break;
            }
            plansza->kolumny[ile_wierszy] = ile_kolumn; // wczytano koniec linii wiec koniec wiersza, czyli zapisujemy ile dotychczas bylo kolumn
            ile_wierszy++;
            ile_kolumn = 0; // ustawiamy docelowa liczbe kolumn poniewaz zaczynamy nowy wiersz
            byl_pusty_wiersz = true;
            if (ile_wierszy >= maks_wierszy) // Zwiększamy pamięć, jeśli potrzeba
            {  
                maks_wierszy = wiecej(maks_wierszy);
                plansza->kolumny = (int *)realloc(plansza->kolumny, sizeof(int) * (size_t)maks_wierszy);
                plansza->pola_na_planszy = (int **)realloc(plansza->pola_na_planszy, sizeof(sizeof(int)) * (size_t)maks_wierszy); // bo sizeof kolumny
            }

        } else {    // dalej wczytujemy w naszym wierszu kolumny
            byl_pusty_wiersz = false; 
            if (ile_kolumn == 0) // Jeśli to początek wiersza, alokujemy pamięć dla tego wiersza
            {  
                plansza->pola_na_planszy[ile_wierszy] = (int *)malloc(sizeof(int) * (size_t)maks_kolumn); // Domyślna liczba kolumn
            }
            if (ile_kolumn >= maks_kolumn) // jesli mamy za malo miejsca alokujemy wiecej
            {    
                maks_kolumn = wiecej(maks_kolumn);
                plansza->kolumny = (int *)realloc(plansza->kolumny, sizeof(int) * (size_t)maks_kolumn);
                plansza->pola_na_planszy[ile_wierszy] = (int *)realloc(plansza->pola_na_planszy[ile_wierszy], sizeof(int) * (size_t)maks_kolumn);
            }
            if (isalpha(tolower(wczytaj_znak))) // jesli wczytujemy litere to zapisujemy gdzie byla poniewaz jest to nasza skrzynia(uzywamy tolower bo na planszy moga byc duze litery a indeksy otrzymujemy z malych)
            {
                indeks = tolower(wczytaj_znak) - ZAMIEN_INDEKS; // moglismy wczytac duza litere i wtedy w zlym indeksie by sie zapisalo dlatego musimy litere zamienic na mala i odjac odpowiednia wartosc poniewaz wczytujemy indeks jako int
                aktualizuj_skrzynie(plansza, wczytaj_znak, ile_wierszy, ile_kolumn, indeks);    // dodajemy informacje o skrzyni do tablicy
            }
            if (wczytaj_znak == '@' || wczytaj_znak == '*') // wczytalismy gracza wiec zapisujemy gdzie byl i jaka mial wartosc
            {
                aktualizuj_gracza(plansza, wczytaj_znak, ile_wierszy, ile_kolumn);
            }
            plansza->pola_na_planszy[ile_wierszy][ile_kolumn] = wczytaj_znak;   // zapisujemy teraz wczytany znak na planszy 
            ile_kolumn++;
        }
        wczytaj_znak = getchar();
    }
    plansza->wiersze = ile_wierszy;  // zapisujemy ile bylo wszystkich wierszy po wczytaniu
    wypisz_plansze(plansza);    // pierwsza komenda to zawsze wypisanie planszy
    return plansza;

}

// czy gracz moze sie ruszyc w podana strone czyli czy wspolrzedne mieszcza sie w zakresie oraz czy nie ma przeszkod w postaci sciany lub innej skrzyni
bool bezpieczny_ruch(Plansza *plansza, int x, int y){
    if((x < plansza->wiersze && x >= 0) && (y >= 0 && y < plansza->kolumny[x]) && (plansza->pola_na_planszy[x][y] != '#') && !isalpha((int)plansza->pola_na_planszy[x][y]))
    {
        return true;
    } else {
        return false;
    }
}

// zerowanie tablicy zwiedzony zeby mozna bylo za kazym razem sprawdzac droge a nie tylko raz
void wyzeruj(int **zwiedzony, Plansza *plansza){
    for (int i = 0; i < plansza->wiersze; i++)
    {
        for (int j = 0; j < plansza->kolumny[i]; j++)
        {
            zwiedzony[i][j] = 0;
        }
    }
}

// x - wiersze, y - kolumny
// sprawdzamy czy mozemy dojsc do skrzyni, czyli do wspolrzednych zmniejszonych o ruch gdzie chcemy skrzynie przeniesc, tzn jak chcemy skrzynie przeniesc do gory to sprawdzamy czy gracz moze dojsc do wspolrzedniej pod skrzynia czyli o 1 wieksza niz skrzynia
// x_d oraz y_d to informacje w ktora strone ma sie poruszyc skrzynka wiec od miejsca gdzie jest skrzynia odejmujemy to zeby dojsc tam gdzie chcemy
bool czy_mozna_sie_ruszyc(Plansza *plansza, int x, int y, int sciana, int **zwiedzony, int x_d, int y_d){
    if (x < 0 || x >= plansza->wiersze || y < 0 || y >= plansza->kolumny[x]) 
    {
        return false;  // Sprawdzenie, czy x i y mieszczą się w granicach tablicy
    }
    // mozna dojsc do miejsca docelowego
    if ((x == plansza->skrzynki[sciana].x - x_d) && y == (plansza->skrzynki[sciana].y - y_d))
    {
        return true;
        
    }
    // juz bylismy w tym miejscu wiec konczymy
    if (zwiedzony[x][y])
    {
        return false;
    }
    // mozna sie ruszyc i nie bylismy tam jeszcze wiec wykonujemy 
    if (bezpieczny_ruch(plansza, x, y)&&!zwiedzony[x][y])
    {
        zwiedzony[x][y] = 1;
        // ruch w prawo
        if (czy_mozna_sie_ruszyc(plansza, x, y + 1, sciana, zwiedzony, x_d, y_d))
        {
            return true;
        }
        // ruch w lewo
        if (czy_mozna_sie_ruszyc(plansza, x, y - 1, sciana, zwiedzony, x_d, y_d))
        {
            return true;
        }
        // ruch do dolu
        if (czy_mozna_sie_ruszyc(plansza, x + 1, y, sciana, zwiedzony, x_d, y_d))
        {
            return true;
        }
        // ruch do gory
        if (czy_mozna_sie_ruszyc(plansza, x - 1, y, sciana, zwiedzony, x_d, y_d))
        {
            return true;
        } 
    }
    return false;
}

// 2: o 1 w dol -> x + 1
// 4: o 1 w lewo -> y - 1
// 6: o 1 w prawo -> y + 1
// 8: o 1 w gore -> x - 1
void wczytaj_ruch(int kierunek, int *x, int *y){
    switch (kierunek)
    {
    case 2:
        *x += 1;
        break;
    case 4:
        *y -= 1;
        break;
    case 6:
        *y += 1;
        break;
    case 8:
        *x -= 1;
        break;
    }
}

void poprzedni_ruch(Plansza *plansza, Zapisane_polecenia *polecenia,int ktory_indeks_polecenia, int ktory_indeks_akt){
    // zapisujemy aktualne pozycje i wartosci naszego gracza i skrzyni
    int akt_gracz = 0, akt_gracz_x = 0, akt_gracz_y = 0, akt_skrzynia = 0, akt_skrzynia_x = 0, akt_skrzynia_y = 0;
    przypisz_wartosci_z_planszy(plansza, &akt_gracz, &akt_gracz_x, &akt_gracz_y, -1);
    int indeks = (tolower(polecenia[ktory_indeks_akt - ktory_indeks_polecenia].skrzynia_wartosc) - ZAMIEN_INDEKS);
    przypisz_wartosci_z_planszy(plansza, &akt_skrzynia, &akt_skrzynia_x, &akt_skrzynia_y, indeks);
    // sprawdzamy jakie pola maja byc po zmienieniu pozycji gracza i skrzyni
    plansza->pola_na_planszy[akt_gracz_x][akt_gracz_y] = (akt_gracz == '*')?'+':'-';
    plansza->pola_na_planszy[akt_skrzynia_x][akt_skrzynia_y] = (isupper((char)akt_skrzynia))?'+':'-';
    // zapisujemy gdzie powinien sie znalezc gracz i skrzynia korzystacjac z tego co mamy zapisane w tablicy polecenia
    indeks = ktory_indeks_akt - ktory_indeks_polecenia;
    przypisz_wartosci_poprzednie(&akt_gracz, &akt_gracz_x, &akt_gracz_y, polecenia, indeks,1);
    przypisz_wartosci_poprzednie(&akt_skrzynia, &akt_skrzynia_x, &akt_skrzynia_y, polecenia, indeks,0);
    // zapisujemy na dobrych miejscach na planszy gracza i skrzynie
    plansza->pola_na_planszy[akt_gracz_x][akt_gracz_y] = akt_gracz;
    plansza->pola_na_planszy[akt_skrzynia_x][akt_skrzynia_y] = akt_skrzynia;
    // musimy zaktualizowac nasze informacje o aktualnej pozycji i wartosci gracza oraz skrzyni
    aktualizuj_gracza(plansza, akt_gracz, akt_gracz_x, akt_gracz_y);
    indeks = tolower(akt_skrzynia) - ZAMIEN_INDEKS;
    aktualizuj_skrzynie(plansza, akt_skrzynia, akt_skrzynia_x, akt_skrzynia_y, indeks);
}

void przypisz_wspolrzedne(int *x, int *y, int x_nowe, int y_nowe){
    *x = x_nowe;
    *y = y_nowe;
}

// wczytujemy wzszystkie polecenia i wykonujemy
void wczytaj_polecenia(Plansza *plansza){
    int znak_skrzyni, znak_kierunku, skrzynia;
    int gracz_wartosc = 0, skrzynia_wartosc = 0;
    int ktory_indeks_polecenia = 1, ktory_indeks_akt = 0, indeks_skrzyni = 0, maks_rozmiar = 2;
    bool jeszcze_nie_wypisano = true, nie_bylo_konca = true, nie_bylo_zera = true;
    int x = 0; int y = 0; int x_p = 0;int y_p = 0;int  x_skrzyni_przed = 0;int y_skrzyni_przed = 0; // x, y - nowe pozycje, x_p,y_p - jaki ruch wykonujemy
    
    Zapisane_polecenia *polecenia = (Zapisane_polecenia *)malloc(sizeof(Zapisane_polecenia) *(size_t)maks_rozmiar); // tablica gdzie zapisujemy informacje kiedy ruch jest wykonywalny
    int **zwiedzony = (int **)malloc(sizeof(int *) * (size_t)plansza->wiersze); // tablica potzrebna do sprawdzenia czy mozna dojsc do odpowiedniego miejsca na planszy
    for (int ii = 0; ii < plansza->wiersze; ii++) 
    {
        zwiedzony[ii] = (int *)calloc((size_t)plansza->kolumny[ii], sizeof(int));
    }
    znak_skrzyni = getchar();
    while (znak_skrzyni != '.' && (nie_bylo_konca))
    {
        znak_kierunku = getchar();
        if (znak_kierunku == '.')
        {
            nie_bylo_konca = false;
            break;
        } else if((znak_skrzyni == '0' || znak_kierunku == '0')) {
            // sprawdzamy czy mamy jeszcze mozliwosc cofania
            if(ktory_indeks_akt - ktory_indeks_polecenia >= 0){
                poprzedni_ruch(plansza, polecenia, ktory_indeks_polecenia, ktory_indeks_akt);
                if(ktory_indeks_akt > ktory_indeks_polecenia) ktory_indeks_polecenia++; // jesli bylo wiecej polecen niz jest zapisanych w tablicy to mozemy zwiekszyc(zeby pozniej wykonywac o 1 mniejszy a nie ten sam)
                nie_bylo_zera = false;
                if (znak_skrzyni == '\n')  // czyli mielismy np 00 i koniec wiersza
                {
                    wypisz_plansze(plansza);
                    jeszcze_nie_wypisano = false;
                }
                if(znak_skrzyni == '0'){ 
                    if (znak_kierunku == '\n') 
                    {
                        wypisz_plansze(plansza);
                        jeszcze_nie_wypisano = false;
                    } else {    // musimy zwrocic wczytany znak zeby mozna bylo poprawnie odczytac
                        ungetc(znak_kierunku, stdin);
                    }
                }
            } else if(ktory_indeks_akt == 0) {    // ktory_indeks_akt = 0 czyli poczatek
                if (znak_kierunku == '0')
                {
                    ungetc(znak_kierunku, stdin);
                } else if(znak_skrzyni == '0') {
                    if (znak_kierunku == '\n')
                    {
                        wypisz_plansze(plansza);
                        jeszcze_nie_wypisano = false;
                    } else { // mamy znak_kierunku != 0 i != '\n' oraz znak_skrzyni = 0, czyli znak_kierunku jest poleceniem
                        ungetc(znak_kierunku, stdin);
                    }
                }
            }
        } else {// wczytano polecenie ktore nie bytlo zerem  
            indeks_skrzyni = znak_skrzyni - ZAMIEN_INDEKS; // zamieniamy wczytany znak tak aby mogl zostac uzyty jako indeks przy zapisywaniu do skrzyni
            if (tolower(plansza->skrzynki[indeks_skrzyni].wartosc) == znak_skrzyni) // wczytalismy skrzynie, ktora jest na planszy
            {
                if (ktory_indeks_akt >= maks_rozmiar) // musimy zwiekszyc rozmiar tablicy przechowujacej polecenia
                {
                        maks_rozmiar = wiecej(maks_rozmiar);
                        polecenia = (Zapisane_polecenia *)realloc(polecenia, sizeof(Zapisane_polecenia) * (size_t)maks_rozmiar);
                }
                znak_kierunku = znak_kierunku - '0';    // odczytujemy w ktora strone przesuwamy skrzynie
                wczytaj_ruch(znak_kierunku, &x, &y);
                skrzynia = tolower(plansza->skrzynki[indeks_skrzyni].wartosc);
                przypisz_wspolrzedne(&x_skrzyni_przed, &y_skrzyni_przed, plansza->skrzynki[indeks_skrzyni].x, plansza->skrzynki[indeks_skrzyni].y);
                przypisz_wspolrzedne(&x_p, &y_p, x, y);
                przypisz_wspolrzedne(&x, &y, x + x_skrzyni_przed, y + y_skrzyni_przed);
                if (bezpieczny_ruch(plansza, x_skrzyni_przed - x_p, y_skrzyni_przed - y_p) && bezpieczny_ruch(plansza, x, y))   // czy w ogole skrzynia moze sie ruszyc i czy gracz moze zajac miejsce odpowiednie
                {
                    if (czy_mozna_sie_ruszyc(plansza, plansza->gracz.x, plansza->gracz.y, skrzynia - ZAMIEN_INDEKS, zwiedzony, x_p, y_p))
                    {
                        if (ktory_indeks_akt >= ktory_indeks_polecenia && !nie_bylo_zera)   // jesli wczytano zero i nie jest mniej wiecej polecen zapisanych niz tych wczytanych to zmieniamy liczbe polecen wczytanych o ilosc tych ktore zostaly wlasnie wykonane
                        {
                            ktory_indeks_akt = ktory_indeks_akt - ktory_indeks_polecenia + 1;   // +1 bo ktory_indeks_polecenia zaczyna sie od 1
                        } 
                        // zapisujemy potzrebne informacje o skrzynkach i graczach do naszej tablicy polecenia
                        dopisz_polecenie(polecenia, plansza, ktory_indeks_akt, indeks_skrzyni);
                        // najpierw uzupelniamy plansze w miejscach gdzie wczesniej byla skrzynia i gracz
                        plansza->pola_na_planszy[x_skrzyni_przed][y_skrzyni_przed] = isupper((char)plansza->pola_na_planszy[x_skrzyni_przed][y_skrzyni_przed])?'+':'-';
                        plansza->pola_na_planszy[plansza->gracz.x][plansza->gracz.y] = (plansza->pola_na_planszy[plansza->gracz.x][plansza->gracz.y] == '*')?'+':'-';
                        // teraz aktualizujemy informacje o graczu i skrzynce
                        gracz_wartosc = (plansza->pola_na_planszy[x_skrzyni_przed][y_skrzyni_przed] == '+')?'*':'@';
                        aktualizuj_gracza(plansza, gracz_wartosc, x_skrzyni_przed, y_skrzyni_przed);
                        skrzynia_wartosc = (plansza->pola_na_planszy[x][y] == '+')?toupper(znak_skrzyni):znak_skrzyni;
                        aktualizuj_skrzynie(plansza, skrzynia_wartosc, x, y, indeks_skrzyni);
                        // i na koniec wpisujemy w dobre miejsce gracza i skrzynke
                        plansza->pola_na_planszy[x][y] = plansza->skrzynki[indeks_skrzyni].wartosc;
                        plansza->pola_na_planszy[x_skrzyni_przed][y_skrzyni_przed] = plansza->gracz.wartosc;
                        // mielismy dobre polecenie wiec zwiekszamy nasz ktory_indeks_akt, ale nie bylo zera wiec musimy ustawic wartosc domyslna dla indeksu polecenia
                        ktory_indeks_akt++;
                        ktory_indeks_polecenia = 1;
                        nie_bylo_zera = true;
                    }
                }
                wyzeruj(zwiedzony, plansza);
                przypisz_wspolrzedne(&x, &y, 0, 0);
                przypisz_wspolrzedne(&x_p, &y_p, 0, 0);// ustawiamy wartosci domyslne indeksow
            }   
         }
        znak_skrzyni = getchar();
        while (znak_skrzyni == '\n') {  // wypisujemy plansze  
            if (jeszcze_nie_wypisano == true)
            {
                wypisz_plansze(plansza);
            }
            znak_skrzyni = getchar();
        }
        jeszcze_nie_wypisano = true;
    }
    // zwalniamy pamiec
    for (int ii = 0; ii < plansza->wiersze; ii++) 
    {
        free(zwiedzony[ii]);
    }
    free(zwiedzony);
    free(polecenia);
}

void usun_plansze(Plansza *plansza) {
    for (int i = 0; i < plansza->wiersze; i++) 
    {
        free(plansza->pola_na_planszy[i]);
    }
    free(plansza->pola_na_planszy);
    free(plansza->kolumny);
    free(plansza);
}
int main(void){
    Plansza *plansza = wczytaj_plansze();
    wczytaj_polecenia(plansza);
    usun_plansze(plansza);
    return 0;
}

