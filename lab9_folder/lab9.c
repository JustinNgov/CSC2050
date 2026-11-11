#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

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

float weights[5] = {
    0.204164f,
    0.180174f,
    0.123832f,
    0.066283f,
    0.027631f
};

int width, height, row_size, blurFactor;

unsigned char *data_1;
unsigned char *data_2;

atomic_int counter;
pthread_barrier_t barrier;

void *worker(void *arg) {
    int id = *(int *)arg;
    for (int t=0; t< blurFactor; t++) {
        while(1) {
            int y = atomic_fetch_add(&counter, 1);

            if (y >= height) {
                break;
            }

            for (int x=4; x < width - 4; x++) {
                int index = y * row_size + x*3;

                for (int c = 0; c < 3; c++) {
                    float sum = 0;

                    for (int k=-4; k <= 4; k++) {
                        int neighbor = y * row_size + (x+k) * 3;

                        int distance = k;

                        if (distance < 0) {
                            distance = -distance;
                        }

                        sum += data_1[neighbor + c] * weights[distance];
                    }
                     if (sum>254.5f) {
                        sum=254.5f;
                    }
                    
                    data_2[index + c] = (unsigned char)(sum + 0.5f);
                }
            }
        }
        pthread_barrier_wait(&barrier);

        if (id==0) {
            atomic_store(&counter, 0);
        }

        pthread_barrier_wait(&barrier);

        while(1) {
            int x = atomic_fetch_add(&counter, 1);

            if (x >= width) {
                break;
            }

            if (x < 4 || x >= width - 4) {
                continue;
            }

            for (int y=4; y < height - 4; y++) {
                int index = y * row_size + x*3;

                for (int c = 0; c < 3; c++) {
                    float sum = 0;

                    for (int k=-4; k <= 4; k++) {
                        int neighbor = (y+k) * row_size + x * 3;

                        int distance = k;

                        if (distance < 0) {
                            distance = -distance;
                        }

                        sum += data_2[neighbor + c] * weights[distance];
                    }
                    
                    if (sum>254.5f) {
                        sum=254.5f;
                    }

                    data_1[index + c] = (unsigned char)(sum + 0.5f);
                }
            }
        }
        pthread_barrier_wait(&barrier);

        if (id==0) {
            atomic_store(&counter, 0);
        }

        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    blurFactor = atoi(argv[3]);
    
    FILE *file = fopen(argv[1], "rb");

    struct tagBITMAPFILEHEADER fileHeader;
    struct tagBITMAPINFOHEADER infoHeader;

    fread(&fileHeader, sizeof(fileHeader), 1, file);
    fread(&infoHeader, sizeof(infoHeader), 1, file);

    width = infoHeader.biWidth;
    height = infoHeader.biHeight;

    int padding = (4 - (width*3) % 4) % 4;
    row_size = width*3 + padding;
    int imageSize = row_size * height;

    data_1 = malloc(imageSize);
    data_2 = malloc(imageSize);

    fseek(file, fileHeader.bfOffBits, SEEK_SET);
    fread(data_1, 1, imageSize, file);
    fclose(file);

    memcpy(data_2, data_1, imageSize);

    atomic_init(&counter, 0);

    pthread_barrier_init(&barrier, NULL, 4);
    pthread_t threads[4];
    int ids[4];

    for (int i=0; i<4; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    FILE *out = fopen(argv[2], "wb");

    fwrite(&fileHeader, sizeof(fileHeader), 1, out);
    fwrite(&infoHeader, sizeof(infoHeader), 1, out);
    fwrite(data_1, 1, imageSize, out);

    fclose(out);

    free(data_1);
    free(data_2);

    return 0;
}