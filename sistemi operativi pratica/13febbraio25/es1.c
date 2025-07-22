#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include <libgen.h>  // per dirname()
#include <limits.h>

#include <limits.h>  // PATH_MAX
#include <stdlib.h>  // realpath

int get_symlink(const char *file_path, const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Errore apertura directory");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];
    char link_target[PATH_MAX];

    // Risolvo file_path in percorso assoluto canonico
    char real_file_path[PATH_MAX];
    if (!realpath(file_path, real_file_path)) {
        perror("Errore realpath file_path");
        closedir(dir);
        return EXIT_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (lstat(full_path, &st) == -1)
            continue;

        if (S_ISDIR(st.st_mode)) {
            get_symlink(file_path, full_path);
        }
        else if (S_ISLNK(st.st_mode)) {
            ssize_t len = readlink(full_path, link_target, sizeof(link_target) - 1);
            if (len == -1)
                continue;
            link_target[len] = '\0';

            // Risolvo percorso a cui punta il link simbolico in percorso assoluto
            char real_link_target[PATH_MAX];
            // Nota: realpath di un link simbolico restituisce il percorso assoluto della destinazione
            if (!realpath(full_path, real_link_target)) {
                // se il link è rotto o non risolvibile, salta
                continue;
            }

            // Confronto i percorsi canonici assoluti
            if (strcmp(real_link_target, real_file_path) == 0) {
                printf("%s -> %s\n", full_path, link_target);
            }
        }
    }

    closedir(dir);
    return 0;
}




int get_hardlink(const char *file_path, const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (!dir)
    {
        perror("Errore apertura directory");
        return EXIT_FAILURE;
    }

    struct stat filestat;
    if (stat(file_path, &filestat) == -1)
    {
        perror("Errore stat file");
        closedir(dir);
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    char full_path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        struct stat st;
        if (lstat(full_path, &st) == -1)
        {
            perror("Errore lstat");
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            get_hardlink(file_path, full_path);
        }
        else if (S_ISREG(st.st_mode))
        {
            // Confronta inode e device per hard link
            if (st.st_ino == filestat.st_ino && st.st_dev == filestat.st_dev)
            {
                printf("Hard link trovato: %s\n", full_path);
            }
        }
    }
    closedir(dir);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <-s|-l> <file_path> <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-s") == 0)
    {
        printf("Elenco link simbolici che puntano a %s:\n", argv[2]);
        return get_symlink(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "-l") == 0)
    {
        printf("Elenco hard link di %s:\n", argv[2]);
        return get_hardlink(argv[2], argv[3]);
    }
    else
    {
        fprintf(stderr, "Opzione errata, usare -l o -s\n");
        return EXIT_FAILURE;
    }
}
