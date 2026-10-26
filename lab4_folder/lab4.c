#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

void sighandler(int sig) {
    write(STDOUT_FILENO, "no activity detected!", 21);
    exit(0);
}

int main()
{
    time_t *last_activity = mmap(NULL, sizeof(time_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *last_activity = time(NULL);
    signal(SIGUSR1, sighandler);

    pid_t parent_pid = getpid();
    pid_t pid = fork();

    if (pid < 0) {
        return 1;
    } else if (pid == 0) {
        while(1) {
            if (time(NULL) - *last_activity >= 10) {
                kill(parent_pid, SIGUSR1);
                exit(0);     
            }
        }
    } else {
        char buffer[4096];

        while(1) {
            int bytes = read(STDIN_FILENO, buffer, sizeof(buffer));

            if (bytes <= 0) {
                break;
            }

            *last_activity = time(NULL);
            write(STDOUT_FILENO, "!", 2);

            if (buffer[bytes - 1] == '\n') {
                write(STDOUT_FILENO, buffer, bytes - 1);
                write(STDOUT_FILENO, "!\n", 3);
            } else {
                write(STDOUT_FILENO, buffer, bytes);
                write(STDOUT_FILENO, "!\n", 3);
            }
        } 
        kill(pid, SIGTERM);
        wait(0);
        munmap(last_activity, sizeof(time_t));
    }
    return 0;
}