#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>
#include <errno.h>

int undo_symlinks(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        perror("Errore apertura directory");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];
    char real_target[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        // Ignora . e ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if (lstat(full_path, &st) == -1) {
            perror("Errore lstat");
            continue;
        }

        // Se è un link simbolico
        if (S_ISLNK(st.st_mode)) {
            ssize_t len = readlink(full_path, real_target, sizeof(real_target) - 1);
            if (len == -1) {
                perror("Errore readlink");
                continue;
            }
            real_target[len] = '\0';

            // Verifica che punti a ./.../nomefile o .../nomefile
            if (strncmp(real_target, ".../", 4) == 0 || strncmp(real_target, "./.../", 6) == 0) {
                // Path completo al file vero
                char real_file_path[PATH_MAX];
                snprintf(real_file_path, sizeof(real_file_path), "%s/%s", dirpath, real_target);

                // Path temporaneo dove mettere il file prima della sostituzione
                char tmp_path[PATH_MAX];
                snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", full_path);

                // Rinomina il file originale da .../file a file.tmp
                if (rename(real_file_path, tmp_path) == -1) {
                    perror("Errore rename su file reale -> tmp");
                    continue;
                }

                // Rimuovi il symlink (opzionale, rename lo sovrascrive comunque)
                if (unlink(full_path) == -1) {
                    perror("Errore unlink symlink");
                    // Puoi anche decidere di proseguire comunque
                    continue;
                }

                // Rinomina file.tmp → file (cioè rimpiazza il link con il file vero)
                if (rename(tmp_path, full_path) == -1) {
                    perror("Errore rename tmp -> originale");
                    continue;
                }
            }
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

int main(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Errore getcwd");
        return EXIT_FAILURE;
    }

    return undo_symlinks(cwd);
}
