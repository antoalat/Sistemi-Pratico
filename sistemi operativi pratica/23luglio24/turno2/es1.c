/*Esercizio 1: Linguaggio C (obbligatorio) 20 punti Scrivere un programma che inverta i byte di un file:
(il primo byte deve diventare l'ultimo, e viceversa; il secondo carattere deve essere scambiato col
penultimo e così via).
Il programma deve fare la trasformazione sul posto (senza usare altri file temporanei) e deve usare al
massimo 2K byte per i buffer.
Sintassi: inverti pathname
(cioé ha un solo parametro).
Se si vuole creare un file con 1MB di dati random per fare una prova usare:
dd if=/dev/urandom of=file_input bs=1024 count=1024
applicando inverti due volte al file casuale deve tornare il file dato
Attenzione: il file ha lunghezza arbitraria*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 1024

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <file_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *path = argv[1];
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("Errore stat");
        return EXIT_FAILURE;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Non è un file regolare.\n");
        return EXIT_FAILURE;
    }

    off_t filesize = st.st_size;

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Errore apertura file");
        return EXIT_FAILURE;
    }

    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];

    off_t start = 0;
    off_t end = filesize;

    while (start < end) {
        int chunk = BUF_SIZE;
        if (end - start < 2 * BUF_SIZE) {
            chunk = (end - start) / 2;
        }

        // Leggi chunk da inizio
        if (lseek(fd, start, SEEK_SET) == -1) {
            perror("Errore lseek inizio");
            close(fd);
            return EXIT_FAILURE;
        }
        if (read(fd, buf1, chunk) != chunk) {
            perror("Errore read inizio");
            close(fd);
            return EXIT_FAILURE;
        }

        // Leggi chunk da fine
        if (lseek(fd, end - chunk, SEEK_SET) == -1) {
            perror("Errore lseek fine");
            close(fd);
            return EXIT_FAILURE;
        }
        if (read(fd, buf2, chunk) != chunk) {
            perror("Errore read fine");
            close(fd);
            return EXIT_FAILURE;
        }

        // Scrivi buf2 (rovesciato) all'inizio
        if (lseek(fd, start, SEEK_SET) == -1) {
            perror("Errore lseek scrittura inizio");
            close(fd);
            return EXIT_FAILURE;
        }
        for (int i = chunk - 1; i >= 0; i--) {
            if (write(fd, &buf2[i], 1) != 1) {
                perror("Errore scrittura inizio");
                close(fd);
                return EXIT_FAILURE;
            }
        }

        // Scrivi buf1 (rovesciato) alla fine
        if (lseek(fd, end - chunk, SEEK_SET) == -1) {
            perror("Errore lseek scrittura fine");
            close(fd);
            return EXIT_FAILURE;
        }
        for (int i = chunk - 1; i >= 0; i--) {
            if (write(fd, &buf1[i], 1) != 1) {
                perror("Errore scrittura fine");
                close(fd);
                return EXIT_FAILURE;    
            }
        }   
        start += chunk;
        end -= chunk;
    }
    close(fd);
    return EXIT_SUCCESS;
}
