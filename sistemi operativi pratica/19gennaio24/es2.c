
/*Scrivere un programma pargrcv che crei una named pipe (il pathname è passato come parametro) e
quando si ridireziona nella named pipe la sequenza di caratteri creata da argsend dell'esercizio 1,
pargrcv deve eseguire il comando.
$ ./pargrcv /tmp/mypipe
crea la named pipe /tmp/mypipe e si mette in attesa.
Da un'altro terminale il comando:
$ ./argsend ls -l /tmp > /tmp/mypipe
fa eseguire il comando "ls -l /" a pargrcv*/




#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <named_pipe_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *fifo_path = argv[1];

    // Creazione named pipe (FIFO) se non esiste
    if (mkfifo(fifo_path, 0666) == -1) {
        perror("mkfifo");
        // Se già esiste, prosegui comunque
    }

    // Apro la FIFO in lettura (bloccante finché qualcuno scrive)
    int fd = open(fifo_path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Buffer per leggere i dati
    char str[MAX];
    int l = read(fd, str, MAX);
    if (l == -1) {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    // Conto quanti argomenti (stringhe separate da '\0')
    int c = 0;
    for (int i = 0; i < l; i++) {
        if (str[i] == '\0') {
            c++;
        }
    }

    // Creo array di puntatori per gli argomenti
    char **args = malloc(sizeof(char *) * (c + 1));
    if (args == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    args[0] = str;
    int k = 1;
    for (int i = 0; i < l; i++) {
        if (str[i] == '\0' && i + 1 < l) {
            args[k++] = &str[i + 1];
        }
    }
    args[c] = NULL;

    // Eseguo il comando
    execvp(args[0], args);

    // Se execvp fallisce
    perror("execvp");
    free(args);
    exit(EXIT_FAILURE);
}
