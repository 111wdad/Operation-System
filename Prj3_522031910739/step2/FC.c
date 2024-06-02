#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "tcp_utils.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ServerAddr> <FSPort>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *server_addr = "localhost";
    int fs_port = atoi(argv[1]);
    tcp_client client = client_init(server_addr, fs_port);
    char buffer[4096];

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        if (feof(stdin)) break;
        client_send(client, buffer, strlen(buffer) + 1);
        int n = client_recv(client, buffer, sizeof(buffer));
        buffer[n] = 0;
        printf("%s\n", buffer);
        if (strcmp(buffer, "Bye!") == 0) break;
    }
    client_destroy(client);
    return 0;
}
