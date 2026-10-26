#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

struct tagBITMAPFILEHEADER //16 bytes with padding
{
WORD bfType; //specifies the file type
DWORD bfSize; //specifies the size in bytes of the bitmap file
WORD bfReserved1; //reserved; must be 0
WORD bfReserved2; //reserved; must be 0
DWORD bfOffBits; //species the offset in bytes from the bitmapfileheader to the bitmap bits
};

struct tagBITMAPINFOHEADER //40 bytes no padding
{
DWORD biSize; //specifies the number of bytes required by the struct
LONG biWidth; //specifies width in pixels
LONG biHeight; //species height in pixels
WORD biPlanes; //specifies the number of color planes, must be 1
WORD biBitCount; //specifies the number of bit per pixel
DWORD biCompression;//spcifies the type of compression
DWORD biSizeImage; //size of image in bytes
LONG biXPelsPerMeter; //number of pixels per meter in x axis
LONG biYPelsPerMeter; //number of pixels per meter in y axis
DWORD biClrUsed; //number of colors used by th ebitmap
DWORD biClrImportant; //number of colors that are important
};

int main(int argc, char *argv[]){

    FILE *in = fopen(argv[1], "rb");
    FILE *out = fopen(argv[2], "wb");

    struct tagBITMAPFILEHEADER fileHeader;
    struct tagBITMAPINFOHEADER infoHeader;

    //read the file header variable by variable to bypass reading padded bytes
    fread(&fileHeader.bfType, sizeof(WORD), 1, in);
    fread(&fileHeader.bfSize, sizeof(DWORD), 1, in);
    fread(&fileHeader.bfReserved1, sizeof(WORD), 1, in);
    fread(&fileHeader.bfReserved2, sizeof(WORD), 1, in);
    fread(&fileHeader.bfOffBits, sizeof(DWORD), 1, in);

    //already 4 byte aligned
    fread(&infoHeader, sizeof(struct tagBITMAPINFOHEADER), 1, in);

    int row = infoHeader.biWidth * 3;
    int rem = row % 4;
    int padding = (4 - rem) % 4;

    unsigned char *data = (unsigned char *)malloc(infoHeader.biWidth * infoHeader.biHeight * 3);

    //read through each row fseek skips junk bytes after fread finishes
    for (int i=0; i < infoHeader.biHeight; i++){
        fread(data + (i * row), 3, infoHeader.biWidth, in);
        fseek(in, padding, SEEK_CUR);
    }

    char *operation = argv[3];
    float factor = atof(argv[4]);
    int pixels = infoHeader.biWidth * infoHeader.biHeight;

    for (int i =0; i < pixels * 3; i+=3){
        float b = data[i] / 255.0;
        float g = data[i+1] / 255.0;
        float r = data[i+2] / 255.0;

        if (strcmp(operation, "contrast") == 0) {
            b = pow(b, factor);
            g = pow(g, factor);
            r = pow(r, factor);
        } else if (strcmp(operation, "lightness") == 0) {
            b = b + factor;
            g = g + factor;
            r = r + factor;
        } else if (strcmp(operation, "saturation") == 0) {
            double avg = (b + g + r) / 3.0;
            b = b + (b - avg) * factor;
            g = g + (g - avg) * factor;
            r = r + (r - avg) * factor;
        }

        if (b > 1.0){
            b = 1.0;
        } else if (b < 0.0){
            b = 0.0;
        }

        if (g > 1.0){
            g = 1.0;
        } else if (g < 0.0){
            g = 0.0;
        }

        if (r > 1.0){
            r = 1.0;
        } else if (r < 0.0){
            r = 0.0;
        }

        data[i] = (unsigned char)(b * 255.0);
        data[i + 1] = (unsigned char)(g * 255.0);
        data[i + 2] = (unsigned char)(r * 255.0);
    }

    fwrite(&fileHeader.bfType, sizeof(WORD), 1, out);
    fwrite(&fileHeader.bfSize, sizeof(DWORD), 1, out);
    fwrite(&fileHeader.bfReserved1, sizeof(WORD), 1, out);
    fwrite(&fileHeader.bfReserved2, sizeof(WORD), 1, out);
    fwrite(&fileHeader.bfOffBits, sizeof(DWORD), 1, out);

    fwrite(&infoHeader, sizeof(struct tagBITMAPINFOHEADER), 1, out);

    // re-add padding
    unsigned char pad[3] = {0, 0, 0};
    for (int i = 0; i < infoHeader.biHeight; i++){
        fwrite(data + (i * row), 3, infoHeader.biWidth, out);
        fwrite(pad, 1, padding, out); 
    }

    free(data);
    fclose(in);
    fclose(out);

    return 0;
}