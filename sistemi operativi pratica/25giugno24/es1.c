#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1)) //Spazio per almeno 10 eventi alla volta + NOme + 1 fine stringa.

void execute_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("fopen");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Leggi la prima riga: eseguibile
    if ((read = getline(&line, &len, file)) == -1) {
        fprintf(stderr, "Errore: file vuoto o non leggibile: %s\n", filepath);
        fclose(file);
        return;
    }
    if (line[read - 1] == '\n') line[read - 1] = '\0';
    char *exec_path = strdup(line);

    // Leggi i parametri riga per riga
    char *args[256];
    int argc = 0;
    while ((read = getline(&line, &len, file)) != -1 && argc < 255) { //getline() legge una riga intera dal file (alloca/realloca automaticamente la memoria).
        if (line[read - 1] == '\n') line[read - 1] = '\0';
        args[argc++] = strdup(line);
    }
    args[argc] = NULL; //Fine array di args

    fclose(file); //chiudi  il file
    free(line); //Libera il buffer

    pid_t pid = fork(); //Crea un child
    if (pid == 0) { //Siamo nel figlio
        execv(exec_path, args);
        perror("execv");
        _exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        unlink(filepath);  // elimina il file dopo l'esecuzione
    } else {
        perror("fork");
    }

    free(exec_path);//Libero la memoria
    for (int i = 0; i < argc; ++i) { //Libero la memoria
        free(args[i]); //Libero
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directory_da_monitorare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *dir = argv[1];

    int fd = inotify_init1(IN_CLOEXEC); //Crea il descrittore inotify -> IN_CLOEXEC chiude automaticamente il descrittore se si fa exec
    if (fd == -1) {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(fd, dir, IN_CLOSE_WRITE); //Monitora la directory dir per l'evento IN_CLOSE_WRITE (scrittura in chiusura file)
    if (wd == -1) {
        perror("inotify_add_watch");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char buf[BUF_LEN];

    while (1) {
        ssize_t len = read(fd, buf, sizeof(buf)); //Legge dal filedescriptor e scrive sul buf gli eventi
        if (len <= 0) {
            perror("read");
            break;
        }

        ssize_t i = 0;
        while (i < len) {
            struct inotify_event *event = (struct inotify_event *)&buf[i]; //Estrae la struct dedicata all'evento
            if (event->len && (event->mask & IN_CLOSE_WRITE)) { //SE esiste un nome associato, controlla l'eventpo se è scrittura in chiusura
                char fullpath[PATH_MAX]; //dichiara il fullpath e lo passa alla funzione che legge da quel file e lo esegue
                snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, event->name);
                execute_file(fullpath);
            }
            i += sizeof(struct inotify_event) + event->len; //incrementa l'offset di i spostandosi al blocco dopo 
        }
    }

    inotify_rm_watch(fd, wd); //Rimuove il watching sul descriptor fd
    close(fd); //Chiude il file descriptor
    return 0;
}
