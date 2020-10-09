#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int encode(void)
{
    // TODO: implement Ascii85 encoding
    return 0;
}

int decode(void)
{
    // TODO: implement Ascii85 decoding
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
