# OS Assignment 02 Report

## Summary

This project reimplements a subset of the Unix `ls` command in C, matching the assignment phases from the starter program through recursive, colorized, and sorted listing modes. The implementation includes the required feature branches, tags, and GitHub releases.

## Build and verification

- `make` builds the project successfully.
- The executable is produced in `bin/ls`.
- Required output modes were checked: default listing, `-l`, `-x`, and `-R src`.

## Feature checklist

- [x] Starter project structure created.
- [x] Default listing works.
- [x] Long listing supports `-l`.
- [x] Horizontal listing supports `-x`.
- [x] Recursive listing supports `-R`.
- [x] Files are sorted alphabetically.
- [x] ANSI colors highlight directories, executables, archives, symlinks, and special files.

## Report questions

### 1. Compare the "down then across" and "across" display logic.
The default vertical layout requires more pre-calculation because the program must determine the longest filename, calculate how many columns fit in the terminal width, compute the row count, and then map entries using `col * rows + row`. This is a column-major layout. The horizontal `-x` mode is simpler because it prints filenames sequentially in row-major order, only inserting a newline when the running horizontal width would exceed the terminal width. It still uses the longest filename to size each column, but it does far less index translation and layout planning.

### 2. How was the display mode selected?
The program uses the standard `getopt()` loop in `main()` and maintains mode flags such as `long_mode`, `horizontal_mode`, and `recursive_mode`. After reading the directory entries into an array, the code checks the corresponding flags and calls the appropriate display routine: long listing for `-l`, horizontal output for `-x`, and the default column layout otherwise.

### 3. Why sort directory entries in memory before printing?
The `ls` output must be deterministic and user-friendly, so the entries must be read into an array, sorted, and then printed. This allows `qsort()` to compare every entry before rendering the final display. The main drawback is memory usage: a directory with millions of files can require a large amount of memory just to store names before the sort and print step.

### 4. What is the `qsort()` comparison function and why does it use `const void *`?
A valid comparison function for `qsort()` has the signature:

```c
int compare(const void *a, const void *b);
```

The function casts the two pointers back to the actual element type, such as `const char **`, and then uses `strcmp()` to compare the names. The `const void *` type is required because `qsort()` is generic and operates on arbitrary data types; it accepts pointers to any item type and the comparator must therefore work with opaque pointer values.

### 5. How do ANSI escape codes produce color?
ANSI color is produced by writing escape sequences to the terminal. The standard pattern is:

```c
printf("\033[0;32mgreen text\033[0m");
```

`\033[` starts the escape sequence, `0;32m` means reset plus green, and `\033[0m` resets the terminal back to the default colors. This is how directory, executable, archive, symlink, and special-file output is colorized in the implementation.

### 6. How is an executable file identified?
An executable file is detected by checking the execute bits in `st_mode`. The relevant bits are:

- `S_IXUSR` for owner execute
- `S_IXGRP` for group execute
- `S_IXOTH` for other/anyone execute

The code checks whether any of these bits are set, and if so, it prints the name in green. This matches the requirement for executable-file coloring.

