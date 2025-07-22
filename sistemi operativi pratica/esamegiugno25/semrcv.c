/*
Questo e' un commento aggiunto automaticamente dallo script.
*/

    // semrecv.c
    #include <stdio.h>
    #include <stdlib.h>
    #include <signal.h>
    #include <unistd.h>

    volatile sig_atomic_t bit_received = -1;
    volatile sig_atomic_t bit_count = 0;
    volatile unsigned char current_char = 0;

    void handle_sigusr1(int signo) {
        bit_received = 0;
    }

    void handle_sigusr2(int signo) {
        bit_received = 1;
    }

    int main() {
        printf("semrecv PID: %d\n", getpid());
        fflush(stdout);

        struct sigaction sa1 = {0}, sa2 = {0};
        sa1.sa_handler = handle_sigusr1;
        sa2.sa_handler = handle_sigusr2;
        sigaction(SIGUSR1, &sa1, NULL);
        sigaction(SIGUSR2, &sa2, NULL);

        while (1) {
            pause(); // Attende segnale
            if (bit_received != -1) {
                current_char = (current_char << 1) | bit_received;
                bit_count++;

                if (bit_count == 8) {
                    if (current_char == '\0') {
                        break; // fine messaggio
                    }
                    putchar(current_char);
                    fflush(stdout);
                    current_char = 0;
                    bit_count = 0;
                }
                bit_received = -1;
            }
        }

        printf("\n");
        return 0;
    }
