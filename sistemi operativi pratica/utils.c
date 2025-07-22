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

// Funzione che confronta due file byte per byte
int files_are_equal(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb");
    FILE *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;  // se uno dei due non si apre, consideriamo non uguali
    }

    int c1, c2;
    do {
        c1 = fgetc(f1);
        c2 = fgetc(f2);
        if (c1 != c2) {
            fclose(f1);
            fclose(f2);
            return 0;
        }
    } while (c1 != EOF && c2 != EOF);

    fclose(f1);
    fclose(f2);

    return c1 == EOF && c2 == EOF;
}


//Per i link simbolici usare lstat//





//Per vettori dinamici (esempio copiare argv): //
 char buffer[MAX_CMDLINE];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (n <= 0) {
        fprintf(stderr, "Nessun contenuto in /proc/%d/cmdline\n", target_pid);
        return EXIT_FAILURE;
    }

    buffer[n] = '\0'; // sicurezza

    // Conta gli argomenti
    size_t pos = 0;
    int arg_count = 0;
    while (pos < n) {
        arg_count++;
        pos += strlen(&buffer[pos]) + 1;
    }

    // Costruisci l'array argv[]
    char **new_argv = calloc(arg_count + 1, sizeof(char *));
    if (!new_argv) {
        fatal("calloc");
    }
    pos = 0;
    for (int i = 0; i < arg_count; i++) {
        new_argv[i] = &buffer[pos];
        pos += strlen(&buffer[pos]) + 1;
    }
    new_argv[arg_count] = NULL;








    //Funzione per copiare da start a end da in fd a out fd

    void copy_range(int in_fd, int out_fd, off_t start, off_t end) {
    char buffer[BUF_SIZE];
    lseek(in_fd, start, SEEK_SET);
    lseek(out_fd, start, SEEK_SET);

    off_t remaining = end - start;

    while (remaining > 0) {
        ssize_t to_read = remaining < BUF_SIZE ? remaining : BUF_SIZE;
        ssize_t read_bytes = read(in_fd, buffer, to_read);
        if (read_bytes <= 0) break;
        write(out_fd, buffer, read_bytes);
        remaining -= read_bytes;
    }
}



Funzione per risolvere il link relativo al dirname
char temp[PATH_MAX];
snprintf(temp, sizeof(temp), "%s", full_path);  // Copia sicura per dirname()
char *link_dir = dirname(temp);
snprintf(resolved_path, sizeof(resolved_path), "%s/%s", link_dir, target);



#include <stdio.h>
#include <stdbool.h>

/* Verifica eseguibilità path*/
bool is_elf(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    unsigned char magic[4];
    if (fread(magic, 1, 4, f) != 4) {
        fclose(f);
        return false;
    }
    fclose(f);

    return magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F';
}



int is_script(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file)
        return 0;
    
    char buffer[2] = {0};
    fread(buffer, 1, 2, file);
    fclose(file);
    
    return memcmp(buffer, "#!", 2) == 0;
}

// Funzione per determinare se un file è un eseguibile binario ELF
int is_elf(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return 0;
    }
    
    char buffer[5] = {0};
    fread(buffer, 1, 4, file);
    fclose(file);
    
    return memcmp(buffer, "\x7f""ELF", 4) == 0;
}
