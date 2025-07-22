/*Esercizio 2: Linguaggio C: 10 punti
Estendere l'esercizio 1 per fare in modo che se prima del timeout il programma termina con un errore,
al termine del timeout il programma venga riattivato.*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <sys/pidfd.h>
#include <signal.h>
#include <stdint.h>

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

    while (1) {
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
            close(pidfd);
            return EXIT_FAILURE;
        }

        struct itimerspec ts = {0};
        ts.it_value.tv_sec = timeout_ms / 1000;
        ts.it_value.tv_nsec = (timeout_ms % 1000) * 1000000;

        if (timerfd_settime(tfd, 0, &ts, NULL) == -1) {
            perror("timerfd_settime");
            kill(pid, SIGKILL);
            close(pidfd);
            close(tfd);
            return EXIT_FAILURE;
        }

        struct pollfd fds[2];
        fds[0].fd = pidfd;
        fds[0].events = POLLIN;
        fds[1].fd = tfd;
        fds[1].events = POLLIN;

        int poll_result = poll(fds, 2, -1);
        if (poll_result == -1) {
            perror("poll");
            kill(pid, SIGKILL);
            close(pidfd);
            close(tfd);
            return EXIT_FAILURE;
        }

        if (fds[0].revents & POLLIN) {
            // Il figlio è terminato, controlliamo come
            int status;
            waitpid(pid, &status, 0);

            close(pidfd);
            close(tfd);

            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if (exit_code == 0) {
                    printf("Processo terminato normalmente.\n");
                    break;  // Esci dal while: programma finito con successo
                } else {
                    printf("Processo terminato con errore (codice %d), rilancio...\n", exit_code);
                    // Riavvio il ciclo per rilanciare il processo
                    continue;
                }
            } else if (WIFSIGNALED(status)) {
                int sig = WTERMSIG(status);
                printf("Processo terminato da segnale %d\n", sig);
                break;
            } else {
                printf("Processo terminato in modo anomalo\n");
                break;
            }
        } else if (fds[1].revents & POLLIN) {
            // Timeout scaduto
            printf("Timeout scaduto. Uccido il processo.\n");
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            close(pidfd);
            close(tfd);
            break;  // Esci dal while
        }
    }

    return EXIT_SUCCESS;
}
