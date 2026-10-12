# OS Assignment 02 Report

## Summary

This project recreates the behavior of the Unix `ls` utility in a simplified form that matches the assignment requirements: column-oriented listing, long format output, alphabetical sorting, recursive traversal, and colored file names.

## Build verification

- Run `make` to compile the project.
- Confirm the executable is produced in `bin/ls`.

## Feature checklist

- [x] Starter project structure created.
- [x] Default listing works.
- [x] Long listing supports `-l`.
- [x] Horizontal listing supports `-x`.
- [x] Recursive listing supports `-R`.
- [x] Files are sorted alphabetically.
- [x] ANSI colors highlight directory, executable, archive, link, and special files.

## Feature 2 — v1.1.0`n- Implemented `-l` long listing with `stat`/`lstat`, `getpwuid`, `getgrgid`, and timestamp formatting.

## Feature 3 — v1.2.0`n- Implemented default column display and terminal-aware width calculation.

## Feature 4 — v1.3.0`n- Implemented `-x` horizontal row-major output while preserving the default down-then-across layout.

## Feature 5 — v1.4.0`n- Added deterministic alphabetical sorting with `qsort` and `strcmp`.

## Feature 6 — v1.5.0`n- Added ANSI colorized output for directories, executables, archives, symlinks, and special files.

## Feature 7 — v1.6.0`n- Added recursive traversal for `-R` with directory headers and safe subdirectory handling.

