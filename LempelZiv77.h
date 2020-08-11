#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>



class LZWwordbook
{

    char** phrases;
    int* lengths;
    int size;

    void resize(int newSize)
    {
        if(newSize < size)
        {
            for(int i = newSize; i < size; i++)
            {
                if(phrases[i] != 0)
                {
                    free(phrases[i]);
                }
            }
        }
        char** tmp = (char**)realloc(phrases, newSize);
        int* tim = (int*)realloc(lengths, newSize);
        if(tmp!=0 && tim!=0)
        {
            phrases = tmp;
            phrases[newSize-1] = 0;
            size = newSize;
            lengths = tim;
            lengths[newSize-1] = 0;
        }
    }

public:

    LZWwordbook()
    {
        phrases = 0;
        lengths = 0;
        size = 0;
    }

    ~LZWwordbook()
    {
        while (size > 0)
        {
            pop();
        }
    }

    bool contains(char* data, int length)
    {
        for(int i = 0; i < size; i++)
        {
            if(lengths[i] == length)
            {
                int c = 0;
                for(c = 0; c < length; c++)
                {
                    if(phrases[i][c] != data[c])
                    {
                        break;
                    }
                }
                if(c == length)
                {
                    return true;
                }
            }
        }
        return false;
    }

    void push(char* data, int length)
    {
        resize(size+1);
        phrases[size-1] = (char*)calloc(length, 1);
        memcpy(phrases[size-1], data, length);
        lengths[size-1] = length;
    }

    void pop()
    {
        if(size > 0)
        {
            resize(size-1);
        }
    }




};

class LZWcompression
{


    static int findInStr(char* source, int sourceSize, char* obj, int objSize)
    {
        //printf("%.*s in %.*s\n", objSize, obj, sourceSize, source);
        int f = -1;
        int c;
        for(int i = 0; i < sourceSize; i++)
        {
            c = 0;
            for(c = 0; c < objSize; c++)
            {
                if(source[i+c] != obj[c])
                {
                    break;
                }
            }
            if(c == objSize)
            {
                return i;
            }
        }
        return f;
    }


    public:


    static unsigned char* compress(char* data, size_t dataSize, size_t* returnCompressedSize)
    {
        unsigned char* compressed = (unsigned char*)calloc(2*dataSize, 1);
        int ci = 0;
        int pos;
        int pos2;
        int pc = 0;
        int u;
        for(int i = 0; i < dataSize; i++)
        {
            pos = findInStr(data, dataSize, data+i, 1);
            if(pos >= i)
            {
                compressed[ci] = data[i];
                ci++;
                compressed[ci] = 0;
                ci++;
                continue;
            }
            pos2 = pos;
            pc = 0;
            unsigned char relative = 0;
            for(u = 0; u < i; u++)
            {
                pos2 = findInStr(data, dataSize, data+i, i-u);
                if(pos2 != -1 && pos2 < i)
                {
                    //printf("Found %.*s in %.*s\n", i-u, data+i, dataSize, data);
                    pc = i-u;
                    relative = i-pos2;
                    i += pc-1;
                    break;
                }
            }
            compressed[ci] = relative;
            ci++;
            compressed[ci] = pc;
            ci++;
        }
        compressed = (unsigned char*)realloc(compressed, ci);
        *returnCompressedSize = ci;
        return compressed;
    }

    static void compressFile(char* inputFile, char* outputFile, size_t* returnCompressedSize)
    {
        FILE* input = fopen(inputFile, "rb");
        FILE* output = fopen(outputFile, "wb");
        fseek(input, 0, SEEK_END);
        long size = ftell(input);
        fseek(input, 0, SEEK_SET);
        unsigned char* buffer = (unsigned char*)malloc(256);
        unsigned char* result = 0;
        size_t resultSize = 0;
        long p = 0;
        long tp;
        
        while (p < size)
        {
            if(size - p < 256)
            {
                memset(buffer, 0, 256);
                tp = fread(buffer, 1, size - p, input);
                result = compress((char*)buffer, tp, &resultSize);
                for(int i = 0; i + 1 < resultSize; i+=2)
                {
                    printf("(%u|%u)", (unsigned int)result[i], (unsigned int)result[i+1]);
                }
                printf("\n\n");
                p += tp;
            }
            else
            {
                tp = fread(buffer, 1, 256, input);
                result = compress((char*)buffer, tp, &resultSize);
                p += tp;
            }
            if(result != 0 && resultSize > 0)
            {
                fwrite(result, 1, resultSize, output);
                free(result);
            }
        }
        free(buffer);
        fclose(input);
        fclose(output);
    }

    static void decompressFile(char* inputFile, char* outputFile, size_t* returnCompressedSize)
    {
        FILE* input = fopen(inputFile, "rb");
        FILE* output = fopen(outputFile, "wb");
        fseek(input, 0, SEEK_END);
        long size = ftell(input);
        fseek(input, 0, SEEK_SET);
        unsigned char* buffer = (unsigned char*)malloc(256);
        unsigned char* result = 0;
        size_t resultSize = 0;
        long p = 0;
        long tp;

        unsigned char* lastResult = (unsigned char*)malloc(512);
        size_t lastResultSize = 0;

        while (p < size)
        {
            if(size - p < 256)
            {
                memset(buffer, 0, 256);
                tp = fread(buffer, 1, size - p, input);
                for(int i = 0; i + 1 < tp; i+=2)
                {
                    printf("(%u|%u)", (unsigned int)buffer[i], (unsigned int)buffer[i+1]);
                }
                printf("\n");
                result = (unsigned char*)decompress((unsigned char*)buffer, tp, &resultSize, lastResult, lastResultSize);
                p += tp;
            }
            else
            {
                tp = fread(buffer, 1, 256, input);
                if(p > 0)
                {
                    result = (unsigned char*)decompress((unsigned char*)buffer, tp, &resultSize, lastResult, lastResultSize);
                    unsigned char* tmpAlloc = (unsigned char*)realloc(lastResult, resultSize);
                    if(tmpAlloc != 0)
                    {
                        lastResult = tmpAlloc;
                        lastResultSize = resultSize;
                    }
                    memcpy(lastResult, result, resultSize);
                }
                else
                {
                    result = (unsigned char*)decompress((unsigned char*)buffer, tp, &resultSize);
                }
                p += tp;
            }
            if(result != 0 && resultSize > 0)
            {
                fwrite(result, 1, resultSize, output);
                free(result);
            }
        }
        free(lastResult);
        free(buffer);
        fclose(input);
        fclose(output);
    }

    static char* decompress(unsigned char* data, size_t dataSize, size_t* returnSize)
    {

        char* result = (char*)calloc(dataSize*3, 1);
        size_t resSZ = dataSize*3;
        int rc = 0;
        unsigned char l = 0;
        short p = 0;
        unsigned char c = 0;
        for(int i = 0; i < dataSize; i+=2)
        {
            if( rc + dataSize >= resSZ)
            {
                char* tmpa = (char*)realloc(result, resSZ+dataSize);
                if(tmpa != NULL)
                {
                    result = tmpa;
                    resSZ += dataSize;
                }
                else
                {
                    printf("Allocation error!");
                }
            }

            if(data[i+1] == 0)
            {
                result[rc] = data[i];
                rc++;
            }
            else
            {
                p = (unsigned int)rc-(unsigned int)(data[i]);
                if(p < 0)
                {
                    printf("Error!----------------------------\n");
                }
                l = data[i+1];
                if( rc + dataSize + l >= resSZ)
                {
                    char* tmpa = (char*)realloc(result, resSZ+dataSize + l);
                    if(tmpa != NULL)
                    {
                        result = tmpa;
                        resSZ += dataSize + l;
                    }
                    else
                    {
                        printf("Allocation error!");
                    }
                }
                for(c=0; c < l; c++)
                {
                    result[rc] = result[p+c];
                    rc++;
                }
            }
        }
        if(resSZ > rc)
        {
            result = (char*)realloc(result, rc);
        }
        *returnSize = rc;
        return result;
    }

    static char* decompress(unsigned char* data, size_t dataSize, size_t* returnSize, unsigned char* lastResult, size_t lastResultSize)
    {

        char* result = (char*)calloc(dataSize*3, 1);
        size_t resSZ = dataSize*3;
        int rc = 0;
        unsigned char l = 0;
        short p = 0;
        unsigned char c = 0;
        for(int i = 0; i < dataSize; i+=2)
        {
            if( rc + dataSize >= resSZ)
            {
                char* tmpa = (char*)realloc(result, resSZ+dataSize);
                if(tmpa != NULL)
                {
                    result = tmpa;
                    resSZ += dataSize;
                }
                else
                {
                    printf("Allocation error!");
                }
            }

            if(data[i+1] == 0)
            {
                result[rc] = data[i];
                rc++;
            }
            else
            {
                p = (int)rc-(int)(data[i]);
                if(p < 0)
                {
                    l = data[i+1];
                    if( rc + dataSize + l >= resSZ)
                    {
                        char* tmpa = (char*)realloc(result, resSZ+dataSize + l);
                        if(tmpa != NULL)
                        {
                            result = tmpa;
                            resSZ += dataSize + l;
                        }
                        else
                        {
                            printf("Allocation error!");
                        }
                    }

                    for(c=0; c < l; c++)
                    {
                        if(p+c < 0)
                        {
                            result[rc] = lastResult[lastResultSize+p+c];
                        }
                        else
                        {
                            result[rc] = result[p+c];
                        }
                        rc++;
                    }

                }
                else
                {
                    l = data[i+1];
                    if( rc + dataSize + l >= resSZ)
                    {
                        char* tmpa = (char*)realloc(result, resSZ+dataSize + l);
                        if(tmpa != NULL)
                        {
                            result = tmpa;
                            resSZ += dataSize + l;
                        }
                        else
                        {
                            printf("Allocation error!");
                        }
                    }
                    for(c=0; c < l; c++)
                    {
                        result[rc] = result[p+c];
                        rc++;
                    }
                }
            }
        }
        if(resSZ > rc)
        {
            result = (char*)realloc(result, rc);
        }
        *returnSize = rc;
        return result;
    }

};
