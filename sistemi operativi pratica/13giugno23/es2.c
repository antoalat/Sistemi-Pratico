#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/timerfd.h>
#include <sys/poll.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    int tfd;
    int remaining;
    char *message;
} TimerInfo;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <n,interval,string> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n_timers = argc - 1;
    TimerInfo *timers = calloc(n_timers, sizeof(TimerInfo));
    struct pollfd *pfds = calloc(n_timers, sizeof(struct pollfd));

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n_timers; i++) {
        // Copia e parsa l'argomento
        char *arg = strdup(argv[i + 1]);
        char *tok1 = strtok(arg, ",");
        char *tok2 = strtok(NULL, ",");
        char *tok3 = strtok(NULL, ",");

        if (!tok1 || !tok2 || !tok3) {
            fprintf(stderr, "Argomento non valido: %s\n", argv[i + 1]);
            free(arg);
            continue;
        }

        int count = atoi(tok1);
        double interval = strtod(tok2, NULL);
        char *message = strdup(tok3);

        int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (tfd == -1) {
            perror("timerfd_create");
            free(arg);
            continue;
        }

        struct itimerspec ts = {
            .it_value = {
                .tv_sec = (time_t)interval,
                .tv_nsec = (interval - (time_t)interval) * 1e9
            },
            .it_interval = {
                .tv_sec = (time_t)interval,
                .tv_nsec = (interval - (time_t)interval) * 1e9
            }
        };

        if (timerfd_settime(tfd, 0, &ts, NULL) == -1) {
            perror("timerfd_settime");
            close(tfd);
            free(arg);
            continue;
        }

        timers[i].tfd = tfd;
        timers[i].remaining = count;
        timers[i].message = message;

        pfds[i].fd = tfd;
        pfds[i].events = POLLIN;

        free(arg);
    }

    int active = n_timers;

    while (active > 0) {
        int ready = poll(pfds, n_timers, -1);
        if (ready == -1) {
            perror("poll");
            break;
        }

        for (int i = 0; i < n_timers; i++) {
            if ((pfds[i].revents & POLLIN) && timers[i].remaining > 0) {
                uint64_t expirations;
                read(pfds[i].fd, &expirations, sizeof(expirations));

                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                double elapsed = (now.tv_sec - start.tv_sec) +
                                 (now.tv_nsec - start.tv_nsec) / 1e9;

                printf("%.6f %s\n", elapsed, timers[i].message);
                fflush(stdout);

                timers[i].remaining--;
                if (timers[i].remaining == 0) {
                    close(timers[i].tfd);
                    pfds[i].fd = -1; // disabilita
                    active--;
                }
            }
        }
    }

    // Cleanup
    for (int i = 0; i < n_timers; i++) {
        free(timers[i].message);
    }

    free(timers);
    free(pfds);
    return EXIT_SUCCESS;
}
