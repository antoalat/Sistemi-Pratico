/*Esercizio 1: Linguaggio C (obbligatorio) 20 punti
Scrivere un programma fifotext che:
* crei una named pipe (FIFO) al pathname indicato come primo (e unico) argomento.
* apra la named pipe in lettura
* stampi ogni riga di testo ricevuta
* se la named pipe viene chiusa la riapra
* se riceve la riga "FINE" termini cancellando la named pipe.
Esempio:
fifotext /tmp/ff
....
se in un altra shell si fornisce il comando: "echo ciao > /tmp/ff", fifotext stampa ciao e rimane in attesa
(questo esperimento si può provare più volte). Con il comando "echo FINE > /tmp/ff" fifotext termina.*/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <pathname_fifo>\n", argv[0]);
        return 1;
    }

    char *fifo_path = argv[1];

    // Crea la FIFO (named pipe)
    if (mkfifo(fifo_path, 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return 1;
        }
    }

    char buffer[BUF_SIZE]; //Definisco il buffer

    while (1) {
        int fd = open(fifo_path, O_RDONLY); //APro in lettura il file descriptor
        if (fd == -1) {
            perror("open");
            unlink(fifo_path);
            return 1;
        }

        FILE *stream = fdopen(fd, "r"); //Apro lo stream del file descriptor
        if (!stream) {
            perror("fdopen");
            close(fd);
            unlink(fifo_path);
            return 1;
        }

        while (fgets(buffer, BUF_SIZE, stream)) { //Legge dallo stream e copia sul buffer la riga di grandezza BUF_SIZE
            // Rimuove eventuale newline
            buffer[strcspn(buffer, "\n")] = '\0'; //Rimuove l'eventuale endline e ci mette il carattere di fine riga

            if (strcmp(buffer, "FINE") == 0) { //Se la stringa trovata è fine allora termino qui
                fclose(stream);
                unlink(fifo_path);
                return 0;
            }

            printf("%s\n", buffer); //Altrimenti stampo la riga
            fflush(stdout);
        }

        // Se si arriva qui, la FIFO è stata chiusa dal lato scrittore
        fclose(stream);
        // Riapriamo la FIFO per continuare ad ascoltare
    }
}




