#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#define MAX_PATH 4096

int tree(char *path, int level)
{
    DIR *dir = opendir(path);
    if (!dir) {
        perror("Errore apertura directory");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) == -1) {
            perror("Errore stat");
            continue;
        }

        for (int i = 0; i < level; i++) {
            printf("\t");
        }
        printf("%s\n", entry->d_name);

        if (S_ISDIR(st.st_mode)) {
            tree(fullpath, level + 1);
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [directory]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("%s\n", argv[1]);
    return tree(argv[1], 1);
}
