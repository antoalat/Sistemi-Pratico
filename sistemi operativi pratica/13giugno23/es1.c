/* Facendo uso dei timerfd (vedi timerfd_create) scrivere un programma che stampi una stringa a
intervalli regolari. (il parametro ha tre campi separati da virgola: il numero di iterazioni, l'intervallo fra
iterazione e la stringa da salvare:
tfdtest 4,1.1,ciao
deve stampare ciao quattro volte, rispettivamente dopo 1.1 secondi, 2.2 secondi, 3.3 secondi 4.4
secondi e terminare. L'esecuzione dovrebbe essere simile alla seguente:
$ tfdtest 4,1.1,ciao
1.100267 ciao
2.200423 ciao
3.300143 ciao
4.400053 ciao*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

int main(int argc, char **argv) {
    // Controlla che ci sia un argomento passato al programma
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <n_iterazioni,intervallo,stringa>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Copia l’argomento perché strtok modifica la stringa originale
    char *input = strdup(argv[1]);
    if (!input) {
        perror("strdup");
        return EXIT_FAILURE;
    }

    // Estrai i 3 token con strtok
    char *tok1 = strtok(input, ","); // Numero di iterazioni
    char *tok2 = strtok(NULL, ","); // Intervallo di tempo
    char *tok3 = strtok(NULL, ","); // Stringa da stampare

    // Controlla che tutti i token siano validi
    if (!tok1 || !tok2 || !tok3) {
        fprintf(stderr, "Formato invalido. Es: 4,1.1,ciao\n");
        free(input);
        return EXIT_FAILURE;
    }

    // Converte i token in valori utili
    int n_iter = atoi(tok1); // Numero di iterazioni
    double interval = strtod(tok2, NULL); // Intervallo di tempo in secondi
    char *message = tok3; // Stringa da stampare

    free(input);  // Dopo il parsing, possiamo liberare la stringa duplicata

    // Crea il timerfd
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1) {
        perror("timerfd_create");
        return EXIT_FAILURE;
    }

    // Imposta il timer iniziale e l’intervallo
    struct itimerspec ts;
    ts.it_value.tv_sec = (time_t)interval; // Secondi interi
    ts.it_value.tv_nsec = (interval - (time_t)interval) * 1e9; // Nanosecondi
    ts.it_interval = ts.it_value; // L'intervallo è uguale al valore iniziale

    // Configura il timer
    if (timerfd_settime(tfd, 0, &ts, NULL) == -1) {
        perror("timerfd_settime");
        close(tfd);
        return EXIT_FAILURE;
    }

    // Tempo iniziale per calcolare il tempo trascorso
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Loop per n_iter scadenze
    for (int i = 0; i < n_iter; i++) {
        uint64_t expirations;
        // Legge il numero di scadenze dal timerfd
        if (read(tfd, &expirations, sizeof(expirations)) != sizeof(expirations)) {
            perror("read");
            close(tfd);
            return EXIT_FAILURE;
        }

        // Calcola il tempo trascorso
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        double elapsed = (now.tv_sec - start.tv_sec) + (now.tv_nsec - start.tv_nsec) / 1e9;
        // Stampa il tempo trascorso e la stringa
        printf("%.6f %s\n", elapsed, message);
        fflush(stdout); // Forza la stampa immediata
    }

    // Chiude il timerfd
    close(tfd);
    return EXIT_SUCCESS;
}