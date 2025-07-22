/*Scrivere un programma cprl che si comporti come il comando "cp -rl". cprl ha due parametri:
cprl a b
deve copiare l'intera struttura delle directory dell'albero che ha come radice a in un secondo albero
con radice b. I file non devono essere copiati ma collegati con link fisici.
(l'operazione deve essere fatta dal codice C, senza lanciare altri programmi/comandi)*/





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>

int cprl(char *patha, char *pathb)
{
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
            cprl(full_path, new_path); // ricorsione
        } else if (S_ISREG(st.st_mode)) {
            if (link(full_path, new_path) == -1) {
                perror("Errore link fisico");
                continue;
            }
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <directorya> <directoryb>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Assicurati che la directory di destinazione esista
    struct stat st;
    if (stat(argv[2], &st) == -1) {
        if (mkdir(argv[2], 0777) == -1) {
            perror("Errore creazione dir destinazione");
            return EXIT_FAILURE;
        }
    }

    return cprl(argv[1], argv[2]);
}
