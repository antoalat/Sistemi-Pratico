#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>  // Per monitorare eventi nel filesystem (inotify)
#include <sys/types.h>
#include <sys/wait.h>     // Per waitpid
#include <errno.h>

#define EVENT_SIZE (sizeof(struct inotify_event))            // Dimensione base di un evento inotify
#define BUF_LEN (1024 * (EVENT_SIZE + 16))                   // Dimensione buffer per leggere eventi in blocco

// Funzione che legge un file di testo contenente comandi,
// esegue i comandi uno per uno, e cancella il file al termine
void esegui_comandi_da_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");  // Apro il file in sola lettura
    if (!file) {
        perror("fopen");
        return;
    }

    char *line = NULL;       // Puntatore per la riga letta da getline
    size_t linecap = 0;      // Capacità allocata per getline
    ssize_t linelen;         // Lunghezza della riga letta

    size_t argv_size = 8;    // Capacità iniziale per l'array argv (argomenti comando)
    size_t argc = 0;         // Numero di argomenti attualmente letti
    char **argv = malloc(argv_size * sizeof(char *));
    if (!argv) {
        perror("malloc");
        fclose(file);
        return;
    }

    // Leggo il file riga per riga finché non finisce
    while ((linelen = getline(&line, &linecap, file)) != -1) {
        // Rimuovo il carattere newline finale se presente
        if (linelen > 0 && line[linelen - 1] == '\n') {
            line[linelen - 1] = '\0';
            linelen--;
        }

        // Riga vuota indica fine comando, quindi eseguo il comando
        if (linelen == 0) {
            if (argc > 0) {
                argv[argc] = NULL;  // execvp richiede argv terminato da NULL

                pid_t pid = fork();  // Creo un processo figlio per eseguire il comando
                if (pid == 0) {
                    execvp(argv[0], argv);  // Eseguo il comando
                    perror("execvp");      // Se fallisce, stampo errore
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    waitpid(pid, NULL, 0); // Padre aspetta la fine del figlio
                } else {
                    perror("fork");        // Errore nel fork
                }

                // Libero la memoria degli argomenti letti
                for (size_t i = 0; i < argc; i++) {
                    free(argv[i]);
                }
                argc = 0;  // Resetto il contatore degli argomenti per il prossimo comando
            }
            continue;  // Passo alla prossima riga
        }

        // Se l'array argv è pieno, ne aumento la dimensione (raddoppio)
        if (argc + 1 >= argv_size) { // uno spazio in più per il terminatore NULL
            argv_size *= 2;
            char **tmp = realloc(argv, argv_size * sizeof(char *));
            if (!tmp) {
                perror("realloc");
                // Libero prima di uscire
                for (size_t i = 0; i < argc; i++) {
                    free(argv[i]);
                }
                free(argv);
                free(line);
                fclose(file);
                return;
            }
            argv = tmp;
        }

        // Copio la linea letta nell'array argv
        argv[argc] = strdup(line);
        if (!argv[argc]) {
            perror("strdup");
            // Libero prima di uscire
            for (size_t i = 0; i < argc; i++) {
                free(argv[i]);
            }
            free(argv);
            free(line);
            fclose(file);
            return;
        }
        argc++;  // Incremento il numero di argomenti
    }

    // Se il file termina senza una riga vuota finale,
    // devo comunque eseguire l'ultimo comando letto
    if (argc > 0) {
        argv[argc] = NULL;

        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
        } else {
            perror("fork");
        }

        for (size_t i = 0; i < argc; i++) {
            free(argv[i]);
        }
    }

    // Libero memoria allocata e chiudo il file
    free(argv);
    free(line);
    fclose(file);

    // Rimuovo il file, perché è stato processato
    if (remove(filepath) != 0) {
        perror("remove");
    }
}

int main(int argc, char *argv[]) {
    // Verifico che il programma abbia ricevuto esattamente 1 argomento (directory)
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dir = argv[1];

    // Inizializzo inotify per monitorare eventi nel filesystem
    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return EXIT_FAILURE;
    }

    // Aggiungo un "watch" sulla directory passata,
    // interessandomi agli eventi di file chiusi in scrittura o spostati dentro la directory
    int wd = inotify_add_watch(fd, dir, IN_CLOSE_WRITE | IN_MOVED_TO);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        return EXIT_FAILURE;
    }

    char buffer[BUF_LEN];  // Buffer per leggere eventi in blocco

    // Ciclo infinito per attendere e processare eventi inotify
    while (1) {
        ssize_t length = read(fd, buffer, sizeof(buffer));
        if (length < 0) {
            perror("read");
            break;  // Esco dal ciclo in caso di errore di lettura
        }

        size_t offset = 0;
        // Scorro tutti gli eventi ricevuti nel buffer
        while (offset < (size_t)length) {
            struct inotify_event *event = (struct inotify_event *)(buffer + offset);

            // Se l'evento ha un nome di file e l'evento è tra quelli che ci interessano
            if (event->len > 0 &&
                (event->mask & (IN_CLOSE_WRITE | IN_MOVED_TO))) {

                char filepath[4096];
                int ret = snprintf(filepath, sizeof(filepath), "%s/%s", dir, event->name);
                if (ret < 0 || ret >= (int)sizeof(filepath)) {
                    fprintf(stderr, "Nome file troppo lungo: %s/%s\n", dir, event->name);
                } else {
                    // Eseguo i comandi nel file appena creato o modificato
                    esegui_comandi_da_file(filepath);
                }
            }

            // Passo al prossimo evento nel buffer
            offset += EVENT_SIZE + event->len;
        }
    }

    // Pulizia: rimuovo il watch e chiudo il file descriptor inotify
    inotify_rm_watch(fd, wd);
    close(fd);

    return EXIT_SUCCESS;
}
