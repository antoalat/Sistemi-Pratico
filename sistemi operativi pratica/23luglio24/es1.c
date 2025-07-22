#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

// Controlla se la directory esiste già
bool isAlreadyExist(const char *nomeCartella) {
    struct stat buffer;
    if (stat(nomeCartella, &buffer) == 0) {
        return S_ISDIR(buffer.st_mode);
    } else {
        if (errno == ENOENT) {
            return false;
        } else {
            perror("Errore durante la chiamata a stat");
            return false;
        }
    }
}

int create_and_move(char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("Errore apertura directory");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];
    char target_path[PATH_MAX];
    snprintf(target_path, sizeof(target_path), "%s/...", path);

    if (!isAlreadyExist(target_path)) {
        if (mkdir(target_path, 0755) == -1) {
            perror("Errore creazione directory");
            closedir(dir);
            return EXIT_FAILURE;
        }
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignora . .. e ...
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0 ||
            strcmp(entry->d_name, "...") == 0) {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == -1) {
            perror("Errore stats file");
            continue;
        }

        if (S_ISREG(st.st_mode)) {
            // Sposta il file in .../
            char new_path[PATH_MAX];
            snprintf(new_path, sizeof(new_path), "%s/%s", target_path, entry->d_name);

            if (rename(full_path, new_path) == -1) {
                perror("Errore spostamento file");
                continue;
            }

            // Crea symlink relativo ../.../nomefile
            char relative_link[PATH_MAX];
            snprintf(relative_link, sizeof(relative_link), "../.../%s", entry->d_name);

            // Crea link simbolico temporaneo
            char tmp_link[PATH_MAX];
            snprintf(tmp_link, sizeof(tmp_link), "%s/%s.tmp", path, entry->d_name);

            if (symlink(relative_link, tmp_link) == -1) {
                perror("Errore creazione symlink");
                continue;
            }

            // Rinomina il link simbolico al nome originale (atomico)
            if (rename(tmp_link, full_path) == -1) {
                perror("Errore rinomina symlink");
                unlink(tmp_link);
                continue;
            }
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    if (argc != 1) {
        fprintf(stderr, "Uso: %s (senza argomenti)\n", argv[0]);
        return EXIT_FAILURE;
    }

    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("Errore current working directory");
        return EXIT_FAILURE;
    }

    return create_and_move(cwd);
}
