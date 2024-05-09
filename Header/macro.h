//definizioni di macro per syscall
#define buff_size 256
#define SYSC(v,c,m)\
    if ((v = c ) == -1){perror(m);exit(errno);};

#define SYSCN(v,c,m)\
    if ((v = c) == NULL){perror(m);exit(errno);};

#define SYST(v,c,m)\
    if ((v = c) != 0){perror(m);exit(errno);};

/*SHORTCUT PER USARE LA WRITE COME UNA PRINTF*/
#define writef(retvalue,message)\
    SYSC(retvalue,write(STDOUT_FILENO,message,strlen(message)),"nella writef");

/*NON SERVE A NIENE MA ALMENO IL PEDANTIC DEL MAKEFILE NON DA WARNING*/
void fun();
