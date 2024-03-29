#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <pwd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <values.h>


typedef struct options
{
    char *name;
    char *sort;
    char *mask;
    struct passwd *user;
    long f;
    long t;
    bool hidden;
    char zero;
    int path;
} options;


void optionsInit(options *option);

void showH();

int parseOptions(int argc, char **argv, options *options);

int onlyNumbers(const char *s);

int isHidden(const char *file);

char *getFileName(const char *path);

char *getPerms(struct stat *s);

int isSuitable(struct dirent *drnt, options *opt, struct stat status, int counter);

int comparePaths(const void *f1, const void *f2);

int compareNames(const void *f1, const void *f2);

int compareSizes(const void *f1, const void *f2);

void sort(char ***array, const int *arraySize, options *options);

void addToArray(char *filename, char ***array, int *arraySize);

int getPaths(DIR *dir, options *options, char **dirpath, char ***result, int *size, int counter);

void closeAndFree(DIR *directory, char **array, const int *size);

int main(int argc, char **argv)
{
    options options;
    optionsInit(&options);
    DIR *directory;
    char **resultsArray = NULL;
    int size = 0;
    char *path = ".";

    int error = parseOptions(argc, argv, &options);

    if (options.path != 0) {
        path = argv[options.path];

        if (strlen(path) > 1 && path[strlen(path) - 1] == '/') {
            path[strlen(path) - 1] = '\0';
        }
        if (strlen(path) > 1 && path[strlen(path) - 1] == '/') {
            path[strlen(path) - 1] = '\0';
        }
    }

    if (error != 0) {
        if (error == -1)
            return 0;
        if (error > 0)
            return error;
    }

    if ((directory = opendir(path)) != NULL) {
        error = getPaths(directory, &options, &path, &resultsArray, &size, 0);
        sort(&resultsArray, &size, &options);
        for (int i = 0; i < size; i++) {
            printf("%s%c", resultsArray[i], options.zero);
        }
        closeAndFree(directory, resultsArray, &size);
        if (error != 0) {
            fprintf(stderr, "Error\n");
            return error;
        }
    } else {
        fprintf(stderr, "%s: No such file or directory.\n", path);
        return 3;
    }

    return 0;
}

void optionsInit(options *option)
{
    if (option != NULL) {
        option->name = NULL;
        option->sort = NULL;
        option->mask = NULL;
        option->user = NULL;
        option->f = 0;
        option->t = LONG_MAX;
        option->hidden = false;
        option->zero = '\n';
        option->path = 0;
    }
}

void showH()
{
    printf("Help.\n"
           "USAGE: [OPTIONS] [START_DIR]\n"
           "OPTIONS:\n"
           "-h 		        Show help.\n"
           "-n NAME	        Filter by name in name.\n"
           "-m MASK 	    Filter by permissions.\n"
           "-u USER 	    Filter by user name.\n"
           "-f NUM 	        Filter by minimal depth.\n"
           "-t NUM          Filter by maximal depth.\n"
           "-s SORT 		Sorting by full path or size.\n"
           "-a 		        Go throught hidden dirs.\n"
           "-0 		        Use null byte as line separator\n\n"
           "SORT:\n"
           "\"s\"           Sort by size.\n"
           "\"f\"           Sort by full/relative file path.");

}

int parseOptions(int argc, char **argv, options *options)
{
    int opt;
    char *endpf = NULL;
    char *endpt = NULL;
    if (options != NULL) {
        while ((opt = getopt(argc, argv, ":a0hn:s:u:f:t:m:")) != -1) {
            switch (opt) {
            case 'n':
                options->name = optarg;
                break;
            case 's':
                if (strcmp(optarg, "s") == 0 || strcmp(optarg, "f") == 0)
                    options->sort = optarg;
                else {
                    fprintf(stderr, "%s: No such comparator\n", optarg);
                    return 1;
                }
                break;
            case 'm':
                if (onlyNumbers(optarg) == 0 || strstr(optarg, "8") != NULL || strstr(optarg, "9") != NULL) {
                    fprintf(stderr, "%s: is invalid mask.\n", optarg);
                    return 1;
                }
                options->mask = optarg;
                break;
            case 'u':
                if ((options->user = getpwnam(optarg)) == NULL) {
                    options->user = NULL;
                    fprintf(stderr, "%s: No such user.\n", optarg);
                    return 1;
                }
                break;
            case 'f':
                errno = 0;
                long valueF = strtol(optarg, &endpf, 10);
                if (*endpf != '\0' || errno != 0) {
                    fprintf(stderr, "%s: invalid number\n", optarg);
                    return 1;
                }
                options->f = valueF;
                break;
            case 't':
                errno = 0;
                long valueT = strtol(optarg, &endpt, 10);
                if (*endpt != '\0' || errno != 0) {
                    fprintf(stderr, "%s: invalid number\n", optarg);
                    return 1;
                }
                options->t = valueT;
                break;
            case 'a':
                options->hidden = true;
                break;
            case '0':
                options->zero = '\0';
                break;
            case 'h':
                showH();
                return -1;
            case ':':
                fprintf(stderr, "option requires an argument -%c!\n", optopt);
                return 1;
            case '?':
                fprintf(stderr, "unknown option: -%c\n", optopt);
                return 1;
            }
        }
        if (optind < argc)
            options->path = optind;
    }
    return 0;
}

int onlyNumbers(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0)
            return 0;
    }
    return 1;
}

int isHidden(const char *file)
{
    if (file != NULL) {
        if (file[0] == '.')
            return 1;
    }
    return 0;
}

char *getFileName(const char *path)
{
    const char *file;
    if (path != NULL) {
        char *lastSlash = strrchr(path, '/');
        if (lastSlash == NULL)
            file = path;
        else
            file = lastSlash + 1;
        char *filename;
        filename = malloc(strlen(file) + 1);
        strncpy(filename, file, strlen(file));
        filename[strlen(file)] = '\0';
        return filename;
    }
    return NULL;
}

char *getPerms(struct stat *s)
{
    static char perms[4];
    int o = 0;
    int g = 0;
    int p = 0;

    if (s->st_mode & S_IRUSR)
        o += 4;
    if (s->st_mode & S_IWUSR)
        o += 2;
    if (s->st_mode & S_IXUSR)
        o += 1;
    if (s->st_mode & S_IRGRP)
        g += 4;
    if (s->st_mode & S_IWGRP)
        g += 2;
    if (s->st_mode & S_IXGRP)
        g += 1;
    if (s->st_mode & S_IROTH)
        p += 4;
    if (s->st_mode & S_IWOTH)
        p += 2;
    if (s->st_mode & S_IXOTH)
        p += 1;

    sprintf(perms, "%d%d%d", o, g, p);
    return perms;
}

int isSuitable(struct dirent *drnt, options *opt, struct stat status, int counter)
{
    if (drnt != NULL && opt != NULL) {
        struct passwd *owner = NULL;
        char *fname = NULL;
        if (opt->name != NULL) {
            fname = getFileName(drnt->d_name);
            if (strstr(fname, opt->name) == NULL) {
                free(fname);
                return 0;
            }
            free(fname);
        }
        if (opt->mask != NULL) {
            if (strtol(opt->mask, NULL, 8) != strtol(getPerms(&status), NULL, 8))
                return 0;
        }
        if (opt->user != NULL) {
            owner = getpwuid(status.st_uid);
            if (owner->pw_uid != opt->user->pw_uid)
                return 0;
        }
        if (opt->f - 1 > counter || opt->t - 1 < counter)
            return 0;
        if (!opt->hidden) {
            if (isHidden(drnt->d_name))
                return 0;
        }
        return 1;
    }
    return 0;
}

void sort(char ***array, const int *arraySize, options *options)
{
    if (array != NULL && arraySize != NULL && options != NULL) {
        char **arr = *array;
        int (*operation)(const void *, const void *) = NULL;
        if (options->sort != NULL) {
            if (strcmp(options->sort, "s") == 0) {
                operation = &compareSizes;
            } else if (strcmp(options->sort, "f") == 0) {
                operation = &comparePaths;
            }
        } else {
            operation = &compareNames;
        }
        if (arr != NULL)
            qsort(arr, *arraySize, sizeof(char *), operation);
    }

}

int compareSizes(const void *f1, const void *f2)
{
    struct stat statusOne;
    struct stat statusTwo;
    lstat(*(char **) f1, &statusOne);
    lstat(*(char **) f2, &statusTwo);
    if (statusOne.st_size < statusTwo.st_size)
        return 1;
    if (statusOne.st_size > statusTwo.st_size)
        return -1;
    return compareNames(f1, f2);
}

int comparePaths(const void *f1, const void *f2)
{
    return strcmp(*(char **) f1, *(char **) f2);
}

int compareNames(const void *f1, const void *f2)
{
    char *firstName = getFileName(*(char **) f1);
    char *secondName = getFileName(*(char **) f2);
    int result = strcasecmp(firstName, secondName);
    if (result == 0) {
        result = comparePaths(f1, f2);
    }
    free(firstName);
    free(secondName);
    return result;
}

void addToArray(char *filename, char ***array, int *arraySize)
{
    if (array != NULL && arraySize != NULL && filename != NULL) {
        char **arr = *array;
        if (*arraySize % 10 == 0) {
            arr = (char **) realloc(arr, sizeof(char *) * (*arraySize + 10));
        }
        arr[*arraySize] = filename;
        (*arraySize)++;
        *array = arr;
    }
}

int getPaths(DIR *dir, options *options, char **dirpath, char ***result, int *size, int counter)
{
    if (dir != NULL && options != NULL && dirpath != NULL && result != NULL) {
        struct dirent *drnt = NULL;
        char *filepath;
        DIR *nextDir = NULL;
        struct stat status;

        while ((drnt = readdir(dir)) != NULL) {
            filepath = malloc(strlen(*dirpath) + strlen(drnt->d_name) + 2);

            strcpy(filepath, *dirpath);
            strcat(filepath, "/");
            strcat(filepath, drnt->d_name);


            if (lstat(filepath, &status) < 0) {
                fprintf(stderr, "Status error: %s\n", filepath);
                free(filepath);
                return 4;
            }

            if (!S_ISDIR(status.st_mode) ) {
                if (isSuitable(drnt, options, status, counter)) {
                    addToArray(filepath, result, size);
                } else {
                    free(filepath);
                }


            } else {
                if (!(options->f - 3 > counter || options->t -2 < counter)) {
                    if ((strcmp(drnt->d_name, ".") != 0) && (strcmp(drnt->d_name, "..") != 0)) {
                        if (options->hidden || isHidden(drnt->d_name) != 1) {
                            if ((nextDir = opendir(filepath)) != NULL) {
                                getPaths(nextDir, options, &filepath, result, size, counter + 1);
                                closedir(nextDir);
                            } else {
                                fprintf(stderr, "Problem while entering directory: %s: Error: %s\n", filepath, strerror(errno));
                            }
                        }
                    }
                }
                free(filepath);
            }
        }
    }
    return 0;
}

void closeAndFree(DIR *directory, char **array, const int *size)
{
    if (array != NULL && size != NULL) {
        for (int i = 0; i < *size; i++) {
            free(array[i]);
        }
        free(array);
    }
    closedir(directory);
}
