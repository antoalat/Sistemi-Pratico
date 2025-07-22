/* Esercizio 2: Linguaggio C: 10 punti
pinverti è una estensione di inverti. Ha due parametri:
pinverti nprocessi pathname
dove nprocessi è il numero di processi paralleli che devono compiere la trasformazione.
Il file da invertire deve essere diviso in nprocessi blocchi.
Per esempio per invertire un file da 800 byte con 4 processi, il primo processo inverte i byte 0..99 con i
byte 700,799, il secondo processo inverte 100,199 con 600,699 .... l'ultimo 300,399 con 400,499*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUF_SIZE 1024

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Uso: %s <nproc> <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *path = argv[2];
    char *nproc = argv[1];
    char *stopstring;
    long l;

    l = strtol(nproc, &stopstring, 10);
    if (*stopstring != '\0' || l <= 0) {
        fprintf(stderr, "Errore: numero di processi non valido.\n");
        return EXIT_FAILURE;
    }

    struct stat st;
    if(stat(path, &st) == -1)
    {
        perror("Errore stat");
        return EXIT_FAILURE;
    }

    if(!S_ISREG(st.st_mode))
    {
        fprintf(stderr, "Il pathname specificato non è un file regolare.\n");
        return EXIT_FAILURE;
    }

    off_t size = st.st_size;
    off_t chunk_for_process = size / (2 * l); // ogni processo lavora su chunk doppio: davanti e dietro

    for(int i = 0; i < l; i++)
    {
        pid_t pid = fork();
        if(pid == -1)
        {
            perror("Errore fork");
            return EXIT_FAILURE;
        }
        else if(pid == 0)
        {
            off_t start = chunk_for_process * i;
            off_t end = size - chunk_for_process * i;
            char buf1[BUF_SIZE];
            char buf2[BUF_SIZE];
            int fd = open(path, O_RDWR);
            if (fd < 0) {
                perror("Errore apertura file");
                exit(EXIT_FAILURE);
            }

            while (start < end) {
                int chunk = BUF_SIZE;
                if (end - start < 2 * BUF_SIZE) {
                    chunk = (end - start) / 2;
                }

                if (chunk <= 0) break;

                // Leggi da inizio
                if (lseek(fd, start, SEEK_SET) == -1) {
                    perror("Errore lseek inizio");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                if (read(fd, buf1, chunk) != chunk) {
                    perror("Errore read inizio");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                // Leggi da fine
                if (lseek(fd, end - chunk, SEEK_SET) == -1) {
                    perror("Errore lseek fine");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                if (read(fd, buf2, chunk) != chunk) {
                    perror("Errore read fine");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                // Scrivi buf2 (rovesciato) al posto di buf1
                if (lseek(fd, start, SEEK_SET) == -1) {
                    perror("Errore lseek scrittura inizio");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                for (int i = chunk - 1; i >= 0; i--) {
                    if (write(fd, &buf2[i], 1) != 1) {
                        perror("Errore scrittura inizio");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                }

                // Scrivi buf1 (rovesciato) al posto di buf2
                if (lseek(fd, end - chunk, SEEK_SET) == -1) {
                    perror("Errore lseek scrittura fine");
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                for (int i = chunk - 1; i >= 0; i--) {
                    if (write(fd, &buf1[i], 1) != 1) {
                        perror("Errore scrittura fine");
                        close(fd);
                        exit(EXIT_FAILURE);    
                    }
                }   

                start += chunk;
                end -= chunk;
            }
            close(fd);
            exit(EXIT_SUCCESS);
        }
    }

    // ATTENDO I FIGLI
    for (int i = 0; i < l; i++) {
        int status;
        wait(&status);
        if (!WIFEXITED(status)) {
            fprintf(stderr, "Un processo figlio è terminato inaspettatamente.\n");
        }
    }

    return EXIT_SUCCESS;
}
