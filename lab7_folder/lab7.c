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

        char *buffer[4096];
        strcpy(buffer, path);
        strcat(buffer, "/");
        strcat(buffer, entry->d_name);

        struct stat sb;

        if (stat(buffer, &sb) == -1) {
            continue;
        }
        
        if (S_ISDIR(sb.st_mode)) {
            lock(processNum, flags, turn);

            *dirCount++;

            unlock(processNum, flags);

            countFiles(buffer, processNum, fileCount, dirCount, flags, turn);

        }

        else {
            lock(processNum, flags, turn);

            *fileCount++;

            unlock(processNum, flags);
        }
    }

    closedir(dir);
}

void splitDir(char *path, int processNum, int *fileCount, int *dirCount, int *flags, int *turn) {
    DIR *dir;
    struct dirent *entry;
    
}