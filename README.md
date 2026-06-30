# c-ls

A from-scratch implementation of the Unix `ls` command in C.

## Features

- Directory traversal with opendir/readdir
- Long listing (-l)
- Hidden files (-a)
- Permission formatting
- User/group lookup
- Date formatting
- Command-line parsing with getopt

## Concepts Learned

- POSIX APIs
- Directory streams
- File metadata (stat/lstat)
- Bitwise operations
- Command-line argument parsing
- Resource management

## Build

make

## Run

./c-ls
./c-ls -a
./c-ls -l
./c-ls -la
