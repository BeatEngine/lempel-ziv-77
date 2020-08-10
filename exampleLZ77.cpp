#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LempelZiv77.h"

int main(int args, char** arg)
{

    char* text = "Hello Max. Hello Pete. Hello World!";

    //CompressedDataLZW data;

    //data.compress(text, strlen(text));
    printf("Original    : %s\n", text);
    int sizeCompressed = 0;
    unsigned char* compressed = LZWcompression::compress(text, strlen(text), (size_t*)&sizeCompressed);

    printf("Compressed  : ");
    for(int i = 0; i < sizeCompressed; i++)
    {
        printf("%d", compressed[i]);
    }
    printf("\n");

    for(int i = 0; i < sizeCompressed; i+=2)
    {
        if(compressed[i+1] == 0)
        {
            printf("%c %d", compressed[i], compressed[i+1]);
        }
        else
        {
            printf("%d %d", compressed[i], compressed[i+1]);
        }
        if(i+2 < sizeCompressed)
        {
            printf(", ");
        }
    }
    printf("\n");

    int sizeDecompressed = 0;
    char* decompressed = LZWcompression::decompress(compressed, sizeCompressed, (size_t*)&sizeDecompressed);

    printf("Decompressed: %.*s\n", sizeDecompressed, decompressed);

    int dbg = 1;


    return 0;
}

