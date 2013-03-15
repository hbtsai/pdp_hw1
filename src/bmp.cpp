#include <cstdlib>
#include "bmp.h"

unsigned short readshort(unsigned char *input)
{
    unsigned short output = 0;
    int i;
    for(i=0;i<2;i++)
    {
	output += *(input-i+1);
	if(i!=1)
	    output = output << 8;
    }
    return output;
}

unsigned int readint(unsigned char *input)
{
    unsigned int output = 0;
    int i;
    for(i=0;i<4;i++)
    {
	output += *(input-i+3);
	if(i!=3)
	    output = output << 8;
    }
    return output;
}

Pixel** readBMP(FILE* fp,BMP_header* bmpheader)
{
    unsigned char header[54];
    fread(header,sizeof(unsigned char),54,fp);

    bmpheader->magicno = readshort(header);
    bmpheader->filesize = readint(header+2);
    bmpheader->reserved = readint(header+6);
    bmpheader->offset = readint(header+10);
    bmpheader->dib_header_size = readint(header+14);
    bmpheader->width = readint(header+18);
    bmpheader->height = readint(header+22);
    bmpheader->nColorPlanes = readshort(header+26);
    bmpheader->nBits = readshort(header+28);
    bmpheader->BI_RGB = readint(header+30);
    bmpheader->raw_size = readint(header+34);
    bmpheader->resolutionH = readint(header+38);
    bmpheader->resolutionV = readint(header+42);
    bmpheader->nColors = readint(header+46);
    bmpheader->imporColor = readint(header+50);

    const int& height = (int)(bmpheader->height);
    const int& width  = (int)(bmpheader->width);

    // two dimensional image 
    Pixel** pixel = new Pixel*[height];
    for(int h = 0; h < height; ++h)
    {
        pixel[h] = new Pixel[width];
    }

    int nPadding = (bmpheader->width*3/4+1)*4 - bmpheader->width*3;
    if(nPadding == 4)
    {
        nPadding = 0;
    }
    unsigned char *padding = (unsigned char*)malloc(sizeof(unsigned char)*nPadding);
    
    for(int h = 0; h < height; ++h)
    {
        fread(pixel[h] , sizeof(Pixel), bmpheader->width, fp);
        if(nPadding != 0)
            fread(padding,sizeof(unsigned char),nPadding,fp);
    }

    return pixel;
}

void writeBMP(FILE* frp,FILE* fop,BMP_header bmpheader,Pixel* filtered)
{
    unsigned char originheader[54];
    fseek(frp,0,SEEK_SET);
    fread(originheader,sizeof(unsigned char),54,frp);
    fwrite(originheader,sizeof(unsigned char),54,fop);
    
    int nPadding = (bmpheader.width*3/4+1)*4 - bmpheader.width*3;
    if(nPadding == 4)
	nPadding = 0;
    unsigned char *padding = (unsigned char*)malloc(sizeof(unsigned char)*nPadding);
    int i;
    for(i=0;i<nPadding;i++)
	padding[i] = 0;
    for(i=0;i<(int)bmpheader.height;i++)
    {
	fwrite(filtered+(i*bmpheader.width),sizeof(Pixel),bmpheader.width,fop);
	if(nPadding!=0)
	    fwrite(padding,sizeof(unsigned char),nPadding,fop);
    }
}
