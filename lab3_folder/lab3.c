#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;

#pragma pack(push, 1)
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
#pragma pack(pop)

int main(int argc, char *argv[]) {

    char *image_file = argv[1];
    float angle = atof(argv[2]); 
    int num_processes = atoi(argv[3]);
    char *output_file = argv[4];

    int in = open(image_file, O_RDONLY);
    struct stat st;
    fstat(in, &st);
    size_t file_size = st.st_size;

    int out = open(output_file, O_RDWR | O_CREAT | O_TRUNC, 0666);
    ftruncate(out, file_size);

    unsigned char *map_in = mmap(NULL, file_size, PROT_READ, MAP_SHARED, in, 0);
    unsigned char *map_out = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, out, 0);

    struct tagBITMAPFILEHEADER *fileHeader = (struct tagBITMAPFILEHEADER *)map_in;
    struct tagBITMAPINFOHEADER *infoHeader = (struct tagBITMAPINFOHEADER *)(map_in + sizeof(struct tagBITMAPFILEHEADER));
    memcpy(map_out, map_in, fileHeader->bfOffBits);

    LONG width = infoHeader->biWidth;
    LONG height = infoHeader->biHeight;
    int rwb = (width * 3) + ((4 - ((width * 3) % 4)) % 4);

    unsigned char *data = map_in + fileHeader->bfOffBits;
    unsigned char *rdata = map_out + fileHeader->bfOffBits;

    for (int i = 0 ; i < num_processes; i++){
        pid_t pid = fork();

        if (pid < 0){
            return 1;
        } else if (pid == 0) {
            int lower = i * (width / num_processes);
            int upper;

            if (i == num_processes - 1) {
                upper = width;
            } else {
                upper = (i + 1) * (width / num_processes);
            }

            for (int xr = lower; xr < upper; xr++){
                for (int yr = 0; yr < height; yr++) {
                    int x, y;

                    float x_shift = xr - (width / 2.0);
                    float y_shift = yr - (height / 2.0);

                    float x_rot = x_shift * cos(-angle) - y_shift * sin(-angle);
                    float y_rot = x_shift * sin(-angle) + y_shift * cos(-angle);

                    x = (int)(x_rot + (width / 2.0));
                    y = (int)(y_rot + (height / 2.0));

                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        unsigned char B = data[x * 3 + y * rwb + 0];
                        unsigned char G = data[x * 3 + y * rwb + 1];
                        unsigned char R = data[x * 3 + y * rwb + 2];

                        rdata[xr * 3 + yr * rwb + 0] = B; 
                        rdata[xr * 3 + yr * rwb + 1] = G;
                        rdata[xr * 3 + yr * rwb + 2] = R;
                    } else {
                        rdata[xr * 3 + yr * rwb + 0] = 0;
                        rdata[xr * 3 + yr * rwb + 1] = 0;
                        rdata[xr * 3 + yr * rwb + 2] = 0;
                    }
                }
            } exit(0);
            
        } else {
            continue;
        }

    }
    for (int i = 0; i < num_processes; i++) {
        wait(0);
    }

    munmap(map_in, file_size);
    munmap(map_out, file_size);
    close(in);
    close(out);

    return 0;
}