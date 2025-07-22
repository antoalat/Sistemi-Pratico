/*
Questo e' un commento aggiunto automaticamente dallo script.
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <pid semrecv> <stringa>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = atoi(argv[1]);
    char *msg = argv[2];

    for (int j = 0; j <= strlen(msg); j++) {  // Include il terminatore '\0'
        unsigned char c = msg[j];
        for (int i = 7; i >= 0; i--) {
            int bit = (c >> i) & 1;
            int sig = bit ? SIGUSR2 : SIGUSR1;
            kill(pid, sig);
            usleep(200);  // attesa per sicurezza
        }
    }
    return 0;
}
