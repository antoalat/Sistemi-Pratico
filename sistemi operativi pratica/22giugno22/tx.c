/* Scrivere due programmi C, tx e rx: tx deve trasferire a rx stringhe di max 8 caratteri usando i valori
assegnati ai segnali (il parametro value della funzione sigqueue).
Il programma rx deve per prima cosa stampare il proprio pid e attendere segnali.
ill programma tx ha due parametri, il pid did rx e il messaggio.
es: tx 22255 sigmsg
(posto che rx sia in esecuzione con pid 22255, rx deve stampare sigmsg). */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LEN 8

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <PID_rx> <messaggio_max_8>\n", argv[0]);
        return 1;
    }

    pid_t pid_rx = atoi(argv[1]);
    char *msg = argv[2];
    int len = strlen(msg);

    if (len > MAX_LEN) {
        fprintf(stderr, "Messaggio troppo lungo (max 8 caratteri)\n");
        return 1;
    }

    union sigval sval;

    for (int i = 0; i < len; i++) {
        sval.sival_int = (int)msg[i];
        if (sigqueue(pid_rx, SIGUSR1, sval) == -1) {
            perror("sigqueue");
            return 1;
        }
    }

    // Invio segnale fine messaggio
    sval.sival_int = 0;
    if (sigqueue(pid_rx, SIGUSR2, sval) == -1) {
        perror("sigqueue fine");
        return 1;
    }

    return 0;
}
