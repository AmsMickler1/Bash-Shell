// Bash Shell Simulator in C
// COP4610 Intro to Operating Systems
// Amber Mickler and Benjamin Hybart


//TODO: the whole thing :D

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
  char instr[256];                          // user input will never be longer than 255 chars
  char *user = getenv("USER");
  char *pwd = getenv("PWD");

  do {
    printf("%s::%s$ ", user, pwd);          // shell header

  } while(strncmp(instr, "exit", 4) != 0);  // run until exit command


}
