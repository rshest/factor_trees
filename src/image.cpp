#include <stdio.h>
#include "image.h"

//  Quick-and-hacky TGA image loader. Needed to load an OpenGL texture somehow, without 
//      using extra dependencies. Supports only uncompressed images.
bool LoadTGA(const char* fileName, Image& image)
{
    FILE* fp = fopen(fileName, "rb");
    if (!fp)
    {
        return false;
    }

    bool bOK = true;

    //  skip the start of the tga header
    fseek(fp, 12, SEEK_CUR);

    unsigned short  width;
    unsigned short  height;
    unsigned char   bitsPerPixel;

    size_t read_bytes = 0;
    read_bytes += fread(&width, sizeof(width), 1, fp);
    read_bytes += fread(&height, sizeof(height), 1, fp);
    read_bytes += fread(&bitsPerPixel, sizeof(bitsPerPixel), 1, fp);
    
    image.width  = (int)width;
    image.height = (int)height;

    // skip the image descriptor
    fseek(fp, 1, SEEK_CUR);  

    int bytesPerPixel   = bitsPerPixel/8;
    int numPixels       = width*height;
    int numBytes        = numPixels*bytesPerPixel;

    if (numBytes > 0)
    {
        if (bitsPerPixel == 24 || bitsPerPixel == 32)
        {
            std::vector<unsigned char> buf(numBytes);
            read_bytes += fread(&buf[0], 1, numBytes, fp);
            image.rgba.resize(numPixels);
            
            unsigned char* pBuf = &buf[0];
            for (int i = 0; i < numPixels; i++)
            {
                //  tga stores pixels in BGR(A) order
                unsigned int r = pBuf[2]; 
                unsigned int g = pBuf[1]; 
                unsigned int b = pBuf[0]; 
                unsigned int a = (bytesPerPixel == 3) ? 0xFF : pBuf[3];
                image.rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
                pBuf += bytesPerPixel;
            }
        }
        else
        {
            bOK = false;
        }
    }

    fclose(fp);
    return bOK;
}

bool SaveTGA(const char* fileName, const BGRA* bgra, unsigned short width, unsigned short height)
{    
    FILE* fp = fopen(fileName, "wb");
    if (!fp)
    {
        return false;
    }

    unsigned char hdr0[] = {0, 0, 2, 
                            0, 0, 0, 0, 0,
                            0, 0, 0, 0};
    fwrite(hdr0, sizeof(hdr0), 1, fp);

    unsigned char bitsPerPixel = 32;

    fwrite(&width, sizeof(width), 1, fp);
    fwrite(&height, sizeof(height), 1, fp);
    fwrite(&bitsPerPixel, sizeof(bitsPerPixel), 1, fp);

    unsigned char imgDescriptor = (1 << 5); // left-top origin
    fwrite(&imgDescriptor, sizeof(imgDescriptor), 1, fp);

    int bytesPerPixel   = bitsPerPixel/8;
    int numPixels       = width*height;
    int numBytes        = numPixels*bytesPerPixel;

    fwrite(bgra, 1, numBytes, fp);
    
    fclose(fp);
    
    return true;
}
