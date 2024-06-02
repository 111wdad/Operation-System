# Multi-User File System

This project implements a simple multi-user file system in C. It allows multiple users to create, read, write, and delete files, while enforcing basic file permissions.

## Features

- User authentication: Users can create accounts and log in with a username and password.
- File management: Users can create, read, write, and delete files in their own directories.
- File permissions: Each file has owner permissions (read, write, execute) that control access by other users.
- Session management: Users are assigned session IDs upon login to track their interactions with the system.

## Usage

1. Compile the code: `make`

2. Run the program:

3.  `./BDS <DiskFileName> <#cylinders> <#sectors per cylinder> <track-to-track delay> <port>`

    `./BDC <DiskServerAddress> <port>` `

    `./FS <DiskServerAddress> <BDSPort> <FSPort>`

    `./FC <ServerAddr> <FSPort>`

   `./Multi`

## Tips

Since I wasn't able to implement the project properly, the teacher doesn't need to spend time checking if the code runs correctly.