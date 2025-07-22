/*
Esercizio 1: Linguaggio C (obbligatorio) – 20 punti
Scrivere un programma di copia parallelo di file.

Uso: pcp file1 file2

Il programma crea due processi figli:
- Il primo copia la prima metà di file1 in file2.
- Il secondo copia la seconda metà.

Il padre attende entrambi i figli con waitpid().
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BUF_SIZE 1024

// Copia la porzione [start, end) del file in_fd nel file out_fd
void copy_range(int in_fd, int out_fd, off_t start, off_t end) {
    char buffer[BUF_SIZE];

    // Posizionamento dei puntatori all'inizio dell'intervallo da copiare
    lseek(in_fd, start, SEEK_SET);
    lseek(out_fd, start, SEEK_SET);

    off_t remaining = end - start;

    // Leggi e scrivi blocchi di BUF_SIZE finché c'è da copiare
    while (remaining > 0) {
        ssize_t to_read = remaining < BUF_SIZE ? remaining : BUF_SIZE;
        ssize_t read_bytes = read(in_fd, buffer, to_read);
        if (read_bytes <= 0) break;
        write(out_fd, buffer, read_bytes);
        remaining -= read_bytes;
    }
}

// Funzione principale di copia parallela
int pcp(char *file1, char *file2) {
    // Apertura file di origine in lettura
    int in_fd = open(file1, O_RDONLY);
    if (in_fd < 0) {
        perror("Errore apertura file1");
        return 1;
    }

    // Stat per conoscere dimensione del file
    struct stat st;
    if (stat(file1, &st) == -1) {
        perror("Errore stat");
        close(in_fd);
        return 1;
    }

    off_t size = st.st_size;

    // Apertura file di destinazione in scrittura (crea/tronca)
    int out_fd = open(file2, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("Errore apertura file2");
        close(in_fd);
        return 1;
    }

    // Prealloca spazio nel file di destinazione
    if (ftruncate(out_fd, size) == -1) {
        perror("Errore ftruncate");
        close(in_fd);
        close(out_fd);
        return 1;
    }

    // Primo fork: primo figlio copia la prima metà
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Errore fork primo figlio");
        close(in_fd);
        close(out_fd);
        return 1;
    }
    if (pid1 == 0) {
        copy_range(in_fd, out_fd, 0, size / 2);
        close(in_fd);
        close(out_fd);
        exit(0);  // figlio termina
    }

    // Secondo fork: secondo figlio copia la seconda metà
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Errore fork secondo figlio");
        // attende il primo figlio prima di uscire
        waitpid(pid1, NULL, 0);
        close(in_fd);
        close(out_fd);
        return 1;
    }
    if (pid2 == 0) {
        copy_range(in_fd, out_fd, size / 2, size);
        close(in_fd);
        close(out_fd);
        exit(0);  // figlio termina
    }

    // Padre: aspetta entrambi i figli con waitpid()
    int status;
    if (waitpid(pid1, &status, 0) == -1) {
        perror("Errore in waitpid per primo figlio");
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Primo figlio terminato con errore %d\n", WEXITSTATUS(status));
    }

    if (waitpid(pid2, &status, 0) == -1) {
        perror("Errore in waitpid per secondo figlio");
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Secondo figlio terminato con errore %d\n", WEXITSTATUS(status));
    }

    // Chiusura file
    close(in_fd);
    close(out_fd);

    return 0;
}

// Main: parsing degli argomenti e chiamata a pcp()
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <file1> <file2>\n", argv[0]);
        return 1;
    }

    return pcp(argv[1], argv[2]);
}
