/*Esercizio 2: Linguaggio C: 10 punti
Estendere il programma invsymlink: se il parametro è una directory e non un file allora tutti i link
simbolici presenti nella directory devono venir "invertititi".*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/limits.h>
#include <dirent.h>


void revert_link(char *link_path)
{
    char target_path[PATH_MAX];
    ssize_t len = readlink(link_path, target_path, sizeof(target_path) - 1);
    if (len == -1) {
        perror("Errore readlink");
        return;
    }
    target_path[len] = '\0';

    // Ottieni il path assoluto del file vero (A)
    char real_target_path[PATH_MAX];
    if (realpath(target_path, real_target_path) == NULL) {
        perror("Errore realpath");
        return;
    }

    // Rimuovi il link simbolico B
    if (unlink(link_path) == -1) {
        perror("Errore unlink");
        return;
    }

    // Sposta (rename) A (il vero file) al posto di B
    if (rename(real_target_path, link_path) == -1) {
        perror("Errore rename");
        return;
    }

    // Crea un link simbolico da A → B
    if (symlink(link_path, real_target_path) == -1) {
        perror("Errore symlink");
        return;
    }
}


int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Uso: %s <directory>", argv[0]);
        return EXIT_FAILURE;
    }
    struct stat st;
    char *path = argv[1];
    if(stat(path, &st) == -1)
    {
        perror("Errore statistiche su path");
        return EXIT_FAILURE;
    }
    if(S_ISDIR(st.st_mode))
    {
        DIR *dir = opendir(path);
        if(!dir){
            perror("Errore apertura directory");
            return EXIT_FAILURE;
        }
        struct dirent *entry;
        while((entry = readdir(dir))!= NULL)
        {
            char full_path[PATH_MAX];
            if(strcmp("..", entry->d_name) == 0 || strcmp(".", entry->d_name)== 0)
            {
                continue;
            }
            snprintf(full_path, sizeof(full_path),"%s/%s", path,entry->d_name);
            struct stat stats;
            if(lstat(full_path, &stats) == -1)
            {
                perror("Errore lettura file/dir statistiche");
                continue;
            }
            if(S_ISLNK(stats.st_mode))
            {
                revert_link(full_path);
            }
        }
        closedir(dir);
    }
    else if(S_ISLNK(st.st_mode))
    {
        revert_link(path);
    }
    return EXIT_SUCCESS;
}