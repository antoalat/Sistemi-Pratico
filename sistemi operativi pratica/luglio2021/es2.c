/*Esercizio 2: Linguaggio C: 10 punti
Estendere l'esercizio 1. Il nuovo programma autolancia deve riconoscere se il primo parametro è una
libreria dinamica o un eseguibile gestendo entrambi i casi:
gcc -o hw hw.c
$ ./autolancia hw.so 1 2 3 4
hello world: hw.so 1 2 3 4
$ ./autolancia hw 1 2 3 4
hello world: hw.so 1 2 3 4*/



#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>

typedef int (*main_func_t)(int, char**);

int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strcmp(str + lenstr - lensuffix, suffix) == 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s path/to/hw.so|hw [args...]\n", argv[0]);
        return 1;
    }

    char *file_path = argv[1];

    if (ends_with(file_path, ".so")) {
        // Caso libreria dinamica: usiamo dlopen e dlsym come prima
        void *handle = dlopen(file_path, RTLD_NOW);
        if (!handle) {
            fprintf(stderr, "dlopen error: %s\n", dlerror());
            return 1;
        }

        dlerror(); // reset errori
        main_func_t hw_main = (main_func_t)dlsym(handle, "main");
        char *error = dlerror();
        if (error != NULL) {
            fprintf(stderr, "dlsym error: %s\n", error);
            dlclose(handle);
            return 1;
        }

        int hw_argc = argc - 1;
        char **hw_argv = malloc(sizeof(char*) * hw_argc);
        if (!hw_argv) {
            perror("malloc");
            dlclose(handle);
            return 1;
        }
        hw_argv[0] = file_path;
        for (int i = 2; i < argc; i++) {
            hw_argv[i-1] = argv[i];
        }

        int ret = hw_main(hw_argc, hw_argv);

        free(hw_argv);
        dlclose(handle);
        return ret;
    } else {
        // Caso eseguibile: usiamo fork + execvp
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            // Figlio: esegue il file
            // Costruiamo argv per execvp: argv[1..] di autolancia diventano argv[0..] di exec
            char **new_argv = malloc(sizeof(char*) * (argc));
            if (!new_argv) {
                perror("malloc");
                exit(1);
            }
            // argv[0] = nome eseguibile da lanciare
            new_argv[0] = file_path;
            for (int i = 2; i < argc; i++) {
                new_argv[i-1] = argv[i];
            }
            new_argv[argc - 1] = NULL;

            execvp(file_path, new_argv);
            // Se execvp fallisce:
            perror("execvp");
            free(new_argv);
            exit(1);
        } else {
            // Padre: aspetta la fine del figlio
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                return 1;
            }
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else {
                fprintf(stderr, "Process terminated abnormally\n");
                return 1;
            }
        }
    }
}
