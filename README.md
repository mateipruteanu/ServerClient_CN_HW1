# ServerClient [Computer Networks Homework1]

## Server and client apps that can show info about currently logged users and show process information.

Built in the C language for the Computer Networks course @Facultatea de Informatica, UAIC, Iasi.


***How to use***
 Run ```./server``` and ```./client```.
 
 If logged-out, the client only has access to 
  - login
  - quit

 If logged-in, the client has access to:
  - get-logged-users
  - get-proc-info
  - logout
  - quit
  
  At any time, the client can type ```help``` to see the available commands.
  
 ```get-logged-users``` shows the users that are currently logged-in to the system and the time they logged-in.
 
 
 ```get-proc-info``` asks for the PID of the process for which we want the info and then shows the PID, PPID, UID and its state.
 
 
 ```login``` asks for a username, checks if it exists in the "users.txt" file, and if it does, it logs the client in.
 
 
 ```logout``` logs the client out.
 
 
 ```quit``` quits the program.
 
  
