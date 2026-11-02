#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

#pragma pack(push, 1)
struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
};

struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
};
#pragma pack(pop)

int main(int argc, char *argv[]) {

    FILE *file = fopen(argv[1], "rb");

    struct tagBITMAPFILEHEADER fileHeader;
    struct tagBITMAPINFOHEADER infoHeader;

    fread(&fileHeader, sizeof(fileHeader), 1, file);
    fread(&infoHeader, sizeof(infoHeader), 1, file);

    int width = infoHeader.biWidth;
    int height = infoHeader.biHeight;

    int padding = (4 - (width*3) % 4) % 4;
    int row_size = width * 3 + padding;
    int imageSize = row_size * height;

    unsigned char *data = malloc(imageSize);
    fseek(file, fileHeader.bfOffBits, SEEK_SET);
    fread(data, 1, imageSize, file);
    fclose(file);


    int fd = -1;

    while (fd<0) {
        fd = shm_open("/sharedMem", O_RDWR, 0777);
        if (fd<0) {
            usleep(10000);
        }
    }

    int ready = -1;
    
    while (ready<0) {
        ready = shm_open("/image_ready", O_RDWR, 0777);
        if (ready < 0) {
            usleep(10000);
        }
    }

    close(ready);

    unsigned char *result = mmap(
        NULL,
        imageSize,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
    0
    );

    int Mx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int My[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    for (int y=1; y<height/2; y++) {
        for (int x=1; x<width-1; x++) {
            int Gx = 0;
            int Gy = 0;

            for (int i=0; i<3; i++) {
                for (int j=0; j< 3; j++) {

                    int index = (x+j-1)*3 + (y+i-1)*row_size;

                    int brightness = (data[index] + data[index+1] + data[index+2]) / 3;
                    Gx += brightness * Mx[i][j];
                    Gy += brightness * My[i][j];
                }
            }

            int edge = sqrt(Gx*Gx + Gy*Gy);

            if (edge > 255) {
                edge = 255;
            }

            int index = y * row_size + x * 3;

            result[index] = edge;
            result[index+1] = edge;
            result[index+2] = edge;
        }
    }

    int done = shm_open("/image_done", O_RDWR | O_CREAT, 0777);
    close(done);

    free(data);
    munmap(result, imageSize);
    close(fd);

    return 0;
}