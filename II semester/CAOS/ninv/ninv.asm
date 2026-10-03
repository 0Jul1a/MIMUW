; Zadanie 4
; Autor: Julia Stasiak

; Korzystamy z następującego algorytmu:
; Alokujemy pamięć roboczą (później nazywaną R), która będzie słuzyć do symulowania sprawdzenia,
; czy 'y' z ustawioną jedynką na danym bicie, dalej będzie spełniał nierówność: x * y <= 2 ^ n < x(y+1),
; tzn. będziemy sprawdzać, czy w danym momencie R >= x
; Jeśli tak, to odejmujemy R - x i ustawiamy odpowiedni bit w 'y'.

; Idąc w pętli for (i = n - 1; i >= 0; i--) wykonujemy następujące działania:
; - przesuwamy po kolei bity w słowie o 1 bit w lewo, uwzględniając przeniesienie (najbardziej znaczący bit w poprzednim słowie),
; - sprawdzamy, czy R jest większe od 'x':
;   - jeśli tak, to odejmujemy 'i', a następnie ustawiamy bit 'i' w 'y' na 1,
;   - jeśli nie, to zmniejszamy 'i', a następnie wracamy na początek pętli.

; --------------------------------------------------------------------
;   Argumenty:
;   rdi - y
;   rsi - x   
;   rdx - n 
;   r10 - R
;   r8 - i (iterator w pętli po bitach w y, na początku równy n - 1)
;   rcx - (n/64), rozmiar x, y oraz R
; --------------------------------------------------------------------

global ninv
section .text

ninv:

    mov rcx, rdx
    shr rcx, 6  

    xor r8, r8                      ; Tymczasowy iterator do wyzerowania y.

.clear_y:

    cmp r8, rcx
    jae .allocate_memory

    mov qword [rdi + 8 * r8], 0    
    inc r8
    jmp .clear_y

.allocate_memory:

    mov rax, rcx
    shl rax, 3                      ; Obliczamy, ile bajtów potrzebujemy.

    sub rsp, rax                    ; Rezerwujemy miejsce na stosie.
    mov r10, rsp                    ; Teraz pod r10 mamy dostęp do pamięci, którą będziemy wykorzystywać do porównywania i odejmowania.
   
    mov qword [r10], 1              ; Na starcie i = n - 1, więc ustawiamy 1 w pamięci roboczej, aby po pierwszym przesunięciu zachodziło R = 2.
    mov r8, 1

.clear_allocated_memory:            

    cmp r8, rcx
    jae .prepare_for_loop_i

    mov qword [r10 + 8 * r8], 0
    inc r8
    jmp .clear_allocated_memory

.prepare_for_loop_i:

    mov r8, rdx
    dec r8                          ; Ustawiamy wartość iteratora r8 na  n - 1. 

.loop_i:                            ; Pętla po bitach od i = n - 1 do 0.

    cmp r8, 0
    jl .done

    xor r9, r9                      ; Iterator służący do przejścia po R.
    xor r11, r11                    ; Rejestr, w którym zapisujemy przeniesienie.

.shift:

    cmp r9, rcx                     ; Przeszliśmy całe R, można zacząć sprawdzać, czy R >= x.
    jae .prepare_for_compare

    mov rax, qword [r10 + 8 * r9]  
    mov rdx, rax                    ; Zapisujemy wartość przed przesunięciem, żeby można było później wyznaczyć carry.
    shl rax, 1                      ; Przesuwamy wszystkie bity o 1 bit w lewo.
    or rax, r11                     ; Uwzględniamy poprzednie przeniesienie.
    mov qword [r10 + 8 * r9], rax   ; Zapisujemy nową wartość.

    mov r11, rdx                    ; Przywracamy wartość przed przesunięciem, aby uzyskać bit, który musimy przenieść do wyższego słowa.
    shr r11, 63                     ; Wyciągamy najbardzięj znaczący bit, aby dostać przeniesienie.
    inc r9
    jmp .shift

.prepare_for_compare:

    test r11, r11                   ; Jeśli zostało przeniesienie, to na pewno można odjąć.
    jnz .substract

    mov r9, rcx                     ; Iterator od najbardziej znaczącego indeksu w x sprawdzający, czy R >= x.
    dec r9                          

.compare:

    cmp r9, 0
    jl .substract                   ; Jest równe, więc można odjąć.

    mov rax, qword [r10 + 8 * r9]
    mov rdx, qword [rsi + 8 * r9]

    cmp rax, rdx
    ja .substract                   ; Jest większe, więc można odjąć.
    jb .do_not_substract            ; Jest mniejsze, więc nie odejmujemy i idziemy dalej w pętli po zmniejszeniu r8.

    dec r9                          ; Równe, więc zmniejszamy i sprawdzamy dalej.
    jmp .compare

.do_not_substract:

    dec r8
    jmp .loop_i

.substract:

    clc                             ; Zerujemy CF, jeśli został jakiś stary.
    xor r9, r9

.substract_loop:

    lahf                            ; Zapisujemy wartość CF, poniewaz cmp moze ją zmienić.
    
    cmp r9, rcx
    jae .set_bit_in_y

    sahf                            ; Przywracamy wartość CF.

    mov rax, qword [r10 + 8 * r9]
    sbb rax, qword [rsi + 8 * r9]   
    mov qword [r10 + 8 * r9], rax

    inc r9
    jmp .substract_loop

.set_bit_in_y:

    mov r9, r8                      ; Zapisujemy, na ktorym bicie zmieniamy.
    shr r9, 6                       ; Obliczamy jaki to będzie indeks.
    mov rax, r8
    and rax, 63                     ; Obliczamy, który bit w tym słowie.
    
    bts qword [rdi + 8 * r9], rax   ; Ustawiamy 1 na odpowiendim bicie w y.
    dec r8
    jmp .loop_i

.done:

    mov rax, rcx
    shl rax, 3
    add rsp, rax                    ; Zwalniamy zaalokowane wcześniej miejsce.

    ret

