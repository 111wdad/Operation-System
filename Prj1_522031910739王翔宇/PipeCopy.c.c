#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

void readFromSourceAndWriteToPipe(int sourceFd, int pipeWriteEnd, int bufferSize) {
    char *buffer = (char *)malloc(bufferSize);
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        exit(EXIT_FAILURE);
    }

    ssize_t bytesRead;
    while ((bytesRead = read(sourceFd, buffer, bufferSize)) > 0) {
        write(pipeWriteEnd, buffer, bytesRead);
    }

    free(buffer);
    close(sourceFd);
    close(pipeWriteEnd);
}

void readFromPipeAndWriteToDestination(int pipeReadEnd, int destFd, int bufferSize) {
    char *buffer = (char *)malloc(bufferSize);
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        exit(EXIT_FAILURE);
    }

    ssize_t bytesRead;
    while ((bytesRead = read(pipeReadEnd, buffer, bufferSize)) > 0) {
        write(destFd, buffer, bytesRead);
    }

    free(buffer);
    close(pipeReadEnd);
    close(destFd);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <source> <destination> <buffer size>\n", argv[0]);
        return 1;
    }

    int bufferSize = atoi(argv[3]);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        close(pipefd[1]); // Close unused write end
        int destFd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (destFd == -1) {
            perror("Failed to open destination file");
            exit(EXIT_FAILURE);
        }
        readFromPipeAndWriteToDestination(pipefd[0], destFd, bufferSize);
    } else {
        // Parent process
        close(pipefd[0]); // Close unused read end
        int srcFd = open(argv[1], O_RDONLY);
        if (srcFd == -1) {
            perror("Failed to open source file");
            exit(EXIT_FAILURE);
        }
        readFromSourceAndWriteToPipe(srcFd, pipefd[1], bufferSize);
        wait(NULL); // Wait for child process to finish
    }

    return 0;
}
