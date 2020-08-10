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
        char* compressed = (char*)calloc(2*dataSize, 1);
        int ci = 0;
        int pos;
        int pos2;
        int pc = 0;
        bool brk = false;
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
            pc = 255;
            if(pc > dataSize)
            {
                pc = dataSize;
            }
            brk = true;
            
            pc = 1;
            for(u = 0; u < i; u++)
            {
                pos2 = findInStr(data, dataSize, data+i, i-u);
                if(pos2 != -1 && pos2 < i)
                {
                    //printf("Found %.*s in %.*s\n", i-u, data+i, dataSize, data);
                    pc = i-u;
                    i+= i-u-1;
                    break;
                }
            }
            compressed[ci] = pos2;
            ci++;
            compressed[ci] = pc;
            ci++;
        }
        compressed = (char*)realloc(compressed, ci);
        *returnCompressedSize = ci;
        return (unsigned char*)compressed;
    }

    static char* decompress(unsigned char* data, size_t dataSize, size_t* returnSize)
    {

        char* result = (char*)calloc(dataSize*2, 1);
        int rc = 0;
        unsigned char l = 0;
        unsigned char p = 0;
        unsigned char c = 0;
        for(int i = 0; i < dataSize; i+=2)
        {
            if(data[i+1] == 0)
            {
                result[rc] = data[i];
                rc++;
            }
            else
            {
                p = data[i];
                l = data[i+1];
                for(c=0; c < l; c++)
                {
                    result[rc] = result[p+c];
                    rc++;
                }
            }
        }
        *returnSize = rc;
        return result;
    }
};
