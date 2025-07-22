#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <ctype.h>

bool is_in_cmdline(const char *pid, const char *cmdline_pattern, size_t pattern_len)
{
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%s/cmdline", pid);

    int fd = open(path, O_RDONLY);
    if (fd == -1) return false;

    char buf[8192];
    ssize_t len = read(fd, buf, sizeof(buf));   
    close(fd);

    if (len <= 0) return false;

    return (len == (ssize_t)pattern_len) && (memcmp(buf, cmdline_pattern, pattern_len) == 0);
}

int find_proc(const char *pattern, size_t pattern_len)
{
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("Errore apertura /proc");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!isdigit(entry->d_name[0]))
            continue;

        struct stat st;
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/proc/%s", entry->d_name);
        if (stat(path, &st) == -1 || !S_ISDIR(st.st_mode))
            continue;

        if (is_in_cmdline(entry->d_name, pattern, pattern_len)) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s comando [arg1 arg2 ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Calcola la lunghezza totale degli argomenti separati da \0
    size_t total_len = 0;
    for (int i = 1; i < argc; i++) {
        total_len += strlen(argv[i]) + 1;
    }

    char *pattern = malloc(total_len);
    if (!pattern) {
        perror("malloc fallita");
        return EXIT_FAILURE;
    }

    // Costruisce la stringa da confrontare: "arg1\0arg2\0arg3\0..."
    size_t offset = 0;
    for (int i = 1; i < argc; i++) {
        size_t len = strlen(argv[i]);
        memcpy(pattern + offset, argv[i], len);
        offset += len;
        pattern[offset++] = '\0';
    }

    int res = find_proc(pattern, total_len);
    free(pattern);
    return res;
}
