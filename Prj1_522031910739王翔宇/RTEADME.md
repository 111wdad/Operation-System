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