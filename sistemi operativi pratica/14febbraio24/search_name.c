#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <errno.h>

void search(const char *dir_path, const char *target_name);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_file>\n", argv[0]);
        return 1;
    }

    const char *target_name = argv[1]; //Salvo il target_name
    search(".", target_name);
    return 0;
}

void search(const char *dir_path, const char *target_name) {
    DIR *dir = opendir(dir_path); //apro la directory
    if (!dir) {
        perror(dir_path);
        return;
    }

    struct dirent *entry; //apro la directory entry
    char path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        // Ignora "." e ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name); //creo path totale

        struct stat st;
        if (lstat(path, &st) == -1) {
            perror(path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // Ricorsione su sottodirectory
            search(path, target_name);
        } else if (S_ISREG(st.st_mode) && strcmp(entry->d_name, target_name) == 0) {
            // File regolare e nome combacia
            if (access(path, X_OK) == 0) {
                // È eseguibile
                FILE *f = fopen(path, "rb");
                if (!f) {
                    perror(path);
                    continue;
                }

                unsigned char buf[4];
                size_t n = fread(buf, 1, 4, f);
                fclose(f);

                if (n >= 2 && buf[0] == '#' && buf[1] == '!') {
                    printf("%s: script\n", path);
                } else if (n == 4 && buf[0] == 0x7f && buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F') {
                    printf("%s: ELF executable\n", path);
                } else {
                    printf("%s: altro tipo eseguibile\n", path);
                }
            }
        }
    }

    closedir(dir);
}
