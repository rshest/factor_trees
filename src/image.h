#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <vector>

struct Image
{
    std::vector<unsigned int> rgba;   // pixel array
    int width, height;
};

struct BGRA { unsigned char b, g, r, a; };

bool LoadTGA(const char* fileName, Image& image);
bool SaveTGA(const char* fileName, const BGRA* bgra, unsigned short width, unsigned short height);

#endif // __IMAGE_H__
