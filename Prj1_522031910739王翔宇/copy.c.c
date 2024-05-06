#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <source> <destination> <buffer size>\n", argv[0]);
        return 1;
    }

    int bufferSize = atoi(argv[3]);
    char *buffer = (char *)malloc(bufferSize * sizeof(char));
    if (!buffer) {
        perror("Memory allocation failed");
        return 1;
    }

    int srcFd = open(argv[1], O_RDONLY);
    if (srcFd < 0) {
        perror("Failed to open source file");
        free(buffer);
        return 1;
    }

    int destFd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destFd < 0) {
        perror("Failed to open destination file");
        close(srcFd);
        free(buffer);
        return 1;
    }

    ssize_t bytesRead;
    while ((bytesRead = read(srcFd, buffer, bufferSize)) > 0) {
        write(destFd, buffer, bytesRead);
    }

    close(srcFd);
    close(destFd);
    free(buffer);
    return 0;
}
