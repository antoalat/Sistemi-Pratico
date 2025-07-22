
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 1024

int main(int argc, char *argv[]) {
  char str[MAX];
  int l = read(STDIN_FILENO, str, MAX);
  int c = 0;
  // calcolo numero di argomenti
  for (int i = 0; i < l; i++) {
    if (str[i] == '\0') {
      c++;
    }
  }
  // allocazione memoria array di stringhe (argomenti comando)
  char **args = malloc(sizeof(char *) * (c + 1));
  args[0] = str; // str da intender come &str[0]
  int k = 1;
  for (int i = 0; i < l; i++) {
    if (str[i] == '\0') {
      args[k++] = &str[i + 1];
    }
  }
  args[c] = NULL;
  execvp(args[0], args);
}
