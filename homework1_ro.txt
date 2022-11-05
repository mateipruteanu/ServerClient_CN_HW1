Dezvoltati doua aplicatii (denumite "client" si "server") ce comunica intre ele pe baza unui protocol care are urmatoarele specificatii:

- comunicarea se face prin executia de comenzi citite de la tastatura in client si executate in procesele copil create de server;
- comenzile sunt siruri de caractere delimitate de new line;
- raspunsurile sunt siruri de octeti prefixate de lungimea raspunsului;
- rezultatul obtinut in urma executiei oricarei comenzi va fi afisat de client;
- procesele copil din server nu comunica direct cu clientul, ci doar cu procesul parinte;
- protocolul minimal cuprinde comenzile: 
      - "login : username" - a carei existenta este validata prin utilizarea unui fisier de configurare, care contine toti utilizatorii care au acces la functionalitati. Executia comenzii va fi realizata intr-un proces copil din server;
      - "get-logged-users" - afiseaza informatii (username, hostname for remote login, time entry was made) despre utilizatorii autentificati pe sistemul de operare (vezi "man 5 utmp" si "man 3 getutent"). Aceasta comanda nu va putea fi executata daca utilizatorul nu este autentificat in aplicatie. Executia comenzii va fi realizata intr-un proces copil din server;
      - "get-proc-info : pid" - afiseaza informatii (name, state, ppid, uid, vmsize) despre procesul indicat (sursa informatii: fisierul /proc/<pid>/status). Aceasta comanda nu va putea fi executata daca utilizatorul nu este autentificat in aplicatie. Executia comenzii va fi realizata intr-un proces copil din server;
      - "logout";
      - "quit".
- in implementarea comenzilor nu se va utiliza nicio functie din familia "exec()" (sau alta similara, de ex. popen(), system()...) in vederea executiei unor comenzi de sistem ce ofera functionalitatile respective;
- comunicarea intre procese se va face folosind cel putin o data fiecare din urmatoarele mecanisme ce permit comunicarea: pipe-uri, fifo-uri si socketpair.


Observatii:
- termen de predare: laboratorul din saptamana 5;
- orice incercare de frauda, in functie de gravitate, va conduce la propunerea pentru exmatriculare a studentului in cauza sau la punctaj negativ.
