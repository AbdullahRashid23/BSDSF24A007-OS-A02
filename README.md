# ls Assignment Project

This project implements a small `ls`-style utility in C, building on the provided starter program. It supports the core features required for the assignment:

- default column-style directory listing
- long listing via `-l`
- horizontal listing via `-x`
- recursive listing via `-R`
- alphabetical sorting
- ANSI color output for common file types

## Build

```bash
make
```

## Run

```bash
./bin/ls
./bin/ls -l
./bin/ls -x
./bin/ls -R src
```
