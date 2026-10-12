# Operating Systems Programming Assignment 02

**Roll number:** BSDSF24A007

**GitHub repository:** [BSDSF24A007-OS-A02](https://github.com/AbdullahRashid23/BSDSF24A007-OS-A02)

## Project summary

This project extends the supplied `ls-v1.0.0.c` starter program with long
listing (`-l`), terminal-width-based columns, horizontal display (`-x`),
alphabetical sorting, file-type colors, and recursive listing (`-R`).
The implementation uses a dynamic array of directory entries, `lstat()` on
POSIX systems, `qsort()`, and the instructor's Makefile.

## Build and run

Build from the project root:

```sh
make clean
make
```

The executable is produced in `bin/`. Examples:

```sh
./bin/ls
./bin/ls -l
./bin/ls -x
./bin/ls -R src
```

## Report questions

### Feature 2: Long listing

#### 1. What is the crucial difference between `stat()` and `lstat()`? When is `lstat()` more appropriate for `ls`?

`stat(path, &info)` follows a symbolic link and returns metadata for its
target. `lstat(path, &info)` returns metadata for the link itself. `lstat()`
is appropriate when `ls` needs to identify and display a symbolic link as a
link, rather than treating it as the target file. This implementation uses
`lstat()` on POSIX systems and `stat()` on Windows.

#### 2. How can bitwise operators and predefined macros extract file type and permissions from `st_mode`?

`st_mode` contains both file-type and permission bits. File-type macros such
as `S_ISDIR(mode)` and `S_ISREG(mode)` test the type. Permission bits can be
tested with bitwise AND, for example `(mode & S_IRUSR)` for owner-read and
`(mode & S_IXUSR)` for owner-execute. The implementation uses these tests to
format the `rwx` permission string, including set-user-ID, set-group-ID, and
sticky bits.

### Feature 3: Down-then-across columns

#### 3. Explain the general logic for printing items in a down-then-across format. Why is one simple loop insufficient?

The program first stores and sorts all entries, finds the longest filename,
and calculates the number of columns and rows. It then iterates over rows and
columns, selecting each cell with `column * rows + row`. A single sequential
loop would print row-major order instead of filling each column from top to
bottom before moving across.

#### 4. What is `ioctl` used for here, and what are the limitations of using only an 80-column fallback?

On POSIX systems, `ioctl` with `TIOCGWINSZ` obtains the terminal's current
width so the column count can be calculated from the available space and the
longest filename. A fixed 80-column fallback cannot adapt to wider or
narrower terminals, so it can waste space or cause lines to exceed the
visible terminal width. The implementation uses 80 columns when terminal
width detection is unavailable.

### Feature 4: Horizontal columns and display-mode selection

#### 5. Compare the complexity of down-then-across and horizontal printing. Which requires more pre-calculation and why?

Down-then-across requires more pre-calculation: it needs the maximum filename
width, number of columns, row count, and a column-major index mapping.
Horizontal output visits entries in row-major order and uses the available
column count to wrap to the next row, so it requires less index remapping.

#### 6. How does the program manage `-l`, `-x`, and default display modes?

`getopt()` sets the `long_mode` and `horizontal_mode` flags. After collecting
and sorting directory entries, `list_directory()` selects long output when
`-l` is set; otherwise `display_entries()` prints columns, using
`horizontal_mode` to choose row-major (`-x`) or down-then-across (default)
ordering. When both `-l` and `-x` are supplied, long mode takes precedence.

### Feature 5: Alphabetical sorting

#### 7. Why must directory entries be read into memory before sorting, and what are the drawbacks for very large directories?

Directory iteration yields entries incrementally in filesystem order, which
is not guaranteed to be alphabetical. To sort the complete set before
printing, the program stores entries in a dynamically allocated array and
then sorts that array. Very large directories can consume substantial memory
and sorting adds time; allocation can also fail.

#### 8. What is the purpose and signature of the `qsort()` comparison function, and why does it take `const void *` arguments?

The comparator tells `qsort()` how to order two array elements. Its generic
signature is `int compare(const void *a, const void *b)`. Since `qsort()`
works with arrays of any element type, it passes untyped pointers; the
comparator converts them to pointers to the actual element type. This
program compares two `Entry` objects by their `name` fields with `strcmp()`.
It returns a negative value, zero, or a positive value when the first name is
less than, equal to, or greater than the second.

### Feature 6: Colorized output

#### 9. How do ANSI escape codes produce color? Give the sequence used for green text.

An ANSI escape sequence is written to the terminal to change its text
attributes; a reset sequence restores the normal attributes. The
implementation uses `"\033[32m"` to start green text and `"\033[0m"` to reset
after the filename. For example:

```c
printf("\033[32mgreen text\033[0m");
```

#### 10. Which `st_mode` bits identify executables for the owner, group, or others?

The execute bits are `S_IXUSR` (owner), `S_IXGRP` (group), and `S_IXOTH`
(others). The implementation treats a file as executable when at least one
of these bits is set.

### Feature 7: Recursive listing

#### 11. What is a base case, and what stops this recursive listing from continuing forever?

A base case is the condition that stops a recursive call from making further
calls. Here, recursion proceeds only for entries whose metadata identifies
them as directories; `.` and `..` are excluded, and symbolic links are
identified with `lstat()` rather than followed. A directory with no eligible
child directories therefore ends that path of recursion.

#### 12. Why must the recursive call use the full path to a subdirectory?

The full path identifies the child relative to the directory currently being
listed, such as `parent_dir/subdir`. Calling the bare name `subdir` would
resolve it relative to the process's current working directory, not
necessarily relative to `parent_dir`, so the program could fail to open it
or list a different directory.

## Git workflow

The feature work is represented by the six feature branches, merge commits,
annotated tags `v1.1.0` through `v1.6.0`, and matching GitHub releases with
version-specific executable assets.
