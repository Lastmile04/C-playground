# Major Libraries and Elements

## Libraries Used

- **A macro for the POSIX.1-2008 standard API** → `#define _POSIX_C_SOURCE 200809L`
  - Without this macro, typedefs like `mode_t`, functions like `lstat`, and fields like `st_mtim.tv_sec` won't be exposed by the system headers.
  - `mode_t` is basically an unsigned integer type (defined via `sys/stat.h`) used to represent a filesystem object's type and permissions.
  - `lstat` is a function that takes a file path and the address of a `struct stat`. It asks the kernel for metadata about the filesystem object and stores that metadata in the `struct stat` (this is why it needs the POSIX macro to be visible).
    - `struct stat` is a data structure from `sys/stat.h` that stores metadata about a single filesystem object (type, permissions, owner, size, timestamps, etc.).

- **Standard input/output library** → `stdio.h`
  - Used for `printf`, `fprintf`, `snprintf`, and other formatted I/O.

- **Directory reading** → `dirent.h`
  - `opendir` opens a directory stream for a given path.
  - `readdir` returns one `struct dirent` per call, each representing one entry (file/subdirectory) inside that directory.
  - `closedir` releases the directory stream.

- **Command-line option parsing** → `getopt.h`
  - `getopt` walks `argv` and recognizes single-character flags (here, `-a` and `-l`), including combined forms like `-la`.
  - `optind` tracks the index of the first non-option argument (used to detect an optional path argument after the flags).

- **POSIX system calls** → `unistd.h`
  - Pulled in for POSIX-level declarations used alongside `sys/stat.h`.

- **File metadata** → `sys/stat.h`
  - Defines `struct stat`, `mode_t`, `lstat`, and all the `S_IS*` / `S_I*` macros used below.

- **User and group lookups** → `pwd.h` and `grp.h`
  - `getpwuid` converts a numeric `st_uid` into a `struct passwd`, from which the username (`pw_name`) is read.
  - `getgrgid` converts a numeric `st_gid` into a `struct group`, from which the group name (`gr_name`) is read.

- **Time formatting** → `time.h`
  - `localtime` converts the raw `time_t` seconds (`st_mtim.tv_sec`) into a broken-down `struct tm`.
  - `strftime` formats that `struct tm` into a human-readable string (e.g. `"Jul  9 14:32"`).

---

## Understanding `st_mode` and Permission Bitmasks

One concept that finally clicked for me while implementing `ls -l` was how Unix stores file metadata.

When we call:

```c
struct stat st;
lstat(path, &st);
```

the kernel fills the `struct stat` with metadata about the filesystem object.

One of its members is:

```c
st.st_mode
```

`st_mode` is of type `mode_t` (a typedef, usually an `unsigned int`).

It is **not just a number** — it is a **bitmask** where different groups of bits represent different pieces of information.

```text
                st_mode (32-bit integer)

┌──────────────────────────────────────────────────────┐
│ Reserved │ File Type │ Special │ Permission Bits     │
└──────────────────────────────────────────────────────┘
                                           │
                                           └──────────────┐
                                                          ▼
                                              Owner  Group Others
                                                rwx    rwx    rwx
```

---

### Why do bitwise operations work?

POSIX defines a standard layout for these bits.

The C library exposes named macros for these predefined bit patterns.

For example:

```c
S_IRUSR    // Owner can read
S_IWUSR    // Owner can write
S_IXUSR    // Owner can execute
```

Internally these are simply bitmasks.

Conceptually:

```text
mode

101101101

S_IRUSR

100000000

AND

101101101
100000000
---------
100000000   (non-zero → permission exists)
```

If the result is **non-zero**, that permission bit was set.

If the result is **0**, the permission is absent.

This is why code like

```c
mode & S_IRUSR
```

works.

---

### File Type

`st_mode` also stores the type of the filesystem object.

Instead of manually checking those bits, POSIX provides helper macros:

```c
S_ISDIR(mode)
S_ISREG(mode)
S_ISLNK(mode)
S_ISSOCK(mode)
```

These macros internally perform a bitmask and compare the extracted bits against the POSIX-defined file type values.

Conceptually:

```text
st_mode
│
├── File Type ─────► S_ISDIR(), S_ISREG(), ...
│
└── Permissions ───► S_IRUSR, S_IWUSR, S_IXUSR, ...
```

---

### Biggest Takeaway

`st_mode` is a packed integer containing multiple pieces of filesystem metadata.

The kernel, POSIX standard, C library, and my program all agree on the meaning of each bit.

The permission string:

```text
-rwxr-xr--
```

is simply a human-readable representation of selected bits inside that integer.

---

## Walking Through the Code

### `mode_string()` — decoding `st_mode` into a display string

```c
void mode_string(mode_t mode, char *str){
    if (S_ISDIR(mode))         str[0] = 'd';
    else if (S_ISLNK(mode))    str[0] = 'l';
    else if (S_ISCHR(mode))    str[0] = 'c';
    else if (S_ISBLK(mode))    str[0] = 'b';
    else if (S_ISFIFO(mode))   str[0] = 'p';
    else if (S_ISSOCK(mode))   str[0] = 's';
    else                       str[0] = '-';
    ...
}
```

- `mode` is the raw integer bitmask returned by the kernel via `lstat`.
- The `S_IS*` macros are checked in sequence to determine the file type character shown in column 1 of `ls -l` output (`d` for directory, `l` for symlink, `c`/`b` for device files, `p` for FIFO, `s` for socket, `-` for a regular file).
- The remaining 9 characters (`str[1]` through `str[9]`) are filled using bitwise `&` against the `S_I{R,W,X}{USR,GRP,OTH}` macros, producing the familiar `rwxr-xr--` permission string.
- `str[10]` is set to `'\0'` to null-terminate the 10-character string (`-rwxr-xr--`).

### `print_long()` — assembling one `ls -l` row

```c
void print_long(const char *dir, const char *name){
    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);

    struct stat st;
    if(lstat(fullpath, &st) < 0){
        perror(name);
        return;
    }
    ...
}
```

- Builds the full path (`dir/name`) since `lstat` needs a path relative to where the program is run, not just the bare entry name from `readdir`.
- `lstat` (not `stat`) is used deliberately — `lstat` reports on the symlink itself rather than following it, which matters for showing `l` correctly for symlinks instead of silently resolving them.
- On failure (e.g. a broken symlink or permission error), `perror` prints a diagnostic and the function returns early rather than crashing.
- `getpwuid(st.st_uid)` and `getgrgid(st.st_gid)` translate the raw numeric owner/group IDs stored in `struct stat` into human-readable names, falling back to `"?"` if the lookup fails (e.g. a UID with no matching passwd entry).
- `localtime` + `strftime` convert `st.st_mtim.tv_sec` (seconds since epoch) into the `"%b %e %H:%M"` format real `ls` uses for recent files.
- The final `printf` assembles all fields — permission string, hard-link count, owner, group, size, timestamp, and name — into one row.

### `main()` — argument parsing and the directory walk

```c
while ((opt = getopt(argc, argv, "al")) != -1) {
    switch(opt){
        case 'a': show_all = 1; break;
        case 'l': long_format = 1; break;
        default:
            fprintf(stderr, "usage: %s [-al] [path]\n", argv[0]);
            return 1;
    }
}
```

- `getopt(argc, argv, "al")` recognizes `-a` and `-l`, including combined form `-la`, setting the corresponding global flags.
- Any unrecognized flag prints a usage message and exits with status 1.

```c
const char *path = (optind < argc) ? argv[optind] : ".";
```

- After option parsing, `optind` points to the first non-flag argument. If one exists, it's treated as the target directory; otherwise the current directory (`"."`) is used.

```c
DIR *dir = opendir(path);
...
struct dirent *entry;
while((entry = readdir(dir)) != NULL){
    if(!show_all && entry->d_name[0] == '.') continue;
    if(long_format){
        print_long(path, entry->d_name);
    } else {
        printf("%s\n", entry->d_name);
    }
}
closedir(dir);
```

- `opendir` opens the directory stream; failure (e.g. path doesn't exist) is reported via `perror` and the program exits.
- `readdir` is called in a loop, returning one entry at a time until `NULL` (end of directory).
- Entries starting with `.` are skipped unless `-a` was passed — this is how dotfiles are hidden by default, matching real `ls` behavior.
- Depending on `-l`, either the full metadata row (`print_long`) or just the bare name is printed.
- `closedir` releases the directory stream once the walk is done.

---

## Debug Output Note

The current code prints extra diagnostic lines inside `print_long` (raw `mode`, and the octal values of each `S_I*` macro via `printf("%#o\n", ...)`). These were left in while verifying the bitmask logic manually against known octal permission values, and should be removed once the padding/alignment work (sorting entries, right-aligning link count and size columns) is implemented — see the main `C-playground` README for what's still pending on `lsbtw`.
