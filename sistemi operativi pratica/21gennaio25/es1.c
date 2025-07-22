#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/limits.h>
#include <string.h>



int samecount(char *dir_path, char* file_path)
{

    DIR *dir = opendir(dir_path);
    struct stat st_file;
    struct dirent *entry;
    if(!dir)
    {
        perror("Errore apertura dir");
        return EXIT_FAILURE;
    }
    if(stat(file_path, &st_file)== -1)
    {
        perror("Errore statistiche sul file target");
        return EXIT_FAILURE;
    }
    char full_path[PATH_MAX];
    while((entry = readdir(dir))!=NULL)
    {
        if(strcmp(entry->d_name,"..")==0 || strcmp(entry->d_name, ".")== 0)
        {
            continue;
        }
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        struct stat st;
        if(stat(full_path, &st)== -1)
        {
            perror("Errore statistiche sul file");
            continue;
        }
        if(S_ISDIR(st.st_mode)){
            samecount(full_path,file_path);
        }
        else if(S_ISREG(st.st_mode))
        {

            if(st.st_size == st_file.st_size && (st_file.st_ino != st.st_ino || st_file.st_dev != st.st_dev))
            {
                fprintf(stdout, "File trovato %s \n",full_path);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        fprintf(stdout, "Usage %s <directory> <file_path>", argv[0]);
        return EXIT_FAILURE;
    }
    printf("Elenco file \n");
    samecount(argv[1], argv[2]);
    return 0;
}