#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <source> <destination> <buffer size>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        // Fork失败
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // 子进程
        // 这里调用execl执行MyCopy程序，将缓冲区大小作为参数传递
        if (execl("./copy", "copy", argv[1], argv[2], argv[3], (char *)NULL) == -1) {
            // 如果execl失败
            perror("execl failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // 父进程
        // 等待子进程完成
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Copy completed successfully.\n");
        } else {
            fprintf(stderr, "Copy failed.\n");
        }
    }

    return 0;
}
