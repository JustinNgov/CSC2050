#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>

int processNum;

void matrix_multiply(int rowsA, int colsA, int colsB, float A[rowsA][colsA],float B[colsA][colsB], float C[rowsA][colsB]) {
    int rowsPerProcess = rowsA/4;
    int start = processNum * rowsPerProcess;
    int end = start + rowsPerProcess;

    for (int i=start; i<end; i++) {
        for (int j=0; j<colsB; j++) {
            for (int k=0; k<colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void barrier(int process_number, int *bi, int no) {
    bi[process_number]++;

    while(1)
    {
        int sum = 0;

        for(int i = 0; i < 4; i++)
            sum += bi[i];

        if(sum >= 4 * no)
            break;
    }
}

int main() {
    float (*A)[100] = mmap(NULL,
         sizeof(float) * 100 * 100,
         PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    float (*B)[100] = mmap(NULL,
         sizeof(float) * 100 * 100,
         PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    float (*M)[100] = mmap(NULL,
         sizeof(float) * 100 * 100,
         PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    int *barrierArray = mmap(
        NULL,
        sizeof(int) * 4,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );

    srand(time(0));

    for (int i=0; i<100; i++) {
        for (int j=0; j<100; j++) {
            A[i][j] = (float) rand() / RAND_MAX;
            B[i][j] = (float) rand() / RAND_MAX;
            M[i][j] = 0;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        barrierArray[i] = 0;
    }

    for (int i=0; i<4; i++) {
        pid_t pid = fork();
        
        if (pid<0) {
            return 1;
        }

        else if (pid==0) {
            processNum = i;

            matrix_multiply(100, 100, 100, A, B, M);

            barrier(processNum, barrierArray, 1);

            int start = processNum * 25;
            int end = start + 25;

            for (int i=start; i<end; i++) {
                for (int j=0; j<100; j++) {
                    A[i][j] = 0;
                }
            }

            matrix_multiply(100, 100, 100, M, B, A);

            barrier(processNum, barrierArray, 2);

            if (processNum==0) {
                printf("A[40][40] = %f\n", A[40][40]);
            }

            munmap(A, sizeof(float) * 100 * 100);
            munmap(B, sizeof(float) * 100 * 100);
            munmap(M, sizeof(float) * 100 * 100);
            munmap(barrierArray, sizeof(int)*4);

        return 0;
        };

    }
    
    for (int i=0; i<4; i++) {
        wait(0);
    }

    munmap(A, sizeof(float) * 100 * 100);
    munmap(B, sizeof(float) * 100 * 100);
    munmap(M, sizeof(float) * 100 * 100);
    munmap(barrierArray, sizeof(int)*4);

    return 0;

}