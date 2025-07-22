#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

/**
 * Verifica se un file è regolare (non directory, link, ecc.).
 */
int is_regular_file(const char *dir, const char *name) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);
    struct stat sb;
    if (stat(fullpath, &sb) != 0)
        return 0;

    return S_ISREG(sb.st_mode);
}

/**
 * Legge i nomi dei file regolari da una directory.
 */
char **vreaddir(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) { 
        perror("opendir");
        return NULL;
    }

    int capacity = 10;
    int count = 0;
    char **list = malloc(capacity * sizeof(char *));
    if (!list) {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Salta "." e ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Salta se non è un file regolare
        if (!is_regular_file(path, entry->d_name))
            continue;

        // Realloc se serve più spazio
        if (count >= capacity) {
            capacity *= 2;
            char **new_list = realloc(list, capacity * sizeof(char *));
            if (!new_list) {
                perror("realloc");
                for (int i = 0; i < count; i++)
                    free(list[i]);
                free(list);
                closedir(dir);
                return NULL;
            }
            list = new_list;
        }

        // Copia il nome del file
        list[count] = strdup(entry->d_name);
        if (!list[count]) {
            perror("strdup");
            for (int i = 0; i < count; i++)
                free(list[i]);
            free(list);
            closedir(dir);
            return NULL;
        }

        count++;
    }

    list[count] = NULL;  // Terminatore
    closedir(dir);
    return list;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char **files = vreaddir(argv[1]);
    if (!files) {
        fprintf(stderr, "Errore nella lettura della directory.\n");
        return EXIT_FAILURE;
    }

    printf("File regolari trovati in %s:\n", argv[1]);
    for (int i = 0; files[i] != NULL; i++) {
        printf(" - %s\n", files[i]);
        free(files[i]);
    }
    free(files);

    return EXIT_SUCCESS;
}
