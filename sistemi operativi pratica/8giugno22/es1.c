#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/limits.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <symlink>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *link_path = argv[1];
    struct stat st;

    // Verifica che il file passato sia un link simbolico
    if (lstat(link_path, &st) == -1) {
        perror("Errore lstat");
        return EXIT_FAILURE;
    }

    if (!S_ISLNK(st.st_mode)) {
        fprintf(stderr, "Errore: %s non è un link simbolico.\n", link_path);
        return EXIT_FAILURE;
    }

    // Leggi il path del file puntato dal link simbolico
    char target_path[PATH_MAX];
    ssize_t len = readlink(link_path, target_path, sizeof(target_path) - 1);
    if (len == -1) {
        perror("Errore readlink");
        return EXIT_FAILURE;
    }
    target_path[len] = '\0';

    // Ottieni il path assoluto del file vero (A)
    char real_target_path[PATH_MAX];
    if (realpath(target_path, real_target_path) == NULL) {
        perror("Errore realpath");
        return EXIT_FAILURE;
    }

    // Rimuovi il link simbolico B
    if (unlink(link_path) == -1) {
        perror("Errore unlink");
        return EXIT_FAILURE;
    }

    // Sposta (rename) A (il vero file) al posto di B
    if (rename(real_target_path, link_path) == -1) {
        perror("Errore rename");
        return EXIT_FAILURE;
    }

    // Crea un link simbolico da A → B
    if (symlink(link_path, real_target_path) == -1) {
        perror("Errore symlink");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
