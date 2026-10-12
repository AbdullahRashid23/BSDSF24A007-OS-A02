#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700

#include <dirent.h>
#include <errno.h>
#ifndef _WIN32
#include <grp.h>
#include <pwd.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <sys/ioctl.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_PINK "\033[35m"
#define COLOR_REVERSE "\033[7m"

#ifndef S_IFLNK
#define S_IFLNK 0xA000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0xC000
#endif
#ifndef S_IFIFO
#define S_IFIFO 0x1000
#endif
#ifndef S_ISLNK
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#endif

#if defined(_WIN32)
#define FILE_STAT(path, st) stat((path), (st))
#else
#define FILE_STAT(path, st) lstat((path), (st))
#endif

typedef struct {
    char *name;
    char *path;
    struct stat st;
} Entry;

static int show_hidden = 0;
static int long_mode = 0;
static int horizontal_mode = 0;
static int recursive_mode = 0;
static int colorize = 0;

static void free_entries(Entry *entries, size_t count) {
    if (entries == NULL) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        free(entries[i].name);
        free(entries[i].path);
    }
    free(entries);
}

static char *join_path(const char *dir, const char *name) {
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    int needs_slash = (dir_len > 0 && dir[dir_len - 1] != '/');
    size_t total = dir_len + (needs_slash ? 1 : 0) + name_len + 1;
    char *path = malloc(total);
    if (path == NULL) {
        return NULL;
    }
    snprintf(path, total, "%s%s%s", dir, needs_slash ? "/" : "", name);
    return path;
}

static int compare_entries(const void *a, const void *b) {
    const Entry *left = (const Entry *)a;
    const Entry *right = (const Entry *)b;
    return strcmp(left->name, right->name);
}

static int name_has_archive_suffix(const char *name) {
    if (name == NULL) {
        return 0;
    }
    size_t length = strlen(name);
    return (length >= 4 && strcmp(name + length - 4, ".tar") == 0) ||
           (length >= 3 && strcmp(name + length - 3, ".gz") == 0) ||
           (length >= 4 && strcmp(name + length - 4, ".zip") == 0);
}

static void print_name(const char *name, const struct stat *st) {
    if (!colorize || !isatty(STDOUT_FILENO)) {
        printf("%s", name);
        return;
    }

    const char *color = NULL;
    if (S_ISDIR(st->st_mode)) {
        color = COLOR_BLUE;
    } else if (S_ISLNK(st->st_mode)) {
        color = COLOR_PINK;
    } else if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode) || S_ISFIFO(st->st_mode) || S_ISSOCK(st->st_mode)) {
        color = COLOR_REVERSE;
    } else if ((st->st_mode & S_IXUSR) || (st->st_mode & S_IXGRP) || (st->st_mode & S_IXOTH)) {
        color = COLOR_GREEN;
    } else if (S_ISREG(st->st_mode) && name_has_archive_suffix(name)) {
        color = COLOR_RED;
    }

    if (color != NULL) {
        printf("%s%s%s", color, name, COLOR_RESET);
        return;
    }

    printf("%s", name);
}

static void format_permissions(mode_t mode, char perms[11]) {
    perms[0] = S_ISDIR(mode) ? 'd' : (S_ISLNK(mode) ? 'l' : (S_ISFIFO(mode) ? 'p' : (S_ISBLK(mode) ? 'b' : (S_ISCHR(mode) ? 'c' : '-'))));
    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';
}

static void print_long_entry(const Entry *entry) {
    char perms[11];
    format_permissions(entry->st.st_mode, perms);

#ifndef _WIN32
    struct passwd *pw = getpwuid(entry->st.st_uid);
    struct group *gr = getgrgid(entry->st.st_gid);
    const char *owner = pw ? pw->pw_name : "";
    const char *group = gr ? gr->gr_name : "";
#else
    const char *owner = "";
    const char *group = "";
#endif
    char time_buffer[32];
    struct tm *tm_info = localtime(&entry->st.st_mtime);
    strftime(time_buffer, sizeof(time_buffer), "%b %e %H:%M", tm_info);

    printf("%s %3lu %-8s %-8s %8lld %s ",
           perms,
           (unsigned long)entry->st.st_nlink,
           owner,
           group,
           (long long)entry->st.st_size,
           time_buffer);
    print_name(entry->name, &entry->st);
    printf("\n");
}

static void print_column_name(const Entry *entry, size_t width) {
    if (colorize && isatty(STDOUT_FILENO)) {
        const char *color = NULL;
        if (S_ISDIR(entry->st.st_mode)) {
            color = COLOR_BLUE;
        } else if (S_ISLNK(entry->st.st_mode)) {
            color = COLOR_PINK;
        } else if (S_ISCHR(entry->st.st_mode) || S_ISBLK(entry->st.st_mode) || S_ISFIFO(entry->st.st_mode) || S_ISSOCK(entry->st.st_mode)) {
            color = COLOR_REVERSE;
        } else if (((entry->st.st_mode & S_IXUSR) || (entry->st.st_mode & S_IXGRP) || (entry->st.st_mode & S_IXOTH)) && !S_ISDIR(entry->st.st_mode)) {
            color = COLOR_GREEN;
        } else if (name_has_archive_suffix(entry->name)) {
            color = COLOR_RED;
        }

        if (color != NULL) {
            printf("%s%s%s", color, entry->name, COLOR_RESET);
            int pad = (int)(width - strlen(entry->name));
            if (pad > 0) {
                printf("%*s", pad + 2, "");
            } else {
                printf("  ");
            }
            return;
        }
    }

    printf("%-*s  ", (int)width, entry->name);
}

static void display_entries(Entry *entries, size_t count) {
    if (count == 0) {
        return;
    }

    size_t max_width = 0;
    for (size_t i = 0; i < count; i++) {
        size_t len = strlen(entries[i].name);
        if (len > max_width) {
            max_width = len;
        }
    }

    size_t columns = 1;
    if (!long_mode) {
        int terminal_width = 80;
#if !defined(_WIN32)
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
            terminal_width = ws.ws_col;
        }
#endif
        columns = (size_t)terminal_width / (max_width + 2);
        if (columns < 1) {
            columns = 1;
        }
    }

    size_t rows = (count + columns - 1) / columns;
    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < columns; col++) {
            size_t index = horizontal_mode ? (row * columns + col) : (col * rows + row);
            if (index >= count) {
                break;
            }
            if (col > 0) {
                printf("  ");
            }
            print_column_name(&entries[index], max_width);
        }
        printf("\n");
    }
}

static void list_directory(const char *path, int recursive_depth) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return;
    }

    if (recursive_mode && strcmp(path, ".") != 0) {
        printf("%s:\n", path);
    } else if (recursive_mode && strcmp(path, ".") == 0) {
        printf(".:\n");
    }

    Entry *entries = NULL;
    size_t count = 0;
    size_t capacity = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char *full_path = join_path(path, entry->d_name);
        if (full_path == NULL) {
            perror("malloc");
            continue;
        }

        struct stat st;
        if (FILE_STAT(full_path, &st) != 0) {
            fprintf(stderr, "Failed to stat: %s\n", full_path);
            free(full_path);
            continue;
        }

        if (count == capacity) {
            size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
            Entry *new_entries = realloc(entries, new_capacity * sizeof(*new_entries));
            if (new_entries == NULL) {
                perror("realloc");
                free(full_path);
                free_entries(entries, count);
                closedir(dir);
                return;
            }
            entries = new_entries;
            capacity = new_capacity;
        }

        entries[count].name = strdup(entry->d_name);
        entries[count].path = full_path;
        entries[count].st = st;
        count++;
    }

    closedir(dir);
    qsort(entries, count, sizeof(entries[0]), compare_entries);

    if (long_mode) {
        if (count > 0) {
            long long total = 0;
            for (size_t i = 0; i < count; i++) {
#if !defined(_WIN32)
                total += (entries[i].st.st_blocks / 2);
#else
                total += entries[i].st.st_size / 512;
#endif
            }
            printf("total %lld\n", total);
        }
        for (size_t i = 0; i < count; i++) {
            print_long_entry(&entries[i]);
        }
    } else {
        display_entries(entries, count);
    }

    if (recursive_mode) {
        for (size_t i = 0; i < count; i++) {
            if (S_ISDIR(entries[i].st.st_mode)) {
                printf("\n%s:\n", entries[i].path);
                list_directory(entries[i].path, recursive_depth + 1);
            }
        }
    }

    free_entries(entries, count);
}

int main(int argc, char const *argv[]) {
    int opt;

    while ((opt = getopt((int)argc, (char *const *)argv, "alRx")) != -1) {
        switch (opt) {
            case 'a':
                show_hidden = 1;
                break;
            case 'l':
                long_mode = 1;
                break;
            case 'R':
                recursive_mode = 1;
                break;
            case 'x':
                horizontal_mode = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-alRx] [directory ...]\n", argv[0]);
                return 1;
        }
    }

    colorize = isatty(STDOUT_FILENO);

    if (optind >= argc) {
        list_directory(".", 0);
        return 0;
    }

    for (int i = optind; i < argc; i++) {
        if (argc - optind > 1) {
            printf("%s:\n", argv[i]);
        }
        list_directory(argv[i], 0);
        if (i + 1 < argc) {
            printf("\n");
        }
    }

    return 0;
}
