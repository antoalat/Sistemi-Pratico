/*Il programma fifosig è una estensione di fifotext. Le righe che riceve attraverso la named pipe sono
composte da due numeri, il pid di un processo e il numero di un segnale. Per ogni riga correttamente
formata il segnale indicato viene mandato al processo indicato dal pid.
In un esempio simile la precedente il comando "echo 12345 15 > /tmp/ff" deve causare l'invio del
segnale 15 al processo 12345.
Scrivere il programma fifosig e un programma di prova che scriva nella pipe il proprio pid e il numero
di SIGUSR1 e controlli di ricevere SIGUSR1.*/
// fifosig.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <pathname_fifo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *fifo_path = argv[1];

    if (mkfifo(fifo_path, 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }

    char buffer[BUF_SIZE];

    while (1) {
        int fd = open(fifo_path, O_RDONLY);
        if (fd == -1) {
            perror("open");
            unlink(fifo_path);
            exit(EXIT_FAILURE);
        }

        FILE *stream = fdopen(fd, "r");
        if (!stream) {
            perror("fdopen");
            close(fd);
            unlink(fifo_path);
            exit(EXIT_FAILURE);
        }

        while (fgets(buffer, BUF_SIZE, stream)) {
            int pid, signo;
            if (sscanf(buffer, "%d %d", &pid, &signo) == 2) {
                if (kill(pid, signo) == -1) {
                    perror("kill");
                } else {
                    printf("Inviato segnale %d al processo %d\n", signo, pid);
                }
            } else {
                fprintf(stderr, "Formato non valido: %s", buffer);
            }
        }

        fclose(stream);
    }

    return 0;
}
