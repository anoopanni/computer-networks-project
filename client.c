#include <stdio.h>


int main(int argc, char *argv[]){

    if (argc != 3)
    {
        printf("Use format: ./client server_name port_number(ex: localhost 3333)\n");
        exit(1); 
    }


    return 0;
}