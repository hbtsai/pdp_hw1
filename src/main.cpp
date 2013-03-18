#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <climits>
#include <omp.h>
#include <time.h>
#include "bmp.h"

using namespace std;

unsigned char** convertToGrayscale(Pixel**, int, int);
unsigned char** shiftImage(unsigned char**, int, int, int, int);
unsigned char findMedian(unsigned char**, int, int);
int unsignedCharCompare(const void*, const void*);
unsigned char* convertToBitimage(unsigned char**, int, int, unsigned char, int*);
void setZero(unsigned char*, int);
void setOne(unsigned char*, int);
unsigned char* xorBitimages(unsigned char*, unsigned char*, int);
int countOnes(unsigned char*, int);
void generateCountTable(int*);
static int countTable[256];

/// main function of median threshold bitmap alignment algorithm
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        cout << "usage: align <input_img1> <input_img2>" << endl;
        exit(-1);
    }

    generateCountTable(countTable);
    char* imageFile1 = argv[1];
    char* imageFile2 = argv[2];
    FILE* inputImage1 = fopen(imageFile1, "rb");
    FILE* inputImage2 = fopen(imageFile2, "rb");

    /// read bmp header
    BMP_header bmpHeader1, bmpHeader2;
    Pixel** image1, ** image2;
	
    image1 = readBMP(inputImage1, &bmpHeader1);
    image2 = readBMP(inputImage2, &bmpHeader2);
    fclose(inputImage1);
    fclose(inputImage2);
    
    int width1, height1, width2, height2;
    width1 = bmpHeader1.width;
    height1 = bmpHeader1.height;
    width2 = bmpHeader2.width;
    height2 = bmpHeader2.height;

    if(width1 != width2 || height1 != height2)
    {
        cout << "error: images given are not of the same size" << endl;
        exit(-1);
    }
    
    int& width = width1;
    int& height = height1;
    cout << "image width = " << width << "\timage height = " << height << endl;

    /// grayscale image
    unsigned char** grayscale1, ** grayscale2;
    grayscale1 = convertToGrayscale(image1, width, height);
    grayscale2 = convertToGrayscale(image2, width, height);

    /// median
    unsigned char median1 = findMedian(grayscale1, height, width);
    unsigned char median2 = findMedian(grayscale2, height, width);

    /// trying all offsets
    int difference;
    int minDifference = INT_MAX;
    int xoffset = 0, yoffset = 0;
    int byteNum;
    unsigned char** shiftedGrayscale;
    unsigned char* medianBitmap1, * medianBitmap2;
    unsigned char* differenceBitmap;
	int a[width1*height1];
	int b[width1*height1];
	int c[width1*height1];
    
    /// for each kind of offset (x and y offset)
#pragma omp parallel
{
#pragma omp for collapse(2) private(shiftedGrayscale, medianBitmap1, medianBitmap2, differenceBitmap, difference, byteNum) firstprivate(grayscale1, grayscale2, width1, height1, median1, median2)
    for(int x = -width1/2; x < width1/2; ++x)
    {
    for(int y = -height1/2; y < height1/2; ++y)
    {
        shiftedGrayscale = shiftImage(grayscale1, x, y, width1, height1);
        medianBitmap1 = convertToBitimage(shiftedGrayscale, height1, width1, median1, &byteNum);
        medianBitmap2 = convertToBitimage(grayscale2,       height1, width1, median2, &byteNum);

        differenceBitmap = xorBitimages(medianBitmap1, medianBitmap2, byteNum);
        difference = countOnes(differenceBitmap, byteNum);

		a[(x+width1/2)*height1+(y+height1/2)] = difference;
		b[(x+width1/2)*height1+(y+height1/2)] = x;
		c[(x+width1/2)*height1+(y+height1/2)] = y;
        
		/*
        if(difference < minDifference)
        {
            minDifference = difference;
            xoffset = x;
            yoffset = y;
        }
		*/

        /// delete dynamic array allocated by functions
        for(int h = 0; h < height1; ++h)
        {
            delete[] shiftedGrayscale[h];
        }
        delete[] shiftedGrayscale;
        delete[] medianBitmap1;
        delete[] medianBitmap2;
        delete[] differenceBitmap;
    }
    }
}

int min =INT_MAX ;
int idx =INT_MAX ;
int l;

for(l = 0; l<width1*height1; l++)
{
	if(a[l]<min)
	{
		min=a[l];
		idx=l;
	}

}
	minDifference = a[idx];
	xoffset = b[idx];
	yoffset = c[idx];

    cout << "alignment offset x = " << xoffset << ", y = " << yoffset << endl;
    cout << "minDifference = " << minDifference << endl;

    /// deleting other pointers
    for(int h = 0; h < height; ++h)
    {
        delete[] image1[h];
        delete[] image2[h];
        delete[] grayscale1[h];
        delete[] grayscale2[h];
    }
    delete[] image1;
    delete[] image2;
    delete[] grayscale1;
    delete[] grayscale2;

    return 0;
}

/// shift the given image by xoffset in x, yoffset in y
/// the pixel assignment will not wrap around
unsigned char** shiftImage(unsigned char** grayscaleImage, int xoffset, int yoffset, int width, int height)
{
    unsigned char** shiftedGrayscale = new unsigned char*[height];

    for(int h = 0; h < height; ++h)
    {
        shiftedGrayscale[h] = new unsigned char[width];
    }

    int x, y;
#pragma omp parallel 
{
#pragma omp for collapse(2) private(x,y) firstprivate(height, width, shiftedGrayscale, grayscaleImage, xoffset, yoffset)
    for(int h = 0; h < height; ++h)
    {
        for(int w = 0; w < width; ++w)
        {
            x = w - xoffset;
            y = h - yoffset;

            x = (x < 0)? 0 : x;
            x = (x >= width)? width-1 : x;
            y = (y < 0)? 0 : y;
            y = (y >= height)? height-1 : y;

            if(x >= 0 && x < width && y >= 0 && y < height)
            {
                shiftedGrayscale[h][w] = grayscaleImage[y][x];
            }
        }
    }
}

    return shiftedGrayscale;
}

/// convert color pixel vector to grascale pixel vector
unsigned char** convertToGrayscale(Pixel** inputImage, int width, int height)
{
    unsigned char** grayscale = new unsigned char*[height];
    for(int h = 0; h < height; ++h)
    {
        grayscale[h] = new unsigned char[width];
    }

#pragma omp parallel
{
#pragma omp for collapse(2)	firstprivate(height, width, inputImage, grayscale) 
    for(int h = 0; h < height; ++h)
    {
    for(int w = 0; w < width; ++w)
    {
        /// the graylevel of a rgb pixel is (54*R + 183*G + 19*B)/256 
//        int graylevel = 
        //graylevel /= 256;
        //assert(graylevel >= 0 && graylevel <= 255);
        grayscale[h][w] = (unsigned char )((54 * inputImage[h][w].R + 183 * inputImage[h][w].G + 19 * inputImage[h][w].B)/256); //(unsigned char)graylevel;
    }
    }

}
    return grayscale;
}

/// find the median of all pixel value in a grayscale image
unsigned char findMedian(unsigned char** grayscaleImage, int height, int width)
{
    unsigned char* copyImage = new unsigned char[height * width];
    for(int h = 0; h < height; ++h)
    {
        memcpy(copyImage + h * width, grayscaleImage[h], width);
    }
    qsort(copyImage, height * width, sizeof(unsigned char), unsignedCharCompare);

    int median = copyImage[height * width / 2];
    delete[] copyImage;

    return median;
}

/// the compare function defined for qsort
int unsignedCharCompare(const void* ucp1, const void* ucp2)
{
    if( *(unsigned char*)ucp1 <  *(unsigned char*)ucp2 ) 
        return -1;
    if( *(unsigned char*)ucp1 == *(unsigned char*)ucp2 )
        return 0;
    if( *(unsigned char*)ucp1 >  *(unsigned char*)ucp2 )
        return 1;

    assert(false);
    return 0;
}

/// convert grayscale vector to bit vector with defined threshold
/// the byteNum is taken as output
unsigned char* convertToBitimage(unsigned char** grayscaleImage, 
    int height, int width, unsigned char threshold, int* _byteNum)
{
    int byteNum = (height * width % 8 == 0? height * width/8 : height * width/8 + 1);

    unsigned char* bitImage = new unsigned char[byteNum];
	unsigned char orPattern[8]={0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001};
	unsigned char andPattern[8]={0b01111111, 0b10111111, 0b11011111, 0b11101111, 0b11110111, 0b11111011, 0b11111101, 0b11111110};

#pragma omp parallel 
{
#pragma omp for collapse(2) firstprivate(height, width, grayscaleImage, threshold, bitImage)
    for(int h = 0; h < height; ++h)
    {
    for(int w = 0; w < width; ++w)
    {
        if(grayscaleImage[h][w] < threshold)
		{
			bitImage[(h * width + w)/8] &= andPattern[(h * width + w)%8];
		}
       //     setZero(bitImage, h * width + w);
        if(grayscaleImage[h][w] >= threshold)
		{
			bitImage[(h * width + w)/8] |= orPattern[(h * width + w)%8];
		}
       //     setOne(bitImage, h * width + w);
    }
    }

}
    *_byteNum = byteNum;
    return bitImage;
}

/// set the value of specific index of a bit vector to one
void setOne(unsigned char* bitImage, int index)
{
	
    int byteIndex = index / 8;
    int bitOffset = index % 8;
    unsigned char orPattern = 0b00000000;

    switch(bitOffset)
    {
        case 0:
        orPattern = 0b10000000;
        break;
        case 1:
        orPattern = 0b01000000;
        break;
        case 2:
        orPattern = 0b00100000;
        break;
        case 3:
        orPattern = 0b00010000;
        break;
        case 4:
        orPattern = 0b00001000;
        break;
        case 5:
        orPattern = 0b00000100;
        break;
        case 6:
        orPattern = 0b00000010;
        break;
        case 7:
        orPattern = 0b00000001;
        break;
        default:
        assert(false);
    }

    bitImage[byteIndex] |= orPattern;

    return;
}

/// set the value of specific index of a bit vector to zero
void setZero(unsigned char* bitImage, int index)
{
    int byteIndex = index / 8;
    int bitOffset = index % 8;
    unsigned char andPattern = 0b00000000;

    switch(bitOffset)
    {
        case 0:
        andPattern = 0b01111111;
        break;
        case 1:
        andPattern = 0b10111111;
        break;
        case 2:
        andPattern = 0b11011111;
        break;
        case 3:
        andPattern = 0b11101111;
        break;
        case 4:
        andPattern = 0b11110111;
        break;
        case 5:
        andPattern = 0b11111011;
        break;
        case 6:
        andPattern = 0b11111101;
        break;
        case 7:
        andPattern = 0b11111110;
        break;
        default:
        assert(false);
    }

    bitImage[byteIndex] &= andPattern;

    return;
}

/// xor two bit vector, this is taken as the difference of two image
/// that is, the heuristic function to minimize in alignment algorithm
unsigned char* xorBitimages(unsigned char* bitimage1, unsigned char* bitimage2, int byteNum)
{
    unsigned char* outputBitimage = new unsigned char[byteNum];
//#pragma omp parallel for firstprivate(byteNum, outputBitimage, bitimage1, bitimage2)
    for(int i = 0; i < byteNum; ++i)
    {
        outputBitimage[i] = bitimage1[i] ^ bitimage2[i];
    }

    return outputBitimage;
}

/// count the number of ones in the bit vector
int countOnes(unsigned char* bitimage, int byteNum)
{
    int count = 0;
#pragma omp parallel 
{
#pragma omp for reduction(+:count) firstprivate(byteNum, bitimage, countTable)
    for(int i = 0; i < byteNum; ++i)
    {
        count += countTable[bitimage[i]];
    }
}

    return count;
}

/// the count table contain the mapping from byte(0--255) 
/// to the number of ones in that byte
void generateCountTable(int* countTable)
{
    for(int i = 0; i < 256; ++i)
    {
		int count = 0;	
        for(int j = 0; j < 8; ++j)
        {
            count += (i >> j) & 1;
        }

        countTable[i] = count;
    }

    return;
}

