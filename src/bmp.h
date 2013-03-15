#ifndef BMP_H_
#define BMP_H_
#include <stdio.h>
#include <stdlib.h>
typedef struct _BMP_header
{
    unsigned short magicno;
    unsigned int filesize;
    unsigned int reserved;
    unsigned int offset;
    unsigned int dib_header_size;
    unsigned int width;
    unsigned int height;
    unsigned short nColorPlanes;
    unsigned short nBits;
    unsigned int BI_RGB;
    unsigned int raw_size;
    unsigned int resolutionH;
    unsigned int resolutionV;
    unsigned int nColors;
    unsigned int imporColor;
}BMP_header;

typedef struct _Pixel
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
}Pixel;


Pixel** readBMP(FILE*,BMP_header*);
void writeBMP(FILE*,FILE*,BMP_header,Pixel*);

#endif //BMP_H_
