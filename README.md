# C-playground

A from-scratch reimplementation of core Unix/POSIX tools and standard library functions in C — built one utility at a time to understand what actually happens under the hood, not just to use them.

The goal isn't production-ready tools. It's understanding the underlying syscalls and mechanisms: how a directory is really read, how `stat` gives you everything in an `ls -l` row, how `malloc` actually manages memory, how `cat` streams bytes from a file descriptor. Each tool here is a small, focused deep-dive.

**Status:** Active — `ls` implemented (`c-ls/`), more utilities in progress.

---

## Why This Repo Exists

Using `ls`, `cat`, `malloc`, or `strlen` every day doesn't teach you how they work. Rebuilding them does. This repo is a running log of that process — each subfolder is one utility or library function, built from the relevant syscalls or primitives up, with notes on what I learned along the way.

## Philosophy

- No copying implementations — everything here is written from reading man pages, POSIX docs, and occasionally a video for when stuck (see `c-ls/README.md` for full attribution style).
- Each tool starts minimal and grows in scope over time (e.g. `ls` → `-a`, `-l`, then sorting/alignment).
- Every folder documents what's implemented, what's missing, and what I learned — this matters more than shipping a "finished" tool.

## Implemented

| Tool | Folder | What it covers |
|---|---|---|
| `ls` | [`c-ls/`](./c-ls) | `opendir`/`readdir`, `lstat`, `struct stat`, permission bitmasks, file type flags, `getpwuid`/`getgrgid`, `getopt` |

## Planned

| Tool | What it'll cover |
|---|---|
| `malloc` / `free` | Heap management, `brk`/`sbrk` or `mmap`, free-list allocators, fragmentation |
| `string.h` functions (`strlen`, `strcpy`, `strcmp`, etc.) | Pointer arithmetic, buffer safety, how the standard library implements these without "magic" |
| `cat` | File descriptors, `open`/`read`/`write`, buffered vs unbuffered I/O |
| More coreutils (`wc`, `cp`, `mv`, or similar) | To be decided as I go |

This list will grow — the point is depth on a few tools, not rushing through all of them.

## Repo Structure

```
C-playground/
├── c-ls/              # ls implementation + its own README
├── (future: c-malloc/, c-string/, c-cat/, ...)
├── LICENSE
└── README.md          # this file
```

Each tool lives in its own folder with its own `README.md` covering build/run instructions, what's implemented, and design notes specific to that tool. This top-level README stays high-level and tracks overall progress across the repo.

## Build

Each tool is self-contained and builds independently. See the individual folder's README for exact build/run commands, e.g.:

```bash
cd c-ls
gcc -Wall -Wextra -o lsbtw ls.c
./lsbtw -la /etc
```

## Contributing

This is primarily a personal learning log, but suggestions, corrections, and PRs (especially pointing out bugs in the systems-level reasoning) are welcome.

## License

MIT
