#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LEN 8

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <PID_rx> <messaggio>\n", argv[0]);
        return 1;
    }

    pid_t pid_rx = atoi(argv[1]);
    char *msg = argv[2];
    int len = strlen(msg);

    union sigval sval;

    // Invio a blocchi da 8 caratteri
    for (int start = 0; start < len; start += MAX_LEN) {
        int block_len = (len - start) < MAX_LEN ? (len - start) : MAX_LEN;
        for (int i = 0; i < block_len; i++) {
            sval.sival_int = (int)msg[start + i];
            if (sigqueue(pid_rx, SIGUSR1, sval) == -1) {
                perror("sigqueue");
                return 1;
            }
            usleep(1000); // Piccola pausa per sicurezza nell'invio
        }
        // Segnale fine blocco
        sval.sival_int = 0;
        if (sigqueue(pid_rx, SIGUSR2, sval) == -1) {
            perror("sigqueue fine blocco");
            return 1;
        }
        usleep(5000); // Pausa per consentire al receiver di processare
    }

    // Segnale fine messaggio
    if (sigqueue(pid_rx, SIGTERM, sval) == -1) {
        perror("sigqueue fine messaggio");
        return 1;
    }

    return 0;
}
