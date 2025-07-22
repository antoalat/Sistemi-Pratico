/*Esercizio 2: Linguaggio C: 10 punti Scrivere un programma che presi come parametri i pathname di
un file f e di una directory d stampi l'elenco dei link simbolici che puntano a f presenti nel sottoalbero
del file system generato dalla directory d.*/






#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/limits.h>

void cerca_link_simbolici(const char *dir_path, const char *target_realpath) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Errore apertura directory");
        return;
    }

    struct dirent *entry;
    char path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        // Salta "." e ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (lstat(path, &st) == -1) {
            perror("Errore lstat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // Ricorsione nelle sottodirectory
            cerca_link_simbolici(path, target_realpath);
        } else if (S_ISLNK(st.st_mode)) {
            // È un link simbolico
            char resolved[PATH_MAX];
            if (realpath(path, resolved) == NULL) {
                // Link rotto, ignoralo
                continue;
            }

            // Se punta al file target
            if (strcmp(resolved, target_realpath) == 0) {
                // Leggiamo il contenuto letterale del link con readlink()
                char link_content[PATH_MAX];
                ssize_t len = readlink(path, link_content, sizeof(link_content) - 1);
                if (len == -1) {
                    perror("Errore readlink");
                    continue;
                }
                link_content[len] = '\0'; // Terminare manualmente

                printf("%s -> %s\n", path, link_content);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <file_target> <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char target_realpath[PATH_MAX];
    if (realpath(argv[1], target_realpath) == NULL) {
        perror("Errore realpath sul file target");
        return EXIT_FAILURE;
    }

    cerca_link_simbolici(argv[2], target_realpath);
    return EXIT_SUCCESS;
}
