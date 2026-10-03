; Zadanie 3
; Autor: Julia Stasiak
; Program odwraca zawartość pliku.

global _start

SYS_OPEN equ 2
SYS_CLOSE equ 3
SYS_LSEEK equ 8
SYS_MMAP equ 9
SYS_MUNMAP equ 11

SEEK_END     equ 2
PROT_READ    equ 1
PROT_WRITE   equ 2
MAP_SHARED   equ 1
SYS_EXIT equ 60
O_RDWR equ 2

BLAD_ODCZYTU equ 1

section .text

_start:

    cmp qword [rsp], 2                          ; Sprawdzenie, czy podano 2 argumenty (łącznie z nazwą programu).    
    jne .blad_zamknij

.otworz_plik:

    mov rax, SYS_OPEN                           ; Próba otwarcia pliku.
    mov rdi, qword [rsp + 16]                   ; Nazwa pliku, który odwracamy.
    mov rsi, O_RDWR                             ; Otwieramy plik z opcją odczyt + zapis.
    syscall

    test rax, rax                               ; Sprawdzenie, czy otwarcie pliku powiodło się.
    js .blad_zamknij                            ; Jeśli nie, to kończymy program z kodem 1.

    mov r12, rax                                ; Zapisujemy deskryptor pliku w rejestrze r12.

; Obliczamy rozmiar pliku poprzez użycie lseek. Argumenty:
; rdi: deskryptor pliku
; rsi: początek pliku (SEEK_SET = 0)
; rdx: koniec pliku (SEEK_END)
.sprawdzenie_rozmiaru_pliku:

    mov rax, SYS_LSEEK
    mov rdi, r12
    xor rsi, rsi
    mov rdx, SEEK_END
    syscall

    test rax, rax
    js .blad_zamknij_zakoncz                    ; Jeśli wystąpił błąd, to musimy zamknąć plik i zakończyć program z kodem 1.

    mov r13, rax                                ; Zapisujemy w rejestrze r13 rozmiar pliku, który będziemy odwracać.

    cmp r13, 2
    jb .koniec                                  ; Plik jest krótszy niż 2 bajty - zamykamy plik i kończymy program z kodem 0.

; Argumenty:
; rdi:  adres początkowy
; rsi:  długość mapowanego fragmentu (przetrzymywana w r13)
; rdx:  uprawnienia = odczyt + zapis (PROT_READ | PROT_WRITE)
; r10:  opcje mapowania (flagi) - MAP_SHARED 
;  r8:  deskryptor pliku (przetrzymywany w r12)
;  r9:  offset (0)
.mapuj_plik:

    mov rax, SYS_MMAP
    xor rdi, rdi                                ; Używamy dowolnego wolnego adresu w pamięci.
    mov rsi, r13
    mov rdx, PROT_READ | PROT_WRITE
    mov r10, MAP_SHARED
    mov r8, r12
    xor r9, r9
    syscall

    test rax, rax
    js .blad_zamknij_zakoncz                    ; Jeśli wystąpił błąd, to zamykamy plik i kończymy program z kodem 1.
                                                
    mov rdi, rax                                ; Zapisujemy w rdi (żeby później nie trzeba było wykonywać dodatkowej procedury mov) wskaźnik do zmapowanej pamięci.
    mov rsi, rdi                                ; rsi - początek (wskaźnik na pierwszy bajt).
    mov rcx, rdi
    add rcx, r13                                    
    dec rcx                                     ; rcx - koniec (wskaźnik na ostatni bajt).

; Przechodząc jednocześnie od początku i końca odwracamy ich zawartość (przenosimy początek <-> koniec), aż do momentu kiedy początek >= koniec.
; Robimy to 'blokami' 8 bajtowymi do czasu kiedy jest to możliwe, później resztę bajt po bajcie.
.odwracanie_petla:

    cmp rsi, rcx
    jge .zakoncz_petle                          ; Doszliśmy do momentu, gdy początek >= koniec - kończymy pętlę.

    mov r8, rcx
    sub r8, rsi                                 ; Sprawdzamy ile zostało bajtów do odwrócenia.
    cmp r8, 8
    jb .odwroc_pojedyncze_bajty                 ; Jeśli zostało mniej niż 8, to odwracamy pozostałe bajty oddzielnie (bajt po bajcie).

    mov r8, qword [rsi]
    mov r9, qword [rcx - 7]                     ; rcx - 7, ponieważ potrzebujemy 8 bajtów, a rcx wskazuje na ostatni.

    bswap r8                                    ; Odwracamy kolejność bajtów w r8 i r9.
    bswap r9

    mov [rsi], r9                               ; Zamieniamy odwrócone 'bloki' miejscami, początek <-> koniec.
    mov [rcx - 7], r8

    add rsi, 8
    sub rcx, 8

    jmp .odwracanie_petla

.odwroc_pojedyncze_bajty:

    mov al, byte [rsi]
    mov bl, byte [rcx]
    mov [rsi], bl                               ; Zamieniamy początek <-> koniec
    mov [rcx], al

    inc rsi
    dec rcx

    jmp .odwracanie_petla

; Po zakończeniu pętli musimy wywołać munmap.
; Argumenty:
; rdi: adres początku (przetrzymywany w rdi, więc nie musimy zapisywać)
; rsi: długość (przetrzymywana w r13)
.zakoncz_petle:

    mov rax, SYS_MUNMAP
    mov rsi, r13
    syscall

    test rax, rax
    js .blad_zamknij_zakoncz

; Zamykamy plik i kończymy program z kodem 0.
.koniec:

    xor rdi, rdi                                ; Zapisujemy w rdi kod wyjścia 0.
    jmp .zamknij_plik

; Kończymy program z kodem 1 (bez zamykania pliku, ponieważ żadnego nie otwieraliśmy).
.blad_zamknij:

    mov rdi, BLAD_ODCZYTU
    jmp .zakoncz_program

; Zamykamy plik i kończymy program z kodem 1.
.blad_zamknij_zakoncz:

    mov rdi, BLAD_ODCZYTU                       ; Zapisujemy w rdi kod wyjścia 1.

; Zamykamy plik i kończymy program z odpowiednim (zapisanym w rejestrze rdi, a później tymczasowo w rejestrze r13) kodem.
.zamknij_plik:

    mov r13, rdi                                ; Zachowujemy jakim kodem powinien zakończyć się program (czy wcześniej wystąpił błąd).
    mov rax, SYS_CLOSE
    mov rdi, r12
    syscall

    cmp rax, -4096                               ; Sprawdzenie, czy zamknięcie pliku udało się, jeśli nie to kończymy program z kodem 1.
    jae .blad_zamknij

    mov rdi, r13                                ; Przywracamy wartośc kodu, którym miał zakończyć się program.

; Kończymy program z odpowiednim kodem.
.zakoncz_program:
    mov rax, SYS_EXIT
    syscall

