#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../Header/Trie.h"

#define DIZIONARIO "../Text/Dizionario.txt"

void Caps_Lock(char* str){
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] >= 'a' && str[i] <= 'z') {
            // Se il carattere è minuscolo, lo trasformo in maiuscolo
            str[i] = str[i] - 32;
        }
        i++;
    }
    return;
}

// int main(){
//     Trie*t; 
//     t = create_node();
//     char stringa[]="ciao";
//     Caps_Lock(stringa);
//     insert_Trie(t,stringa);
//     /*insert_Trie()*/
    
//     int retvalue,dizionario_fd;ssize_t n_read;
//     char buffer[16];
//     dizionario_fd = open(DIZIONARIO,O_RDONLY);
//     //leggo dal dizionario
//     //controllo di aver letto qualcosa
//     n_read = 1;
//     int j  =0;
//     off_t safe_pos;
//     while(n_read != 0){
//         n_read = read(dizionario_fd,buffer,16);
//         char word[16];
//         for(int i =0;i<n_read;i++){
//             if (buffer[i] == '\n'){
//                 word[j] = '\0';
//                 insert_Trie(t,word);
//                 j = 0;
//                 safe_pos = i+1;
//             }else{
//                 if (j<16){
//                     word[j++] = buffer[i];
//                 }
//             }
//         }
//         off_t offset = lseek(dizionario_fd,0,SEEK_CUR);
//         off_t diff = offset-safe_pos;
//         offset -= diff;
//         lseek(dizionario_fd,offset,SEEK_SET);  
//     }
    
//     close(dizionario_fd);
    
//     write(1,"inserito tutto",15);
//     //char b2[280000];
//     //Print_Trie(t,b2,0);
//     printf("%d\n",search_Trie("ciao",t));
//     return 0;
// }


int main() {
    Trie *t; 
    t = create_node();
    
    int dizionario_fd = open(DIZIONARIO, O_RDONLY);
    if (dizionario_fd == -1) {
        perror("Errore apertura file dizionario");
        return 1;
    }

    ssize_t n_read;
    char buffer[16];
    char word[256];
    int j = 0;
    
    while ((n_read = read(dizionario_fd, buffer, 16)) > 0) {
        for (int i = 0; i < 16; i++) {
            if (buffer[i] == '\n') {
                word[j] = '\0';
                printf("inserisco;%s\n",word);
                insert_Trie(t, word);
                printf("inserito\n");
                j = 0;
            } else {
                if (j < 15) {
                    word[j++] = buffer[i];
                    printf("top\n");
                }
            }
        }
    }
    printf("fine\n");

    close(dizionario_fd);
    //char bufer[256];
    write(1, "inserito tutto\n", 15);
    //Print_Trie(t,bufer,0);
    printf("%d\n", search_Trie("ciao", t));
    return 0;
}