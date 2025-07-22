/* Esercizio 1: Linguaggio C (obbligatorio) 20 punti
Scrivere un programma timeout che esegua un programma e lo termini se supera una durata
massima prefissata. timeout ha almeno due argomenti: il primo è la durata massima in millisecondi, i
parametri dal secondo in poi sono il programma da lanciare coi rispettivi argomenti.
Es:
timeout 5000 sleep 2
temina in due secondi (sleep termina in tempo).
timeout 3000 sleep 5
passati tre secondi il programma sleep viene terminato.
Tmeout deve essere scritto usando le system call poll, pidfd_open, timerfd*.*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <sys/pidfd.h>   // <-- qui usi direttamente pidfd_open()
#include <signal.h>
#include <stdint.h>      // per uint64_t

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <timeout_ms> <comando> [arg1 arg2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int timeout_ms = atoi(argv[1]);
    if (timeout_ms <= 0) {
        fprintf(stderr, "Errore: timeout non valido (%s)\n", argv[1]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // --- FIGLIO ---
        execvp(argv[2], &argv[2]);
        perror("execvp fallita");
        exit(EXIT_FAILURE);
    }

    // --- PADRE ---
    int pidfd = pidfd_open(pid, 0);
    if (pidfd == -1) {
        perror("pidfd_open");
        kill(pid, SIGKILL);
        return EXIT_FAILURE;
    }

    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1) {
        perror("timerfd_create");
        kill(pid, SIGKILL);
        return EXIT_FAILURE;
    }

    struct itimerspec ts = {0};
    ts.it_value.tv_sec = timeout_ms / 1000;
    ts.it_value.tv_nsec = (timeout_ms % 1000) * 1000000;

    if (timerfd_settime(tfd, 0, &ts, NULL) == -1) { //Attesa relativa (parametro 0 a partire da ora, si bassa sulla struttura ts,NULL puntatore a struttura per il vecchio valore   
        perror("timerfd_settime");
        kill(pid, SIGKILL);
        return EXIT_FAILURE;
    }

    struct pollfd fds[2];
    fds[0].fd = pidfd;
    fds[0].events = POLLIN;
    fds[1].fd = tfd;
    fds[1].events = POLLIN; 

    int poll_result = poll(fds, 2, -1); //Attende sui fd  (-1 attende indefinitivamente finchè uno dei due fd è pronto)
    if (poll_result == -1) {
        perror("poll");
        kill(pid, SIGKILL);
        return EXIT_FAILURE;
    }

    if (fds[0].revents & POLLIN) {
        // figlio ha terminato in tempo
        waitpid(pid, NULL, 0);
        printf("Processo terminato normalmente.\n");
    } else if (fds[1].revents & POLLIN) {
        // tempo scaduto
        printf("Timeout scaduto. Uccido il processo.\n");
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    }

    close(pidfd);
    close(tfd);
    return EXIT_SUCCESS;
}



//Versione alternativa


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/pidfd.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <timeout_ms> <comando> [arg1 arg2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int timeout_ms = atoi(argv[1]);
    if (timeout_ms <= 0) {
        fprintf(stderr, "Timeout non valido: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Figlio: esegue il comando
        execvp(argv[2], &argv[2]);
        perror("execvp fallita");
        exit(EXIT_FAILURE);
    }

    // Padre: controlla il figlio con pidfd e poll
    int pidfd = pidfd_open(pid, 0);
    if (pidfd == -1) {
        perror("pidfd_open");
        kill(pid, SIGKILL);
        return EXIT_FAILURE;
    }

    struct pollfd pfd = {
        .fd = pidfd,
        .events = POLLIN,
    };

    int poll_res = poll(&pfd, 1, timeout_ms);
    if (poll_res == -1) {
        perror("poll");
        kill(pid, SIGKILL);
        close(pidfd);
        return EXIT_FAILURE;
    } else if (poll_res == 0) {
        // Timeout scaduto, uccido il figlio
        printf("Timeout scaduto, uccido il processo figlio\n");
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
    } else {
        // Il figlio è terminato prima del timeout
        if (pfd.revents & POLLIN) {
            waitpid(pid, NULL, 0);
            printf("Processo terminato normalmente.\n");
        }
    }

    close(pidfd);
    return EXIT_SUCCESS;
}

