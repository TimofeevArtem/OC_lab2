#include <stdio.h>

int main(){
    int ch;
    int fl = 0;
    while((ch = getchar()) != EOF){
        if(ch == ' ' && fl == 0){
            fl = 1;
            putchar(ch);
        } else if(ch != ' ' && fl == 1){
            fl = 0;
            putchar(ch);
        }
        else if(ch == ' ' && fl == 1){
            continue;
        }
        else{
            putchar(ch);
        }
    }
    return 0;
}