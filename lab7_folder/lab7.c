#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

void lock(int processNum, int *flags, int *turn) {
    flags[processNum] = 1;
    *turn = 1 - processNum;
    
    while (flags[1-processNum] == 1 && *turn == 1-processNum) {}
    
}

void unlock(int processNum, int *flags)
{
    flags[processNum] = 0;
}

void countFiles(char *path, int processNum, int *fileCount, int *dirCount, int *flags, int *turn) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(path);

    if (dir == NULL) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char buffer[4096];
        strcpy(buffer, path);
        strcat(buffer, "/");
        strcat(buffer, entry->d_name);

        struct stat sb;

        if (stat(buffer, &sb) == -1) {
            continue;
        }
        
        if (S_ISDIR(sb.st_mode)) {
            lock(processNum, flags, turn);

            (*dirCount)++;

            unlock(processNum, flags);

            countFiles(buffer, processNum, fileCount, dirCount, flags, turn);

        }

        else {
            lock(processNum, flags, turn);

            (*fileCount)++;

            unlock(processNum, flags);
        }
    }

    closedir(dir);
}

void splitDir(char *path, int processNum, int *fileCount, int *dirCount, int *flags, int *turn) {
    DIR *dir;
    dir = opendir(path);
    struct dirent *entry;

    if (dir == NULL) {
        return;
    }

    int index = 0;

    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (index % 2 == processNum) {
            char buffer[4096];
            strcpy(buffer, path);
            strcat(buffer, "/");
            strcat(buffer, entry->d_name);

            struct stat sb;

            if (stat(buffer, &sb) == 0) {
                if (S_ISDIR(sb.st_mode)) {
                    lock(processNum, flags, turn);

                    (*dirCount)++;

                    unlock(processNum, flags);

                    countFiles(buffer, processNum, fileCount, dirCount, flags, turn);
                }
                else {
                    lock(processNum, flags, turn);

                    (*fileCount)++;

                    unlock(processNum, flags);
                }
            }
        }
        index++;
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    char *start = ".";

    if (argc > 1)
    {
        start = argv[1];
    }

    int *fileCount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *dirCount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *flags = mmap(NULL, sizeof(int) * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int *turn = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *fileCount = 0;
    *dirCount = 0;
    flags[0] = 0;
    flags[1] = 0;
    *turn = 0;

    pid_t pid = fork();

    if (pid < 0) {
        return 1;
    }
    else if (pid == 0) {
        splitDir(start, 1, fileCount, dirCount, flags, turn);
        munmap(fileCount, sizeof(int));
        munmap(dirCount, sizeof(int));
        munmap(flags, sizeof(int)*2);
        munmap(turn, sizeof(int));

        return 0;
    }
    else {
        splitDir(start, 0, fileCount, dirCount, flags, turn);
        wait(0);

        printf("Files: %d\n", *fileCount);
        printf( "Subdirectories: %d\n", *dirCount);

        munmap(fileCount, sizeof(int));
        munmap(dirCount, sizeof(int));
        munmap(flags, sizeof(int)*2);
        munmap(turn, sizeof(int));
    }
    return 0;
}