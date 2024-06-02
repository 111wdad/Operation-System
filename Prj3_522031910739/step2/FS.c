#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#define BLOCK_SIZE 256
#define MAX_FILENAME_LENGTH 8
#define MAX_PATH_LENGTH 1024
#define MAX_USERS 100
#define MAX_FILES_PER_DIRECTORY 100
#define MAX_FILE_SIZE 16384
#define INODES_PER_BLOCK (BLOCK_SIZE / sizeof(inode))
#define POINTERS_PER_INODE (BLOCK_SIZE / sizeof(int))
#define MAX_BLOCKS (MAX_FILE_SIZE / BLOCK_SIZE)
#define SUPERBLOCK_SIZE 1
#define INODE_BLOCK_SIZE 1
#define USER_BLOCK_SIZE 1
#define DATA_BLOCK_SIZE 1
#define INODE_DIRECT_POINTERS 10
#define INODE_INDIRECT_POINTERS 1

// Structure for inode
typedef struct {
    int file_size;
    int direct_blocks[INODE_DIRECT_POINTERS];
    int indirect_block;
    int is_directory;
    char filename[MAX_FILENAME_LENGTH];
    time_t last_modified;
} inode;

// Structure for user
typedef struct {
    char username[32];
    char password[32];
    int home_directory;
} user;

// Structure for directory entry
typedef struct {
    char name[MAX_FILENAME_LENGTH];
    int inode_number;
} directory_entry;

// Structure for file system
typedef struct {
    inode inodes[MAX_FILES_PER_DIRECTORY];
    user users[MAX_USERS];
    directory_entry root_directory[MAX_FILES_PER_DIRECTORY];
    int free_blocks[MAX_BLOCKS];
    int num_free_blocks;
} file_system;

// Global variables
file_system *fs;
int fs_socket;
int disk_socket;
struct sockaddr_in fs_addr;
struct sockaddr_in disk_addr;
char disk_server_address[256];
int disk_server_port;
int fs_port;
int current_directory_inode = 0;  // Start at root directory

// Function prototypes
void init_file_system();
void handle_client(int client_socket);
void process_command(char *command, int client_socket);
void format_file_system();
void create_file(char *filename);
void create_directory(char *dirname);
void delete_file(char *filename);
void change_directory(char *path);
void list_directory(int client_socket);
void read_file(char *filename, int client_socket);
void write_file(char *filename, char *data, int length);
void insert_file(char *filename, int pos, char *data, int length);
void delete_data_in_file(char *filename, int pos, int length);
void exit_file_system();
void connect_to_disk_server();
void send_to_disk(char *command, char *response);
int allocate_block();
void free_block(int block_number);
int find_free_inode();
int find_file_inode(char *filename);
void read_inode(int inode_number, inode *node);
void write_inode(int inode_number, inode *node);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <DiskServerAddress> <BDSPort> <FSPort>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(disk_server_address, argv[1]);
    disk_server_port = atoi(argv[2]);
    fs_port = atoi(argv[3]);

    // Initialize file system
    init_file_system();

    // Connect to disk server
    connect_to_disk_server();

    // Set up file system server socket
    fs_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (fs_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    fs_addr.sin_family = AF_INET;
    fs_addr.sin_addr.s_addr = INADDR_ANY;
    fs_addr.sin_port = htons(fs_port);

    if (bind(fs_socket, (struct sockaddr *)&fs_addr, sizeof(fs_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(fs_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("File system server listening on port %d\n", fs_port);

    while (1) {
        int client_socket = accept(fs_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        handle_client(client_socket);
        close(client_socket);
    }

    close(fs_socket);
    close(disk_socket);

    return 0;
}

void init_file_system() {
    fs = malloc(sizeof(file_system));
    memset(fs, 0, sizeof(file_system));
    fs->num_free_blocks = MAX_BLOCKS;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        fs->free_blocks[i] = 1;
    }
}

void handle_client(int client_socket) {
    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }

        process_command(buffer, client_socket);
    }
}

void process_command(char *command, int client_socket) {
    char response[1024];
    memset(response, 0, sizeof(response));

    // Example of processing a command
    if (strncmp(command, "f", 1) == 0) {
        format_file_system();
        strcpy(response, "File system formatted\n");
    } else if (strncmp(command, "mk ", 3) == 0) {
        char filename[MAX_FILENAME_LENGTH];
        sscanf(command + 3, "%s", filename);
        create_file(filename);
        snprintf(response, sizeof(response), "File %s created\n", filename);
    } else if (strncmp(command, "mkdir ", 6) == 0) {
        char dirname[MAX_FILENAME_LENGTH];
        sscanf(command + 6, "%s", dirname);
        create_directory(dirname);
        snprintf(response, sizeof(response), "Directory %s created\n", dirname);
    } else if (strncmp(command, "rm ", 3) == 0) {
        char filename[MAX_FILENAME_LENGTH];
        sscanf(command + 3, "%s", filename);
        delete_file(filename);
        snprintf(response, sizeof(response), "File %s deleted\n", filename);
    } else if (strncmp(command, "cd ", 3) == 0) {
        char path[MAX_PATH_LENGTH];
        sscanf(command + 3, "%s", path);
        change_directory(path);
        snprintf(response, sizeof(response), "Changed directory to %s\n", path);
    } else if (strncmp(command, "ls", 2) == 0) {
        list_directory(client_socket);
        return;  // `list_directory` will send the response
    } else if (strncmp(command, "cat ", 4) == 0) {
        char filename[MAX_FILENAME_LENGTH];
        sscanf(command + 4, "%s", filename);
        read_file(filename, client_socket);
        return;  // `read_file` will send the response
    } else if (strncmp(command, "w ", 2) == 0) {
        char filename[MAX_FILENAME_LENGTH];
        char data[MAX_FILE_SIZE];
        int length;
        sscanf(command + 2, "%s %d %s", filename, &length, data);
        write_file(filename, data, length);
        snprintf(response, sizeof(response), "Wrote %d bytes to file %s\n", length, filename);
    } else if (strncmp(command, "i ", 2) == 0) {
        char filename[MAX_FILENAME_LENGTH];
        char data[MAX_FILE_SIZE];
        int pos, length;
        sscanf(command + 2, "%s %d %d %s", filename, &pos, &length, data);
        insert_file(filename, pos, data, length);
        snprintf(response, sizeof(response), "Inserted %d bytes at position %d in file %s\n", length, pos, filename);
    } else if (strncmp(command, "d ", 2) == 0) {
        char filename[MAX_FILENAME_LENGTH];
        int pos, length;
        sscanf(command + 2, "%s %d %d", filename, &pos, &length);
        delete_data_in_file(filename, pos, length);
        snprintf(response, sizeof(response), "Deleted %d bytes from position %d in file %s\n", length, pos, filename);
    } else if (strncmp(command, "e", 1) == 0) {
        exit_file_system();
        strcpy(response, "Exiting file system\n");
    }

    send(client_socket, response, strlen(response), 0);
}

void format_file_system() {
    memset(fs, 0, sizeof(file_system));
    fs->num_free_blocks = MAX_BLOCKS;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        fs->free_blocks[i] = 1;
    }
}

void create_file(char *filename) {
    int inode_number = find_free_inode();
    if (inode_number < 0) {
        printf("No free inode available\n");
        return;
    }

    inode *node = &fs->inodes[inode_number];
    memset(node, 0, sizeof(inode));
    node->file_size = 0;
    strncpy(node->filename, filename, MAX_FILENAME_LENGTH);
    node->is_directory = 0;
    node->    last_modified = time(NULL);

    // Add to current directory
    for (int i = 0; i < MAX_FILES_PER_DIRECTORY; i++) {
        if (fs->root_directory[i].inode_number == -1) {
            fs->root_directory[i].inode_number = inode_number;
            strncpy(fs->root_directory[i].name, filename, MAX_FILENAME_LENGTH);
            break;
        }
    }

    write_inode(inode_number, node);
}

void create_directory(char *dirname) {
    int inode_number = find_free_inode();
    if (inode_number < 0) {
        printf("No free inode available\n");
        return;
    }

    inode *node = &fs->inodes[inode_number];
    memset(node, 0, sizeof(inode));
    node->file_size = 0;
    strncpy(node->filename, dirname, MAX_FILENAME_LENGTH);
    node->is_directory = 1;
    node->last_modified = time(NULL);

    // Add to current directory
    for (int i = 0; i < MAX_FILES_PER_DIRECTORY; i++) {
        if (fs->root_directory[i].inode_number == -1) {
            fs->root_directory[i].inode_number = inode_number;
            strncpy(fs->root_directory[i].name, dirname, MAX_FILENAME_LENGTH);
            break;
        }
    }

    write_inode(inode_number, node);
}

void delete_file(char *filename) {
    int inode_number = find_file_inode(filename);
    if (inode_number < 0) {
        printf("File not found\n");
        return;
    }

    inode *node = &fs->inodes[inode_number];

    // Free allocated blocks
    for (int i = 0; i < INODE_DIRECT_POINTERS; i++) {
        if (node->direct_blocks[i] != -1) {
            free_block(node->direct_blocks[i]);
        }
    }

    // Free indirect blocks
    if (node->indirect_block != -1) {
        int *indirect_blocks = malloc(BLOCK_SIZE);
        send_to_disk("R %d %d", node->indirect_block / fs->cylinders, node->indirect_block % fs->cylinders, (char *)indirect_blocks);
        for (int i = 0; i < POINTERS_PER_INODE; i++) {
            if (indirect_blocks[i] != -1) {
                free_block(indirect_blocks[i]);
            }
        }
        free_block(node->indirect_block);
        free(indirect_blocks);
    }

    // Clear inode
    memset(node, 0, sizeof(inode));
    node->file_size = 0;
    node->is_directory = 0;
    write_inode(inode_number, node);

    // Remove from directory
    for (int i = 0; i < MAX_FILES_PER_DIRECTORY; i++) {
        if (fs->root_directory[i].inode_number == inode_number) {
            fs->root_directory[i].inode_number = -1;
            memset(fs->root_directory[i].name, 0, MAX_FILENAME_LENGTH);
            break;
        }
    }
}

void change_directory(char *path) {
    if (strcmp(path, "/") == 0) {
        current_directory_inode = 0;
        return;
    }

    int inode_number = find_file_inode(path);
    if (inode_number < 0 || !fs->inodes[inode_number].is_directory) {
        printf("Directory not found\n");
        return;
    }

    current_directory_inode = inode_number;
}

void list_directory(int client_socket) {
    char response[4096];
    memset(response, 0, sizeof(response));

    for (int i = 0; i < MAX_FILES_PER_DIRECTORY; i++) {
        if (fs->root_directory[i].inode_number != -1) {
            inode *node = &fs->inodes[fs->root_directory[i].inode_number];
            char entry[128];
            snprintf(entry, sizeof(entry), "%s\t%s\t%ld\n",
                     node->filename,
                     node->is_directory ? "DIR" : "FILE",
                     node->last_modified);
            strcat(response, entry);
        }
    }

    send(client_socket, response, strlen(response), 0);
}

void read_file(char *filename, int client_socket) {
    int inode_number = find_file_inode(filename);
    if (inode_number < 0) {
        send(client_socket, "File not found\n", 15, 0);
        return;
    }

    inode *node = &fs->inodes[inode_number];
    char response[MAX_FILE_SIZE];
    memset(response, 0, sizeof(response));

    // Read direct blocks
    for (int i = 0; i < INODE_DIRECT_POINTERS; i++) {
        if (node->direct_blocks[i] != -1) {
            send_to_disk("R %d %d", node->direct_blocks[i] / fs->cylinders, node->direct_blocks[i] % fs->cylinders, response + i * BLOCK_SIZE);
        }
    }

    // Read indirect blocks
    if (node->indirect_block != -1) {
        int *indirect_blocks = malloc(BLOCK_SIZE);
        send_to_disk("R %d %d", node->indirect_block / fs->cylinders, node->indirect_block % fs->cylinders, (char *)indirect_blocks);
        for (int i = 0; i < POINTERS_PER_INODE; i++) {
            if (indirect_blocks[i] != -1) {
                send_to_disk("R %d %d", indirect_blocks[i] / fs->cylinders, indirect_blocks[i] % fs->cylinders, response + (INODE_DIRECT_POINTERS + i) * BLOCK_SIZE);
            }
        }
        free(indirect_blocks);
    }

    send(client_socket, response, node->file_size, 0);
}

void write_file(char *filename, char *data, int length) {
    int inode_number = find_file_inode(filename);
    if (inode_number < 0) {
        create_file(filename);
        inode_number = find_file_inode(filename);
    }

    inode *node = &fs->inodes[inode_number];

    // Write direct blocks
    for (int i = 0; i < INODE_DIRECT_POINTERS && length > 0; i++) {
        if (node->direct_blocks[i] == -1) {
            node->direct_blocks[i] = allocate_block();
        }
        int bytes_to_write = length < BLOCK_SIZE ? length : BLOCK_SIZE;
        send_to_disk("W %d %d %d %s", node->direct_blocks[i] / fs->cylinders, node->direct_blocks[i] % fs->cylinders, bytes_to_write, data + i * BLOCK_SIZE);
        length -= bytes_to_write;
    }

    // Write indirect blocks
    if (length > 0) {
        if (node->indirect_block == -1) {
            node->indirect_block = allocate_block();
        }
        int *indirect_blocks = malloc(BLOCK_SIZE);
        send_to_disk("R %d %d", node->indirect_block / fs->cylinders, node->indirect_block % fs->cylinders, (char *)indirect_blocks);
        for (int i = 0; i < POINTERS_PER_INODE && length > 0; i++) {
            if (indirect_blocks[i] == -1) {
                indirect_blocks[i] = allocate_block();
            }
            int bytes_to_write = length < BLOCK_SIZE ? length : BLOCK_SIZE;
            send_to_disk("W %d %d %d %s", indirect_blocks[i] / fs->cylinders, indirect_blocks[i] % fs->cylinders, bytes_to_write, data + (INODE_DIRECT_POINTERS + i) * BLOCK_SIZE);
            length -= bytes_to_write;
        }
        send_to_disk("W %d %d %d %s", node->indirect_block / fs->cylinders, node->indirect_block % fs->cylinders, BLOCK_SIZE, (char *)indirect_blocks);
        free(indirect_blocks);
    }

    node->file_size += length;
    node->last_modified = time(NULL);
    write_inode(inode_number, node);
}

void insert_file(char *filename, int pos, char *data, int length) {
     int inode_number = find_file_inode(filename);
    if (inode_number < 0) {
        printf("File not found\n");
        return;
    }

    inode *node = &fs->inodes[inode_number];
    if (pos > node->file_size) {
        printf("Position out of bounds\n");
        return;
    }

    char buffer[MAX_FILE_SIZE];
    read_file_into_buffer(node, buffer);

    // Move existing data after position to create space for new data
    memmove(buffer + pos + length, buffer + pos, node->file_size - pos);
    memcpy(buffer + pos, data, length);
    node->file_size += length;

    // Write the updated buffer back to the file
    write_buffer_to_file(node, buffer, node->file_size);
    node->last_modified = time(NULL);
    write_inode(inode_number, node);
}

void delete_data_in_file(char *filename, int pos, int length) {
     int inode_number = find_file_inode(filename);
    if (inode_number < 0) {
        printf("File not found\n");
        return;
    }

    inode *node = &fs->inodes[inode_number];
    if (pos + length > node->file_size) {
        printf("Delete range out of bounds\n");
        return;
    }

    char buffer[MAX_FILE_SIZE];
    read_file_into_buffer(node, buffer);

    // Move data after position + length to position
    memmove(buffer + pos, buffer + pos + length, node->file_size - (pos + length));
    node->file_size -= length;

    // Write the updated buffer back to the file
    write_buffer_to_file(node, buffer, node->file_size);
    node->last_modified = time(NULL);
    write_inode(inode_number, node);
}

void exit_file_system() {
     save_file_system_state();
    close_connection_to_disk_server();
    free_allocated_memory();

void connect_to_disk_server() {
    disk_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (disk_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    disk_addr.sin_family = AF_INET;
    disk_addr.sin_port = htons(disk_server_port);
    inet_pton(AF_INET, disk_server_address, &disk_addr.sin_addr);

    if (connect(disk_socket, (struct sockaddr *)&disk_addr, sizeof(disk_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void send_to_disk(char *command, char *response) {
    send(disk_socket, command, strlen(command), 0);
    recv(disk_socket, response, 1024, 0);
}

int allocate_block() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (fs->free_blocks[i]) {
            fs->free_blocks[i] = 0;
            fs->num_free_blocks--;
          return i;
        }
    }
    return -1;  // No free blocks available
}

void free_block(int block_number) {
    fs->free_blocks[block_number] = 1;
    fs->num_free_blocks++;
}

int find_free_inode() {
    for (int i = 0; i < MAX_FILES_PER_DIRECTORY; i++) {
        if (fs->inodes[i].file_size == 0 && fs->inodes[i].filename[0] == '\0') {
            return i;
        }
    }
    return -1;  // No free inodes available
}

int find_file_inode(char *filename) {
    for (int i = 0; i < MAX_FILES_PER_DIRECTORY; i++) {
        if (strcmp(fs->root_directory[i].name, filename) == 0) {
            return fs->root_directory[i].inode_number;
        }
    }
    return -1;  // File not found
}

void read_inode(int inode_number, inode *node) {
    memcpy(node, &fs->inodes[inode_number], sizeof(inode));
}

void write_inode(int inode_number, inode *node) {
    memcpy(&fs->inodes[inode_number], node, sizeof(inode));
}