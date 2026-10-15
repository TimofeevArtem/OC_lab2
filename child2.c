#include <stdio.h>

int main(){
    int ch;
    int fl = 0;
    while((ch = getchar()) != EOF){
        if (ch == ' '){
            if (!fl){
                putchar(ch);
                fflush(stdout);
            }
            fl = 1;
        }
        else{
            putchar(ch);
            fflush(stdout);
            fl = 0;
        }
    }
    return 0;
}