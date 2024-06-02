#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 100
#define MAX_FILES_PER_USER 100
#define MAX_FILENAME_LENGTH 100
#define MAX_CONTENT_LENGTH 1000

typedef struct {
    char username[MAX_FILENAME_LENGTH];
    char password[MAX_FILENAME_LENGTH];
    int session_id;
} User;

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    int owner_id;
    int permissions; // e.g., read, write, execute
} File;

User users[MAX_USERS];
int num_users = 0;

File user_files[MAX_USERS][MAX_FILES_PER_USER];
int num_files[MAX_USERS] = {0};

int current_session_id = 1;

int authenticate_user(char *username, char *password) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            users[i].session_id = current_session_id++;
            return users[i].session_id;
        }
    }
    return -1; // Authentication failed
}

void create_user(char *username, char *password) {
    if (num_users < MAX_USERS) {
        strcpy(users[num_users].username, username);
        strcpy(users[num_users].password, password);
        users[num_users].session_id = 0; // Set session ID to 0 initially
        num_users++;
        printf("User %s created successfully.\n", username);
    } else {
        printf("Maximum number of users reached.\n");
    }
}

void create_file(char *username, char *filename, char *content) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (num_files[i] < MAX_FILES_PER_USER) {
                strcpy(user_files[i][num_files[i]].filename, filename);
                strcpy(user_files[i][num_files[i]].content, content);
                user_files[i][num_files[i]].owner_id = i;
                user_files[i][num_files[i]].permissions = 0; // Set default permissions
                num_files[i]++;
                printf("File %s created for user %s.\n", filename, username);
                return;
            } else {
                printf("User %s has reached the maximum number of files.\n", username);
                return;
            }
        }
    }
    printf("User %s not found.\n", username);
}

void main() {
    // Example usage
    create_user("user1", "password1");
    create_user("user2", "password2");

    int session_id = authenticate_user("user1", "password1");
    if (session_id != -1) {
        printf("User authenticated with session ID: %d\n", session_id);
        create_file("user1", "file1.txt", "This is a test file.");
    } else {
        printf("Authentication failed.\n");
    }
}
