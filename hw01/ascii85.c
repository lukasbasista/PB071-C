#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void printEncoded(unsigned int n){
    int c;
    int a[5];
    for (int i = 0; i <= 4; i++){
        c = n % 85 + 33;
        n = n / 85;
        a[i]=c;
    }
    for (int i = 4; i >= 0; i--){
        putchar(a[i]);
    }
}

void printDecoded(unsigned int n){
    unsigned int c = 255;
    int a[4];
    for (int i = 0; i < 4; i++){
        a[i] = n & c;
        n = n >> 8;
    }
    for (int i = 3; i >= 0; i--){
        putchar(a[i]);
    }
}

int encode(void)
{
    int c;
    int cCount = 0;
    unsigned int c4 = 0;
    while ((c = getchar()) != EOF) {
        cCount++;
        c4 = c4 | c;
        if (cCount%4!=0) {
            c4 = c4 << 8;
        }
        if (cCount % 4 == 0) {
            printEncoded(c4);
            c4 = 0;
        }
    }
    if (cCount%4!=0){
        int a=4-cCount%4;
        for (int i=1; i<a+1; i++){
            c4 = c4 | '\0';
            if ((cCount+i)%4!=0){ c4 = c4<<8; }
        }
        printEncoded(c4);
    }
    printf("\n");
    return 0;
}

int decode(void)
{
    int c;
    int cCount = 0;
    unsigned int c4 = 0;
    while ((c = getchar()) != EOF) {
        if (isspace(c)) {
            continue;} else {
            cCount++;
            c4 = c4 * 85 + (c - 33);
            if (cCount % 5 == 0) {
                printDecoded(c4);
                c4 = 0;
            }
        }
    }
    return 0;
}

// ================================
// DO NOT MODIFY THE FOLLOWING CODE
// ================================
int main(int argc, char *argv[])
{
    int retcode = 1;

    if (argc == 1 || (argc == 2 && strcmp(argv[1], "-e") == 0)) {
        retcode = encode();
    } else if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        retcode = decode();
    } else {
        fprintf(stderr, "usage: %s [-e|-d]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (retcode != 0) {
        fprintf(stderr, "an error occured\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
