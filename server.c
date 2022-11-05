#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdbool.h>
#include <utmpx.h>
#include <paths.h>
#include <time.h>

#define CS_FIFO "client_server_fifo"
#define SC_FIFO "server_client_fifo"

bool LOGGED_IN;

// client -> server FIFO
// server -> client FIFO
int cs_f, sc_f; 

void initialSetup();
void mainProgram();
void helpClient();
void login();
void logout();
void quitApp();
void get_LoggedUsers();
void get_ProcessInfo();

int main() {
  initialSetup();

  mainProgram();
}

void mainProgram() {
  int n;
  char userInput[50];
  int numberCharacters;
  do {
    if((n = read(cs_f, userInput, 50)) == -1)
      perror("[mainProgram] Eroare la citirea comenzii de la client! \n");
    else {
      userInput[n] = '\0';
      if(LOGGED_IN) {
        if(!strcasecmp(userInput, "get-logged-users"))
          get_LoggedUsers();
        else if(!strcasecmp(userInput, "get-proc-info"))
          get_ProcessInfo();
        else if(!strcasecmp(userInput, "logout"))
          logout();
        else if (!strcasecmp(userInput, "quit"))
          quitApp();
        else if (!strcasecmp(userInput, "help"))
          helpClient();
        else {
          char l_exit_text[] = "[Logged-in] Invalid Command! Type help to see available commands.\0";
          numberCharacters = strlen(l_exit_text);
          write(sc_f, &numberCharacters, 4);
          write(sc_f, l_exit_text, numberCharacters);
          printf("[logged-in] Invalid command. \n\n");
        }
          
      }
      else {
        if (!strcasecmp(userInput, "login"))
          login();   /// child processes the authentication sends result to parent
        else if (!strcasecmp(userInput, "quit"))
          quitApp();
        else if (!strcasecmp(userInput, "help"))
          helpClient();
        else {
          char nl_exit_text[] = "[Logged-out] Invalid Command! Type help to see available commands.\0";
          numberCharacters = strlen(nl_exit_text);
          write(sc_f, &numberCharacters, 4);
          write(sc_f, nl_exit_text, numberCharacters);
          printf("[logged-out] Invalid command. \n\n");
        }
          
      }

    }
  } while(n > 0);
}

void initialSetup() {
  LOGGED_IN = false;
  mknod(CS_FIFO, S_IFIFO | 0666, 0);
  mknod(SC_FIFO, S_IFIFO | 0666, 0);
  printf("Establishing connection to client. \n");
  cs_f = open(CS_FIFO, O_RDONLY); // client -> server   =>    read only for server
  sc_f = open(SC_FIFO, O_WRONLY); // server -> client   =>    write only for server
  printf("Connection to client established. \n");
}

void login() { /// socketpair && fifo
  int numberCharacters;
  int childProcess;
  char returnValue[10] = "";
  int n;
  int sockp[2];

  //parent variables
  char userInput[50] = "";

  //child variables
  char fileText[50] = "";
  char parentInput[50];
  FILE *users_fp;
  
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0)
  {
    perror("[server/login] Error at the socketpair\n");
    exit(1);
  }
  
  switch(childProcess = fork()) {
    case -1:
      perror("  [login/child] There has been an error in the login fork. \n");

    case 0: /// child
      /// importing userInput from Parent
      close(sockp[1]);
      if(read(sockp[0], parentInput, 50) < 0)
          perror("  [login/child] Error when reading username from parent\n");
      
      printf("  [login/child] Read userinput from parent\n");

      users_fp = fopen("users.txt", "r");
      if (users_fp == NULL) {
        perror("  [login/child] Error while opening users.txt\n");
        exit(-1);
      }
      printf("  [login/child] opened users.txt\n");

      while(fgets(fileText, 50, users_fp)) {    /// getting input from users.txt
        if(!feof(users_fp))   /// if it isn't the last line
          fileText[strlen(fileText) - 1] = '\0';    /// removes endline
        
        printf("  [login/child] fileText = %s \n", fileText);
        if(!strcasecmp(fileText, parentInput)) {
          printf("  [login/child] Username found! \n");
          if(write(sockp[0], "TRUE", 4) < 0)
            perror("  [login/child] Error writing true to parent \n");
          exit(1);
        }
      }
      if (write(sockp[0], "FALSE", 5) < 0)
        perror("  [login/child] Error writing false to parent \n");
      close(sockp[0]);
      fclose(users_fp);
      printf("  [login/child] Username not Found\n");
      exit(0);
    default:      /// parent
      numberCharacters = 11;
      write(sc_f, &numberCharacters, 4);
      if (write(sc_f, "username: ", numberCharacters) == -1) /// writing to client to say username
        perror("[login/parent] Eroare la scrierea prompt-ului username catre client\n");
      else
        printf("[login/parent] Am scris catre client: username. \n");

        if ((n = read(cs_f, userInput, 50)) == -1)
          perror("[login/parent] Eroare la citirea username-ului de la client! \n");
        printf("[login/parent] read userinput from client\n");

        close(sockp[0]); // copilul -> socket 0, parinte -> socket 1
        if (write(sockp[1], userInput, sizeof(userInput)) < 0)
          perror("[login/parent]Eroare la trimiterea username-ului catre copil\n");
        printf("[login/parent] Wrote userinput to child.\n");

        if(read(sockp[1], returnValue, 10) < 0)
          perror("[login/parent] Error reading the return value \n");
        printf("[login/parent] returnValue = %s\n", returnValue);

        close(sockp[1]);
        if (!strcasecmp(returnValue, "TRUE")) {
          numberCharacters = 24;
          write(sc_f, &numberCharacters, 4);
          if (write(sc_f, "Logged in successfully.", 24) == -1)
            perror("[login/parent] Eroare la scriere login.");
          printf("[login/parent] Logged in! \n");
            LOGGED_IN = true;
        }
        else {
          numberCharacters = 29;
          write(sc_f, &numberCharacters, 4);
          if (write(sc_f, "The username is not correct.", 29) == -1)
            perror("[login/parent] Eroare la scriere !login.");
          printf("[login/parent] NOT logged in! \n");
          LOGGED_IN = false;
        }

        printf("[login/parent] Waiting for child...\n");
        wait(NULL); /// WAIT E ULTIMA
        printf("[login/parent] Done.\n\n");
  }
}
void logout() {
  LOGGED_IN = false;
  int numberCharacters;
  char nl_exit_text[] = "Logged-out\0";
  numberCharacters = strlen(nl_exit_text);
  write(sc_f, &numberCharacters, 4);
  write(sc_f, nl_exit_text, strlen(nl_exit_text));
  printf("[SERVER] LOGGED OUT\n");
  //exit(0);
}

void helpClient() {  /// fifo
  int numberCharacters;
  char loginHelp[111] = "You are currently logged in. The available commands are:\n  get-logged-users\n  get-proc-info\n  logout\n  quit\0";
  char logoutHelp[76] = "You are currently logged out. The available commands are:\n  login\n  quit\0";

  if(LOGGED_IN) {
    numberCharacters = strlen(loginHelp);
    write(sc_f, &numberCharacters, 4);
    if (write(sc_f, loginHelp, numberCharacters) < 0)
      perror("[help/login] Error sending loginHelp \n");
    printf("[help/login] Sent help message.\n");
  }
  else {
    numberCharacters = strlen(logoutHelp);
    write(sc_f, &numberCharacters, 4);
    if (write(sc_f, logoutHelp, numberCharacters) < 0)
      perror("[help/logout] Error sending logoutHelp \n");
    printf("[help/logout] Sent help message.\n");
  }
}

void quitApp() {  /// fifo
  int numberCharacters;
  char nl_exit_text[] = "[QUIT]\0";
  numberCharacters = strlen(nl_exit_text);
  write(sc_f, &numberCharacters, 4);
  write(sc_f, nl_exit_text, numberCharacters);
  printf("[SERVER] QUIT\n");
  exit(0);
}

void get_LoggedUsers() {  /// pipe && fifo
  int numberCharacters;
  int cp_pipe[2];

  char userInfo[300] = "";
  char currentUser[300] = "";
  char returnText[300] = "";
  char parentReturnText[300] = "";
  struct tm *timeptr;
  char timeText[300] = "";
  struct utmpx *usr;

  if(pipe(cp_pipe) == -1) {
    perror("PIPE ERROR! \n");
    exit(1);
  }

  switch (fork())
  {
  case -1:
    perror("[getLoggedUsers/Fork] Fork error! \n");
    exit(1);
  case 0: /// copilul
    setutxent();
    usr = getutxent();
    while (usr != NULL) {
      if (usr->ut_type == 7) { // if it's a user process
        if (!strstr(userInfo, usr->ut_user)) {
          strcat(userInfo, usr->ut_user);
          strcat(userInfo, "\n");

          timeptr = localtime(&usr->ut_tv.tv_sec);
          strftime(timeText, sizeof(timeText), "%c", timeptr);

          sprintf(currentUser, "\n  user: %s\n  host: %s\n  time: %s  \n", usr->ut_user, usr->ut_host, timeText);
          strcat(returnText, currentUser);
        }
      }
      usr = getutxent();
    }
    endutxent();


    if(write(cp_pipe[1], returnText, strlen(returnText)) == -1)
      perror("  [getLoggedUsers/child] Eroare la scriere\n");
    close(cp_pipe[1]);
    printf("  [getLoggedUsers/child] Sent to parent\n");
    exit(1);
  default: /// parintele
    printf("[getLoggedUsers/parent] Waiting for child to send output...\n");
    if(read(cp_pipe[0], parentReturnText, 300) == -1)
      perror("[getLoggedUsers/parent] Eroare la citire\n");
    close(cp_pipe[0]);
    
    printf("[getLoggedUsers/parent] Received output from child.\n");

    numberCharacters = strlen(parentReturnText);
    write(sc_f, &numberCharacters, 4);
    if(write(sc_f, parentReturnText, numberCharacters) < 0)
      perror("[getLoggedUsers/parent] Eroare la scriere catre client\n");

    printf("[getLoggedUsers/parent] Sent output to client.\n");

    printf("[getloggedUsers/parent] Waiting for child...\n");
    wait(NULL);
    printf("[getLoggedUsers/parent] Done. \n\n");
    break;
  }
}

void get_ProcessInfo() {  /// socketpair && fifo
  int sockp[2];

  /// Parent variables
  int numberCharacters;
  int pid_value = -1;
  char pid_char[20] = "";
  char parentReturnText[300] = "";

  /// Child variables
  int processPid = -1;
  char childReturnText[300] = "Pid not found.\n";
  struct kinfo_proc *proc_list = NULL;
  size_t length = 0;
  int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, KERN_PROCNAME, 0};
  char processStatus[50] = "";

  if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) == -1) {
    perror("[getProcessInfo] Socketpair error! \n");
    exit(1);
  }

  switch (fork()) {
    case -1:
      perror("[getProcessInfo/Fork] Fork error! \n");
      exit(1);
    case 0: /// child
      close(sockp[1]);
      if(read(sockp[0], &processPid, 4) == -1)
        perror("  [getProcessInfo/child] eroare la citirea pidului de la parinte\n");

      if (sysctl(name, (sizeof(name) / sizeof(*name)) - 1, NULL, &length, NULL, 0) < 0) {
        perror("  [getProcessInfo/child] sysctl with NULL error! \n");
        exit(1);
      }
      
      proc_list = malloc(length);
      if (!proc_list)
        printf("  [getProcessInfo/child] Error allocating memory for proc_list");

      if(sysctl(name, (sizeof(name) / sizeof(*name)) - 1, proc_list, &length, NULL, 0) < 0) {
        printf("  [getProcessInfo/child] Error calling sysctl with buffer allocated \n");
        exit(1);
      }

      int proc_count = length / sizeof(struct kinfo_proc);
      printf("  [getProcessInfo/child] Searching for process...\n");
      
      for (int i = 0; i < proc_count; i++) {
        struct kinfo_proc *proc = &proc_list[i];

        if (proc->kp_proc.p_pid == processPid) {
          if (proc->kp_proc.p_stat == SIDL)
            strcpy(processStatus, "idle");
          else if(proc->kp_proc.p_stat == SRUN)
            strcpy(processStatus, "running");
          else if(proc->kp_proc.p_stat == SSLEEP)
            strcpy(processStatus, "sleeping");
          else if(proc->kp_proc.p_stat == SSTOP)
            strcpy(processStatus, "stopped");
          else if(proc->kp_proc.p_stat == SZOMB)
            strcpy(processStatus, "zombie");
          else
            strcpy(processStatus, "unknown");

          sprintf(childReturnText, "\n  name: %s\n  pid: %d\n  ppid: %d\n  uid: %d\n  state: %s\n", proc->kp_proc.p_comm, proc->kp_proc.p_pid, proc->kp_eproc.e_ppid, proc->kp_eproc.e_pcred.p_ruid, processStatus);
          

          printf("  [getProcessInfo/child] Found process\n");
        }
      }

      if(write(sockp[0], childReturnText, strlen(childReturnText)) == -1)
        perror("  [getProcessInfo/child] eroare la scrierea pidului la parinte\n");
      close(sockp[0]);
      exit(1);
      
    default: ///  parent
      numberCharacters = 6;
      write(sc_f, &numberCharacters, 4);
      if (write(sc_f, "pid: ", numberCharacters) == -1) /// writing to client to say pid
        perror("[getProcessInfo/parent] Eroare la scrierea prompt-ului pid catre client\n");
      else
        printf("[getProcessInfo/parent] Wrote to client: pid. \n");

      if ((read(cs_f, pid_char, 20)) == -1)
        perror("[getProcessInfo/parent] Eroare la citirea pid-ului de la client! \n");
      printf("[getProcessInfo/parent] Read pid from client\n");
      pid_value = atoi(pid_char);

      if(write(sockp[1], &pid_value, 4) == -1)
        perror("[getProcessInfo/parent] Eroare scriere pid catre copil\n");

      if(read(sockp[1], parentReturnText, 300) == -1)
        perror("[getProcessInfo/parent] Eroare la citirea informatiilor despre proces de la copil\n");
      close(sockp[1]);

      numberCharacters = strlen(parentReturnText);
      write(sc_f, &numberCharacters, 4);
      if (write(sc_f, parentReturnText, numberCharacters) == -1) /// writing to client final
        perror("[getProcessInfo/parent] Eroare la scrierea prompt-ului executed catre client\n");
      else
        printf("[getProcessInfo/parent] Sent to client: %s \n", parentReturnText);
      printf("[getProcessInfo/parent] Waiting for child... \n");
      wait(NULL);
      printf("[getProcessInfo/parent] Done. \n\n");
  }
}