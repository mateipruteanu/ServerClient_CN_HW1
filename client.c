#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>

#define CS_FIFO "client_server_fifo"
#define SC_FIFO "server_client_fifo"

int cs_f, sc_f;
char userInput[100];

void initialSetup();
void mainProgram();

int main() {
  initialSetup();

  mainProgram();
}

void mainProgram() {
  int numberCharacters = -1;

  while (printf("\n[client] "), fgets(userInput, sizeof(userInput), stdin), !feof(stdin)) {
    userInput[strlen(userInput) - 1] = '\0'; /// scoate newline de la fgets
    char receivedTxt[400] = "";
    if(strcasecmp(userInput, "")) {
      write(cs_f, userInput, strlen(userInput));

      if(read(sc_f, &numberCharacters, 4) == -1) {
        perror("Eroare la citirea nrChar\n");
      }

      if (read(sc_f, receivedTxt, numberCharacters) != -1) {
        if (strcasecmp(userInput, "quit"))
          printf("  [server][%d] %s\n", numberCharacters, receivedTxt);
        else {
          printf("  [server][%d] %s\n", numberCharacters, receivedTxt);
          break;
        }
      }
      else
        perror("Eroare la citirea de la server\n");
    }   
  }
}

void initialSetup() {
  printf("Establishing connection to server. \n");
  cs_f = open(CS_FIFO, O_WRONLY); // client -> server   =>    write only for client
  sc_f = open(SC_FIFO, O_RDONLY); // server -> client   =>    read only for client
  printf("Connection to server established. \n");
}