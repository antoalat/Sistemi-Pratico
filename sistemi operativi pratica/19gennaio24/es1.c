#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 1024
/*
n1 = len stringa dest
n2 = len stringa da concatenare
*/
char *strncat_with_EOS(char *dest, char *src, size_t n1, ssize_t n2) {
  for (int i = 0; i < n2; i++)
    dest[n1 + i] = src[i];

  return dest;
}

/*PRINTF SI INTERROMPE CON \0*/

int main(int argc, char *argv[]) {
  int size = 0;
  // calcolo size per allocare memoria
  for (int i = 1; i < argc; i++) {
    size += strlen(argv[i]) + 1;
  }
  // allocazione
  char *str = malloc(sizeof(char) * size);
  size = 0;
  for (int i = 1; i < argc; i++) {
    strncat_with_EOS(str, argv[i], size, strlen(argv[i]) + 1);
    size += strlen(argv[i]) + 1;
  }
  write(STDOUT_FILENO, str, size);
}



