/*Esercizio 1: Linguaggio C (obbligatorio) 20 punti
Sia dato questo programma hw.c (vi viene fornito in /public/hw.c)
#include <stdio.h>
int main(int argc, char*argv[]) {
 printf("hello world:");
 for(argv++, argv--; argc > 0; argv++, argc--)
 printf(" %s",*argv);
 printf("\n");
 return 0;
}
Il programma hw.c può essere compilato come libreria dinamica:
gcc --shared -o hw.so hw.c
La libreria dinamica non è un eseguibile
$ ./hw.so 1 2 3 4
Segmentation fault
ma può essere caricata a tempo di esecuzione tramite dlopen. Scrivere un programma "lancia" in
grado di eseguire il codice di hw.so
$ ./lancia hw.so 1 2 3 4
hello world: hw.so 1 2 3 4
(suggerimenti: dlopen non cerca nella directory corrente, occorre passare il path assoluto della libreria.
"main" in hw.so è una normale funzione: occorre cercare l'indirizzo della funzione main nella libreria
ed invocarla,)*/



#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

typedef int (*main_func_t)(int, char**); //Definisco funzione che ritorna int e prende un int e un array di stringhe

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s path/to/hw.so [args...]\n", argv[0]);
        return 1;
    }

    char *lib_path = argv[1];

    // Apriamo la libreria condivisa con dlopen
    void *handle = dlopen(lib_path, RTLD_NOW); //Risolvo link e riferimenti subito
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return 1;
    }

    // Cerchiamo la funzione main nella libreria
    dlerror(); // pulisce buffer errori
    main_func_t hw_main = (main_func_t)dlsym(handle, "main");
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "dlsym error: %s\n", error);
        dlclose(handle);
        return 1;
    }

    // Prepariamo argc e argv da passare alla funzione main in hw.so
    // argc e argv per hw.so devono essere:
    // argv[0] = nome libreria, argv[1] = arg1, argv[2] = arg2, ...
    int hw_argc = argc - 1; // escludiamo "lancia"
    char **hw_argv = malloc(sizeof(char*) * hw_argc);
    if (!hw_argv) {
        perror("malloc");
        dlclose(handle);
        return 1;
    }
    // argv[1] è il path della libreria, quindi lo mettiamo come argv[0] per hw.so
    hw_argv[0] = lib_path;
    // copiamo il resto degli argomenti da argv[2...]
    for (int i = 2; i < argc; i++) {
        hw_argv[i-1] = argv[i];
    }

    // Chiamiamo la funzione main dentro hw.so
    int ret = hw_main(hw_argc, hw_argv);

    free(hw_argv);
    dlclose(handle);

    return ret;
}
