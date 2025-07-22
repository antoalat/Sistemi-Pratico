#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/limits.h>

void run_name(const char *dir_path, const char *file_name, char **exec_args) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror(dir_path);
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
            perror(path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // Ricorsione
            run_name(path, file_name, exec_args);
        }
        else if (S_ISREG(st.st_mode) && strcmp(entry->d_name, file_name) == 0) {
            if (access(path, X_OK) == 0) {
                pid_t pid = fork();
                if (pid == -1) {
                    perror("fork");
                    continue;
                }

                if (pid == 0) {
                    // Processo figlio: cambia directory e esegui
                    if (chdir(dir_path) == -1) {
                        perror("chdir");
                        exit(EXIT_FAILURE);
                    }

                    // Stampa debug: dove sta eseguendo
                    printf("Eseguo: %s (cwd: %s)\n", path, dir_path);

                    execv(path, exec_args);
                    perror("execv fallita");
                    exit(EXIT_FAILURE);
                } else {
                    // Processo padre: aspetta il figlio
                    int status;
                    waitpid(pid, &status, 0);
                }
            }
        }
    }

    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <nome_file> [arg1 arg2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *target_name = argv[1];

    // Costruisci array argomenti per execv (incluso il nome del file)
    int arg_count = argc - 1;
    char **exec_args = malloc((arg_count + 1) * sizeof(char *)); // +1 per NULL

    if (!exec_args) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < arg_count; i++) {
        exec_args[i] = argv[i + 1];
    }
    exec_args[arg_count] = NULL;

    run_name(".", target_name, exec_args);

    free(exec_args);
    return 0;
}
