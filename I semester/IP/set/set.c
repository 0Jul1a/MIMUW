/*
    Zadanie zaliczeniowe 1
    Autor: Julia Stasiak
    Data: 26.11.2024
*/

#include <stdio.h>
#include <stdbool.h>
#define MAX 81      // maksymalnie w talii moze byc 81 kart
#define PIERWSZE_KARTY 12

typedef int t[MAX];         // tablica przechowujaca karty w talii, pod kazdym indeksem znajduje sie jedna karta

// wczytanie danych do tablicy z pominieciem wszystkich spacji
int wczytaj(t talia){
    int i = 0;
    int karta;
    int licznik = 0;                    // liczymy ile kart zostalo wpisanych zeby wiedziec ile jest w talii i czy bedzie mozna z niej dobierac karty pozniej
    while (scanf("%d", &karta) != EOF)
    {
        talia[i] = karta;
        i++;
        ++licznik;
    }
    return licznik;
}

// wypisanie kart na stole
void wypisz(t talia, int licznik){
    printf("=");
    for (int i = 0; i < licznik; i++)
        {
            printf(" %d", talia[i]);
        }
    printf("\n");
}

// zwraca a^j, gdzie a zawsze wynosi 10
int potega(int a, int j){
    int wynik = 1;
    for (int i = 0; i < j; i++)
    {
        wynik *= a;
    }
    return wynik;
}

// sprawdza czy podane 3 liczby tworza set
bool czy_set(t talia, int i, int j, int k){
    int l1p = talia[i], l2p = talia[j], l3p = talia[k];
    int l1 = l1p,l2 = l2p, l3 = l3p;
    bool te_same = true;                                                    // zmienna dzieki ktorej petla sie zakonczy kiedy znajdziemy pierwsza niepasujaca 3 cyfr(tzn wszystkie 3 nie sa takie same lub wszystkie 3 nie sa rozne)
    int s = 3, waga;        // karty sa przedstawione za pomoca 4 cyfr dlatego s = 3 (bedziemy sprawdzali wszystkie cyfry ktore sa od konca do poczatku z pomoca liczby poczatkowej i wagi)
    while(s >= 0 && te_same == true){
        waga = potega(10,s);
        l1 = l1p/waga;                                                      // wyciagamy cyfry z podanych liczb od pierwszej do ostatniej za pomoca dzielenia przez odpowiednia potege liczby 10
        l2 = l2p/waga;
        l3 = l3p/waga;
        if ((l1 == l2 && l2 == l3)||(l1 != l2 && l2 != l3 && l1 != l3))     // sprawdzanie czy podane liczby maja cyfry na tej samej pozycji wszystkie takie same lub wszystkie rozne
        {
            te_same = true;
        } else {
            te_same = false;
        }  
        l1p %= waga;
        l2p  %= waga;
        l3p  %= waga;
        s--;
    }
    
    // jesli znalezlismy jakies niepasujace cyfry to nie moze byc set wiec zwracamy false jesli wszystko jest dobrze to mamy set
    return te_same;

}

// wykonywanie "-", czyli usuwanie z talii seta i przesuwanie reszty kart w lewo oraz skrocenie "licznika"(ilosci kart w talii) o 3 poniewaz usunieto 3 karty
void zamien(t talia, int i, int j, int k, int licznik){
    for (int s = i; s < licznik - 1; s++)                   // usuwanie 1 karty i przeniesienie reszty o 1 w lewo
    {
        talia[s] = talia[s+1];
    }
    licznik--;  
    for (int s = j-1; s < licznik - 1; s++)                 // usuwanie 2 karty i przeniesienie reszty o 1 w lewo
    {
        talia[s] = talia[s+1];
    }
    licznik--;
    for (int s = k-2; s < licznik - 1; s++)                 // usuwanie 3 karty i przeniesienie reszty o 1 w lewo
    {
        talia[s] = talia[s+1];
    }
    licznik--;
}

// szukamy pierwszych 3 kart, ktore spelniaja warunek na bycie setem
int szukaj_seta(t talia, int licznik, int licznik_stol){
    bool przerwij = true;                               // zmienna, ktora kontroluje czy juz znalezlismy seta i nie musimy dalej wykonywac petli
    int i = 0, j, k;
    if (licznik_stol >= 3)                                 // jesli na stole jest mniej niz 3 karty to nie ma seta
    {
        while (i < licznik_stol - 2 && przerwij == true)   // szukanie seta
        {
            j = i + 1;
            while (j < licznik_stol - 1 && przerwij == true)
            {
                k = j + 1;
                while (k < licznik_stol && przerwij == true)
                {
                    if (czy_set(talia, i, j, k))
                    {
                        przerwij = false;                   // znalezlismy pierwszy set wiec mozemy juz nie szukac
                    }
                    ++k;
                }
                ++j;
            }
            ++i;
        }
        if (przerwij == false)                          // znalezlismy seta wiec wypisujemy go i usuwamy karty tworzace go 
        {
            printf("- %d %d %d", talia[i-1], talia[j-1],talia[k-1]);        // wypisujemy znaleziony set
            zamien(talia, i-1, j-1, k-1, licznik);                          // usuwamy set i przesuwamy wszystkie pozostale karty w lewo
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

// sprawdzamy czy stol jest pusty czyli czy koniec gry
bool niepusty(t talia){
    if (talia[0] != '0')
    {
        return true;
    } else {
        return false;
    }
}

// sprawdzamy czy w talii sa karty ktore mozna dolozyc, tzn. kart jest wiecej niz 12
bool mozna_dolozyc(t talia, int licznik){
    if(licznik > PIERWSZE_KARTY){
        if (talia[13] != 0)
        {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// symulujemy gre 
void graj(t talia, int licznik){
    bool brak_seta = false;
    int licznik_stol;          // licznik_stol kontroluje ilosc kart na stole a licznik ilosc kart w talii
    if (licznik > PIERWSZE_KARTY)
    {
        licznik_stol = PIERWSZE_KARTY;
    } else {
        licznik_stol = licznik; 
    }
    // symulujemy gre dopoki stol nie bedzie pusty lub dopoki mamy z czego dokladac gdy nie ma seta
    while ((niepusty(talia)) && (!brak_seta))
    {   
        wypisz(talia, licznik_stol);
        if (szukaj_seta(talia, licznik, licznik_stol) == 0)     // znalezlismy set
        {
            printf("\n");
            if (licznik - licznik_stol >= 3)                    // jesli roznica liczby kart w talii i tych na stole jest wieksza niz 3, czyli nadal bedzie mozna dobrac karty z talii to zmienia sie tylko liczba kart w talii (bo po usunieciu 3 kart reszta zostala przesunieta w lewo wiec te 3, ktore byly w talii beda teraz na stole)
            {
                licznik = licznik - 3;
            } else {                                            // w przeciwntm przypadku nie ma wiecej kart w talii wiec karty na stole to tez te w talii
                licznik_stol = licznik - 3;
                licznik = licznik_stol;
            }
            if (licznik_stol > PIERWSZE_KARTY)
            {
                licznik_stol = licznik_stol - 3;
            }
        } else {                                                // nie znalezlismy seta wiec sprawdzamy czy mozna dolozyc karty
            if (mozna_dolozyc(talia, licznik)==false)           // jesli nie bylo seta w talii i nie mozna juz dolozyc kart to konczymy rozgrywke
            {
                brak_seta = true;
            } else {                                            // jesli nie ma seta ale mamy z czego dolozyc to zwiekszy sie liczba kart na stole
                if (licznik_stol <= licznik)
                {
                    licznik_stol = licznik_stol + 3;
                }
                printf("+\n");
            }
        }
    }
}

// uruchamia gre
int main(void){
    t talia = {0};              // wpisujemy same zera do tablicy
    int licznik;
    licznik = wczytaj(talia);   
    graj(talia, licznik);
    return 0;
}

