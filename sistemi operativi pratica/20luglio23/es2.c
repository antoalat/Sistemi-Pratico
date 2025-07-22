
/*Esercizio 2: Linguaggio C: 10 punti
Il programma cprlt ha 3 paramentri: un tempo in secondi da epoch, una directory sorgente e una di
destinazione. es:
cprl 1689577839 a b
Il programma si deve comportare come cprl dell'esercizio 1 con la differenza che i file con tempo di
ultima modifica precedente al tempo indicato nel parametro vengono collegati con link fisici gli altri
devono essere copiati.*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 4096

int cprl_time(time_t time, char *patha, char *pathb) {
    DIR *dir = opendir(patha);
    if (!dir) {
        perror("Errore apertura dir");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];
    char new_path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", patha, entry->d_name);
        snprintf(new_path, sizeof(new_path), "%s/%s", pathb, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == -1) {
            perror("Errore stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (mkdir(new_path, 0777) == -1) {
                perror("Errore creazione dir");
                continue;
            }
            cprl_time(time, full_path, new_path); // ricorsione
        } else if (S_ISREG(st.st_mode)) {
            time_t time_of_file = st.st_mtime;
            if (time_of_file < time) {
                if (link(full_path, new_path) == -1) {
                    perror("Errore link fisico");
                    continue;
                }
            } else {
                int source_fd = open(full_path, O_RDONLY);
                if (source_fd == -1) {
                    perror("Errore apertura file origine");
                    continue;
                }

                int dest_fd = open(new_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (dest_fd == -1) {
                    perror("Errore apertura/creazione file destinazione");
                    close(source_fd);
                    continue;
                }

                ssize_t bytes_read, bytes_written;
                char buffer[BUFFER_SIZE];

                while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
                    bytes_written = write(dest_fd, buffer, bytes_read);
                    if (bytes_written != bytes_read) {
                        perror("Errore scrittura file");
                        break;
                    }
                }

                if (bytes_read == -1) {
                    perror("Errore lettura file");
                }

                close(source_fd);
                close(dest_fd);

                printf("File '%s' copiato in '%s'.\n", full_path, new_path);
            }
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <time> <directorya> <directoryb>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Assicurati che la directory di destinazione esista
    struct stat st;
    if (stat(argv[3], &st) == -1) {
        if (mkdir(argv[3], 0777) == -1) {
            perror("Errore creazione dir destinazione");
            return EXIT_FAILURE;
        }
    }

    time_t time = atoi(argv[1]);
    return cprl_time(time, argv[2], argv[3]);
}
