#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int fd[2];
int dupStdIn;

void sighandler(int sig) {
    dup2(fd[0], STDIN_FILENO);
}

int main(int argc, char const *argv[])
{
    time_t *last_activity = mmap(NULL, sizeof(time_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *last_activity = time(NULL);

    pipe(fd);
    dupStdIn = dup(STDIN_FILENO);
    signal(SIGUSR1, sighandler);

    pid_t ppid = getpid();
    pid_t pid = fork();

    if (pid < 0) { 
        return -1;
    } else if (pid == 0) {
        close(fd[0]);

        while (1) {
            sleep(10);

            if (time(NULL) - *last_activity >= 10) {
                write(fd[1], "no activity detected!\n", 22);

                kill(ppid, SIGUSR1);
                
                *last_activity = time(NULL);
            }

        }
    } else {

        close(fd[1]);
        char buffer[100];

        while(1) {
            int bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

            if (bytes < 0)
            {
                continue;
            }

            if (bytes == 0)
            {
                break;
            }

            buffer[bytes] = '\0';

            if (strcmp(buffer, "no activity detected!\n") == 0) {
                write(STDOUT_FILENO, buffer, bytes);
                dup2(dupStdIn, STDIN_FILENO);
                continue;
            }

            *last_activity = time(NULL);

            
            if (strcmp(buffer, "quit\n") == 0 || strcmp(buffer, "quit") == 0) {
                break;
            }

            write(STDOUT_FILENO, "!", 1);

            if (buffer[bytes - 1] == '\n') {
                write(STDOUT_FILENO, buffer, bytes - 1);
                write(STDOUT_FILENO, "!\n", 2);
            } else {
                write(STDOUT_FILENO, buffer, bytes);
                write(STDOUT_FILENO, "!\n", 2);
            }
        }
        kill(pid, SIGTERM);
        wait(0);
        munmap(last_activity, sizeof(time_t));

        close(fd[0]);
        close(dupStdIn);

    }

    return 0;
}
