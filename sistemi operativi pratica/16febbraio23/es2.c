/*
Esercizio: Linguaggio C – 20 punti

Scrivere un programma di copia parallelo di file con grado di parallelismo scelto da linea di comando.

Uso: pcp -j NUM file1 file2

Il programma crea NUM processi figli:
- Ogni processo copia 1/NUM del file file1 in file2.
- Il padre attende tutti i figli con waitpid().
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

#define BUF_SIZE 1024

// Copia la porzione [start, end) del file in_fd nel file out_fd
void copy_range(int in_fd, int out_fd, off_t start, off_t end) {
    char buffer[BUF_SIZE];

    lseek(in_fd, start, SEEK_SET);
    lseek(out_fd, start, SEEK_SET);

    off_t remaining = end - start;

    while (remaining > 0) {
        ssize_t to_read = remaining < BUF_SIZE ? remaining : BUF_SIZE;
        ssize_t read_bytes = read(in_fd, buffer, to_read);
        if (read_bytes <= 0) break;
        write(out_fd, buffer, read_bytes);
        remaining -= read_bytes;
    }
}

// Funzione principale di copia parallela
int pcp(char *file1, char *file2, int n_proc) {
    int in_fd = open(file1, O_RDONLY);
    if (in_fd < 0) {
        perror("Errore apertura file1");
        return 1;
    }

    struct stat st;
    if (stat(file1, &st) == -1) {
        perror("Errore stat");
        close(in_fd);
        return 1;
    }

    off_t size = st.st_size;

    int out_fd = open(file2, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("Errore apertura file2");
        close(in_fd);
        return 1;
    }

    if (ftruncate(out_fd, size) == -1) {
        perror("Errore ftruncate");
        close(in_fd);
        close(out_fd);
        return 1;
    }

    off_t chunk = size / n_proc;
    off_t rest = size % n_proc;

    for (int i = 0; i < n_proc; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Errore fork");
            close(in_fd);
            close(out_fd);
            return 1;
        }
        if (pid == 0) {
            off_t start = i * chunk;
            off_t end = (i == n_proc - 1) ? (start + chunk + rest) : (start + chunk);
            copy_range(in_fd, out_fd, start, end);
            close(in_fd);
            close(out_fd);
            exit(0);
        }
    }

    // Padre: attende tutti i figli
    for (int i = 0; i < n_proc; i++) {
        int status;
        waitpid(-1, &status, 0);
    }

    close(in_fd);
    close(out_fd);
    return 0;
}

// Main: parsing di -j e chiamata a pcp()
int main(int argc, char **argv) {
    int opt, n_proc = -1;

    while ((opt = getopt(argc, argv, "j:")) != -1) {
        if (opt == 'j') {
            n_proc = atoi(optarg);
        } else {
            fprintf(stderr, "Uso: %s -j NUM file1 file2\n", argv[0]);
            return 1;
        }
    }

    if (n_proc <= 0 || argc - optind != 2) {
        fprintf(stderr, "Uso: %s -j NUM file1 file2\n", argv[0]);
        return 1;
    }

    return pcp(argv[optind], argv[optind + 1], n_proc);
}
