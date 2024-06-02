# Project 1: File Copy and Sorting Analysis

This project includes implementations for file copying and sorting algorithms. The file copying is demonstrated through direct copying, using `fork()`, and inter-process communication via pipes. The sorting analysis compares the performance between single-threaded and multi-threaded merge sort algorithms.

## File Structure

The project is organized as follows:


## Compilation

To compile all the executables, run the following command at the root of the project directory:

```bash
make
./Copy <source> <destination> <buffer_size>
./ForkCopy <source> <destination> <buffer_size>
./PipeCopy <source> <destination> <buffer_szie>
./MergesortSingle < data.in
./MergesortMulti < data.in
./shell
```
# Project 3: Multi-User File System
This project implements a simple multi-user file system in C. It allows multiple users to create, read, write, and delete files, while enforcing basic file permissions.

## Features

User authentication: Users can create accounts and log in with a username and password.
File management: Users can create, read, write, and delete files in their own directories.
File permissions: Each file has owner permissions (read, write, execute) that control access by other users.
Session management: Users are assigned session IDs upon login to track their interactions with the system.

## Usage
Compile the code: make

Run the program:

./BDS <DiskFileName> <#cylinders> <#sectors per cylinder> <track-to-track delay> <port>

./BDC <DiskServerAddress> <port> `

./FS <DiskServerAddress> <BDSPort> <FSPort>

./FC <ServerAddr> <FSPort>

./Multi```
