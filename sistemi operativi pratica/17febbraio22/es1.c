#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <errno.h>
#include <libgen.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <directory_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *dir_path = argv[1];
    DIR *dir = opendir(dir_path);
    if (!dir)
    {
        perror("Errore apertura dir");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (lstat(full_path, &st) == -1)
        {
            perror("Errore lstat");
            continue;
        }

        if (S_ISLNK(st.st_mode))
        {
            char target[PATH_MAX];
            ssize_t len = readlink(full_path, target, sizeof(target) - 1);
            if (len == -1)
            {
                perror("Errore readlink");
                continue;
            }
            target[len] = '\0';

            char resolved_path[PATH_MAX];

            if (target[0] == '/')
            {
                // Path assoluto → usalo direttamente
                snprintf(resolved_path, sizeof(resolved_path), "%s", target);
            }
            else
            {
                // Path relativo → rispetto alla directory del link
                char temp[PATH_MAX];
                snprintf(temp, sizeof(temp), "%s", full_path);  // Copia sicura per dirname() (Modifica in place la stringa)
                char *link_dir = dirname(temp); //Passo la directory
                snprintf(resolved_path, sizeof(resolved_path), "%s/%s", link_dir, target); //PRinto la directory col target
            }
            struct stat target_st;
            if (lstat(resolved_path, &target_st) == -1)
            {
                // Il target potrebbe non esistere
                continue;
            }

            if (S_ISLNK(target_st.st_mode))
            {
                printf("Rimuovo link simbolico nidificato: %s -> %s\n", full_path, target);
                if (unlink(full_path) == -1)
                {
                    perror("Errore unlink");
                }
            }
            
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}
