#ifndef CBUILD_H
#define CBUILD_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/wait.h>

/**
 * @brief String Builder implementation
 */
typedef struct {
  char *str; ///< the string being built
  size_t size; ///< number of character in the string
  size_t capacity; ///< total capacity of str
} cbuild_str_builder;

/**
 * @brief append a character to the string builder
 * @param sb the string builder
 * @param c the character
 */
void cbuild_str_builder_append_char(cbuild_str_builder *str, char c);
/**
 * @brief append a cstring to the string builder
 * @param sb the string builder
 * @param cstr the cstring
 */
void cbuild_str_builder_append_cstr(cbuild_str_builder *str, char *cstr);
/**
 * @brief create a string builder from a cstring
 * @param str the cstring
 */
char *cbuild_str_builder_to_cstr(cbuild_str_builder *str);

/**
 * TODO
 * @brief String view
 */
typedef struct {
  const char *str; ///< the string
  size_t size; ///< size of the string
} cbuild_str_view;

/**
 * @brief dynamic string vector
 */
typedef struct {
    char **strs; ///< array of strings
    size_t size; ///< number of strings in the array
    size_t capacity; ///< total capacity of the array
} cbuild_str_vector;

/**
 * @brief adds a string to a vector
 * @param vector the vector
 * @param str the string
 */
void cbuild_str_vector_add_str(cbuild_str_vector *vector, char *str);
/**
 * \
 * @brief adds strings to a vector
 * @param vector the vector
 * @param __VA_ARGS__ the strings
 */
void __cbuild_str_vector_add_strs(cbuild_str_vector *vector, char *str, ...);
#define cbuild_str_vector_add_strs(STR_VECTOR, ...)                               \
    __cbuild_str_vector_add_strs(STR_VECTOR, __VA_ARGS__, NULL)

/**
 * @brief remove the last string of a vector
 * @param vector the vector
 */
void cbuild_str_vector_pop_back(cbuild_str_vector *vector);
/**
 * @brief creates a string, joining all the strings in a vector with a separator
 * @param vector the vector
 * @param sep the separator between each element
 */
char *cbuild_str_vector_join(cbuild_str_vector *vector, char *sep);

/**
 * @brief command to be executed, basically a str_vector
 */
typedef struct {
    cbuild_str_vector argv; ///< the command to execute
} cbuild_command;

/**
 * @brief adds an argument to a command
 * @param command the command
 * @param arg the argument
 */
void cbuild_command_add_arg(cbuild_command *command, char *arg);
/**
 * @brief adds arguments to a command
 * @param command the command
 * @param __VA_ARGS__ the argument
 */
#define cbuild_command_add_args(COMMAND, ...)                                  \
    __cbuild_command_add_args(COMMAND, __VA_ARGS__, NULL)
void __cbuild_command_add_args(cbuild_command *command, char *arg, ...);
/**
 * @brief executes a command synchronously (aka. waits)
 * @param command the command
 */
int cbuild_command_exec_sync(cbuild_command *command);

/**
 * @brief representation of a target's source
 */
typedef struct {
    union
    {
        struct cbuild_target *target; ///< the target source
        char *file; ///< the filename source
    } source; ///< the source
    enum {
        CBUILD_NONE,
        CBUILD_FILE_SOURCE, ///< if the source is a file
        CBUILD_TARGET_SOURCE, ///< if the source is an other target
    } source_type; ///< the type of source for the target
    int add_to_command;
} cbuild_source;

/**
 * @def CBUILD_MAKE_FILE_SOURCE(FILENAME)
 * @brief creates a cbuild_source using the file type
 */
#define CBUILD_MAKE_FILE_SOURCE(FILENAME)                                      \
    {                                                                          \
        .source.file = FILENAME, .source_type = CBUILD_FILE_SOURCE,            \
        .add_to_command = 1                                                    \
    }
#define CBUILD_MAKE_FILE_HEADER(FILENAME)                                      \
    {                                                                          \
        .source.file = FILENAME, .source_type = CBUILD_FILE_SOURCE,            \
        .add_to_command = 0                                                    \
    }
/**
 * @def CBUILD_MAKE_TARGET_SOURCE(FILENAME)
 * @brief creates a cbuild_source using the target type
 */
#define CBUILD_MAKE_TARGET_SOURCE(TARGET)                                      \
    {                                                                          \
        .source.target = TARGET, .source_type = CBUILD_TARGET_SOURCE,        \
        .add_to_command = 1                                                    \
    }

/**
 * @brief structure to represent a target that can be built
 */
typedef struct cbuild_target {
    char *target_file; ///< file to be generated by the target
    int is_built;
    cbuild_str_vector command; ///< first part of the command to execute to build the target
    cbuild_source sources[]; ///< sources required by the target
} cbuild_target;

/**
 * @brief initializer for a target
 * @param FILENAME name of the file associated to the target
 * @param __VA_ARGS__ all the different sources,
 *        see CBUILD_MAKE_(FILE|TARGET)_SOURCE
 *
 * @code
 * static cbuild_target target = CBUILD_TARGET("elf",
 *         CBUILD_MAKE_FILE_SOURCE("elf.c"),
 *         CBUILD_MAKE_TARGET_SOURCE(&target2)
 *     )
 * @endcode
 */
#define CBUILD_TARGET(FILENAME, ...)                                           \
    {                                                                          \
        .target_file = FILENAME,                                               \
        .sources = {                                                           \
            __VA_ARGS__,                                                       \
            { .source_type = CBUILD_NONE }           \
        }                                                                      \
    }

int cbuild_target_is_older_than_source(const char *target, const char *source);

int cbuild_file_exists(const char *file);

/**
 * @brief builds a target
 */
int cbuild_build_target(cbuild_target *target, int *built,
        int always_recompile);
int cbuild_multiprocess_build_target(cbuild_target *target, int *built,
        int always_recompile, unsigned nb_process);
int cbuild_clean_target(cbuild_target *target);

typedef struct cbuild_target_stack_item
{
    cbuild_target *target;
    struct cbuild_target_stack_item *next;
} cbuild_target_stack_item;

typedef struct
{
    cbuild_target_stack_item *head;
} cbuild_target_stack;

void cbuild_target_stack_push(cbuild_target_stack *sk, cbuild_target *target);

typedef struct
{
    pid_t pid;
    cbuild_target *target;
} cbuild_target_map_item;

typedef struct
{
    cbuild_target_map_item *items;
    size_t size;
    size_t capacity;
} cbuild_target_map;

void cbuild_target_map_init(cbuild_target_map *map, size_t capacity);
void cbuild_target_map_insert(cbuild_target_map *map, pid_t pid, cbuild_target *target);
void cbuild_target_map_remove(cbuild_target_map *map, pid_t pid);
cbuild_target *cbuild_target_map_get(cbuild_target_map *map, pid_t pid);

/**
 * @brief different levels of logging
 */
enum cbuild_log_level {
  CBUILD_CLEAR, ///< "empty" type
  CBUILD_INFO, ///< for printing information (blue)
  CBUILD_DEBUG, ///< for printing debug information (green)
  CBUILD_WARN, ///< printing warnings (orange)
  CBUILD_ERROR, ///< printing errors (red)
};

/**
 * @brief logs information
 * @param log_level the level of logging
 * @param format the format string
 * @param ... the format arguments
 */
void cbuild_log(enum cbuild_log_level log_level, char *format, ...);

/**
 * @brief gets the path of the cbuild header file for self-rebuilding
 */
extern const char *cbuild_header_file_name;

extern unsigned cbuild_bootstrap_step;

int cbuild_write_argument(char *name, char *type, char* default_value, char
        *args, char *desc);
int cbuild_bootstrap_first_step(char *cbuild_source, char *cbuild_target,
        cbuild_command *build_command);

int __cbuild_rebuild_yourself(char *cbuild_source, char *cbuild_target, char *argv[]);
/**
 * @brief macro used to make cbuild rebuild itself if necessary
 * @details cbuild will try to detect if its sources have been modified more
 *          recently than the executable itself, just like the make utility
 *          inspired from tsoding's nob
 */
#define CBUILD_REBUILD_YOURSELF(ARGC, ARGV)                                    \
    do { \
        if (__cbuild_rebuild_yourself(__FILE__, argv[0], argv)) \
            return 1;\
    } while (0)

#ifdef CBUILD_IMPLEMENTATION

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define __COUNT_VAARGS(a, b, c, d, e, f, g, h, i, j, k, l, m, ...) m
#define COUNT_VAARGS(...) \
    __COUNT_VAARGS(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#if CBUILD_ENABLE_CARGPARSE

#define CARGPARSE_IMPLEMENTATION
#define CARG_LOCATION "cargparse.h.in"

#ifndef CARGPARSE_HEADER
#  define CARGPARSE_HEADER "cargparse.h"
#endif
#include CARGPARSE_HEADER

#else /* CBUILD_ENABLE_CARGPARSE */

int nb_process;

#endif /* ! CBUILD_ENABLE_CARGPARSE */


const char *cbuild_header_file_name = __FILE__;

#ifndef CBUILD_BOOTSTRAP
unsigned cbuild_bootstrap_step = 0;
#else
unsigned cbuild_bootstrap_step = CBUILD_BOOTSTRAP;
#endif

int cbuild_target_is_older_than_source(const char *target, const char *source)
{
    struct stat st;
    if (stat(source, &st) == -1)
        // TODO PANIC
        return 0;
    struct timespec source_st = st.st_mtim;

    if (stat(target, &st) == -1)
        return 1;
    struct timespec target_st = st.st_mtim;
    return source_st.tv_sec == target_st.tv_sec
               ? source_st.tv_nsec > target_st.tv_nsec
               : source_st.tv_sec > target_st.tv_sec;
}

int cbuild_file_exists(const char *file)
{
    return access(file, R_OK) == 0;
}

int cbuild_rename(char *source, char *target)
{
    if (rename(source, target))
    {
        cbuild_log(CBUILD_ERROR, "Could not rename %s to %s: %s", source,
                   target, strerror(errno));
        return 1;
    }
    return 0;
}

/*** str_builder impl ***/

void cbuild_str_builder_append_char(cbuild_str_builder *sb, char c)
{
    if (sb->str == NULL)
    {
        sb->capacity = 8;
        sb->str = malloc(sb->capacity * sizeof(char));
        sb->size = 0;
    }
    if (sb->size == sb->capacity)
    {
        sb->capacity *= 2;
        sb->str = realloc(sb->str, sb->capacity);
    }

    sb->str[sb->size++] = c;
}

void cbuild_str_builder_append_cstr(cbuild_str_builder *str, char *cstr)
{
    for (size_t i = 0; cstr[i] != 0; i++)
        cbuild_str_builder_append_char(str, cstr[i]);
}

char *cbuild_str_builder_to_cstr(cbuild_str_builder *str)
{
    cbuild_str_builder_append_char(str, '\0');
    char *res = str->str;
    str->capacity = 0;
    str->size = 0;
    str->str = NULL;
    return res;
}

void cbuild_str_vector_add_str(cbuild_str_vector *vector, char *str)
{
    if (vector->strs == NULL)
    {
        vector->capacity = 8;
        vector->strs = malloc(vector->capacity * sizeof(char *));
        vector->size = 0;
    }
    if (vector->size == vector->capacity)
    {
        vector->capacity *= 2;
        vector->strs = realloc(vector->strs, vector->capacity * sizeof(char *));
    }
    vector->strs[vector->size++] = str;
}

void __cbuild_str_vector_add_strs(cbuild_str_vector *vector, char *str, ...)
{
    va_list ap;
    va_start(ap, str);
    cbuild_str_vector_add_str(vector, str);
    while ((str = va_arg(ap, char *)) != NULL)
    {
        cbuild_str_vector_add_str(vector, str);
    }
}

void cbuild_str_vector_pop_back(cbuild_str_vector *vector)
{
    vector->size--;
}

char *cbuild_str_vector_join(cbuild_str_vector *vector, char *sep)
{
    cbuild_str_builder str_builder = { 0 };
    cbuild_str_builder_append_cstr(&str_builder, vector->strs[0]);
    for (size_t i = 1; i < vector->size; i++)
    {
        if (vector->strs[i] == NULL)
            continue;
        cbuild_str_builder_append_cstr(&str_builder, sep);
        cbuild_str_builder_append_cstr(&str_builder, vector->strs[i]);
    }
    return cbuild_str_builder_to_cstr(&str_builder);
}

void cbuild_command_add_arg(cbuild_command *command, char *arg)
{
    cbuild_str_vector_pop_back(&command->argv);
    cbuild_str_vector_add_str(&command->argv, arg);
    cbuild_str_vector_add_str(&command->argv, NULL);
}

void __cbuild_command_add_args(cbuild_command *command, char *arg, ...)
{
    cbuild_str_vector_pop_back(&command->argv);
    va_list ap;
    va_start(ap, arg);
    cbuild_str_vector_add_str(&command->argv, arg);
    while ((arg = va_arg(ap, char *)) != NULL)
    {
        cbuild_str_vector_add_str(&command->argv, arg);
    }
    cbuild_str_vector_add_str(&command->argv, NULL);
}

int pid_wait(pid_t pid)
{
    int stat_loc;
    waitpid(pid, &stat_loc, 0);
    return WEXITSTATUS(stat_loc);
}

int cbuild_command_exec_async(cbuild_command *command)
{
    assert(command->argv.size > 1);

    pid_t pid = fork();
    if (pid == 0)
    {
        char *command_str = cbuild_str_vector_join(&command->argv, " ");
        cbuild_log(CBUILD_INFO, "CMD `%s'", command_str);
        execvp(command->argv.strs[0], command->argv.strs);
        free(command_str);
        exit(127);
    }
    return pid;
}

int cbuild_command_exec_sync(cbuild_command *command)
{
    pid_t pid = cbuild_command_exec_async(command);
    if (pid == -1)
        return -1;
    return pid_wait(pid);
}

int cbuild_clean_target(cbuild_target *target)
{
    if (cbuild_file_exists(target->target_file))
    {
        cbuild_log(CBUILD_WARN, "Removing `%s'", target->target_file);
        remove(target->target_file);
    }
    for (size_t i = 0; target->sources[i].source_type; i++)
    {
        if (target->sources[i].source_type == CBUILD_TARGET_SOURCE)
        {
            cbuild_clean_target(target->sources[i].source.target);
        }
    }
}

int cbuild_build_target(cbuild_target *target, int *built, int always_recompile)
{
    int build_needed = always_recompile;
    int local_built = 0;
    if (built == NULL)
        built = &local_built;

    for (size_t i = 0; target->sources[i].source_type; i++)
    {
        if (target->sources[i].source_type == CBUILD_TARGET_SOURCE)
        {
            if (!target->sources[i].source.target->is_built && 
                    cbuild_build_target(target->sources[i].source.target, built,
                                    always_recompile) != 0)
                return 1;
            build_needed |= *built;
            build_needed |= cbuild_target_is_older_than_source(
                    target->target_file,
                    target->sources[i].source.target->target_file);
        }
        else
            build_needed |= cbuild_target_is_older_than_source(
                    target->target_file,
                    target->sources[i].source.file);
    }

    if (build_needed || *built)
    {
        cbuild_command build_command = { 0 };
        for (size_t i = 0; i < target->command.size; i++)
        {
            cbuild_command_add_arg(&build_command, target->command.strs[i]);
        }
        cbuild_command_add_args(&build_command, "-o", target->target_file);
        for (size_t i = 0; target->sources[i].source_type; i++)
        {
            if (target->sources[i].add_to_command)
                cbuild_command_add_arg(&build_command,
                    target->sources[i].source_type == CBUILD_FILE_SOURCE
                            ? target->sources[i].source.file
                            : target->sources[i].source.target->target_file);
        }
        *built = 1;
        return cbuild_command_exec_sync(&build_command);
    }
    return 0;
}

int cbuild_build_target_async(cbuild_target *target, int always_recompile)
{
    int build_needed = always_recompile;

    for (size_t i = 0; target->sources[i].source_type; i++)
    {
        if (target->sources[i].source_type == CBUILD_TARGET_SOURCE)
            build_needed |= cbuild_target_is_older_than_source(
                    target->target_file,
                    target->sources[i].source.target->target_file);
        else
            build_needed |= cbuild_target_is_older_than_source(
                    target->target_file,
                    target->sources[i].source.file);
    }

    if (build_needed)
    {
        cbuild_command build_command = { 0 };
        for (size_t i = 0; i < target->command.size; i++)
        {
            cbuild_command_add_arg(&build_command, target->command.strs[i]);
        }
        cbuild_command_add_args(&build_command, "-o", target->target_file);
        for (size_t i = 0; target->sources[i].source_type; i++)
        {
            if (target->sources[i].add_to_command)
                cbuild_command_add_arg(&build_command,
                    target->sources[i].source_type == CBUILD_FILE_SOURCE
                            ? target->sources[i].source.file
                            : target->sources[i].source.target->target_file);
        }
        return cbuild_command_exec_async(&build_command);
    }
    target->is_built = 1;
    return 0;
}

void cbuild_target_stack_push(cbuild_target_stack *sk, cbuild_target *target)
{
    cbuild_target_stack_item *sti = calloc(1, sizeof(cbuild_target_stack_item));
    sti->target = target;
    sti->next = sk->head;
    sk->head = sti;
}

void cbuild_target_map_init(cbuild_target_map *map, size_t capacity)
{
    map->capacity = capacity;
    map->items = calloc(capacity, sizeof(cbuild_target_map_item));
}

void cbuild_target_map_insert(cbuild_target_map *map, pid_t pid, cbuild_target *target)
{
    assert(map->size != map->capacity && "Map is full");
    size_t i = pid % map->capacity;
    while (map->items[i].pid != 0)
        i = (i + 1) % map->capacity;
    map->items[i].pid = pid;
    map->items[i].target = target;
}

void cbuild_target_map_remove(cbuild_target_map *map, pid_t pid)
{
    assert(map->size != map->capacity && "Map is full");
    size_t i = pid % map->capacity;
    while (map->items[i].pid != pid)
        i = (i + 1) % map->capacity;
    map->items[i].pid = 0;
    map->items[i].target = NULL;

}

cbuild_target *cbuild_target_map_get(cbuild_target_map *map, pid_t pid)
{
    assert(map->size != map->capacity && "Map is full");
    size_t i = pid % map->capacity;
    while (map->items[i].pid != pid)
        i = (i + 1) % map->capacity;
    return map->items[i].target;
}

cbuild_target *cbuild_find_buildable_target(cbuild_target_stack *targets)
{
    if (targets->head == NULL)
        return NULL;
    cbuild_target_stack_item *it = targets->head;
    cbuild_target *current = it->target;
    if (it != NULL)
    {
        size_t i = 0;
        for (; current->sources[i].source_type; i++)
        {
            if (current->sources[i].source_type == CBUILD_TARGET_SOURCE
                    && !current->sources[i].source.target->is_built)
                break;
        }
        if (current->sources[i].source_type == CBUILD_NONE)
        {
            targets->head = it->next;
            return it->target;
        }
        while (it->next != NULL)
        {
            size_t i = 0;
            current = it->next->target;
            for (; current->sources[i].source_type; i++)
            {
                if (current->sources[i].source_type == CBUILD_TARGET_SOURCE
                        && !current->sources[i].source.target->is_built)
                    break;
            }
            if (current->sources[i].source_type == CBUILD_NONE)
            {
                cbuild_target *res = it->target;
                it->next = it->next->next;
                return res;
            }
            it = it->next;
        }
    }
    return NULL;
}

void cbuild_setup_target_stack(cbuild_target *target,
        cbuild_target_stack *res)
{
    cbuild_target_stack_push(res, target);
    if (target)
    for (size_t i = 0; target->sources[i].source_type; i++)
    {
        if (target->sources[i].source_type == CBUILD_TARGET_SOURCE)
            cbuild_setup_target_stack(target->sources[i].source.target, res);
    }
}

int cbuild_multiprocess_build_target(cbuild_target *target, int *built,
        int always_recompile, unsigned nb_process)
{
    cbuild_target_stack targets = { 0 };
    cbuild_setup_target_stack(target, &targets);
    cbuild_target_map map = { 0 };
    cbuild_target_map_init(&map, nb_process);

    unsigned running_processes = 0;
    int error = 0;
    while (targets.head != NULL && !error)
    {
        while (running_processes < nb_process)
        {
            cbuild_target *to_build = cbuild_find_buildable_target(&targets);
            if (to_build == NULL)
                break;
            // TODO: goto
            pid_t pid = cbuild_build_target_async(to_build, always_recompile);
            if (pid == -1)
                break;
            if (pid == 0)
                continue;
            running_processes += 1;
            cbuild_target_map_insert(&map, pid, to_build);
        }

        int wstatus;
        pid_t pid = waitpid(-1, &wstatus, 0);
        if (pid == -1)
            break;
        running_processes -= 1;
        error = WEXITSTATUS(wstatus);
        cbuild_target *target = cbuild_target_map_get(&map, pid);
        target->is_built = 1;
        cbuild_target_map_remove(&map, pid);
    }
    // TODO: Clean up processes in goto
    return error != 0;
}

int cbuild_write_argument(char *name, char *type, char *default_value,
                          char *args, char *desc)
{
    FILE *arg_file = fopen("cargparse.h.in", "a");
    fprintf(arg_file, "ARGUMENT(%s, %s, %s, \"%s\", \"%s\")\n", name, type,
            default_value, args, desc);
    fclose(arg_file);
}

int cbuild_bootstrap_first_step(char *cbuild_source, char *cbuild_target,
        cbuild_command *build_command)
{
    if (!cbuild_file_exists("cargparse.h"))
        return 0;

    cbuild_log(CBUILD_INFO, "Initiating bootstrapping step number %d",
            cbuild_bootstrap_step);

    char *bootstrap_macro = calloc(50, sizeof(char));
    sprintf(bootstrap_macro, "-DCBUILD_BOOTSTRAP=%d", cbuild_bootstrap_step + 1);

    cbuild_command_add_args(build_command, bootstrap_macro,
            "-DCBUILD_ENABLE_CARGPARSE");

    remove("cargparse.h.in");
    cbuild_write_argument("clean", "bool", "false", "clean",
                          "clean all generated files");
    cbuild_write_argument("nb_process", "int", "1", "j",
                          "number of process that can run simultaneously");
    cbuild_write_argument("always_compile", "bool", "false", "B",
                          "recompile every targets");
#ifdef CBUILD_CUSTOM_ARGS
    CBUILD_CUSTOM_ARGS;
#endif /* CBUILD_CUSTOM_ARGS */

    return 0;
}

static const char *cbuild_log_level_strs[] = {
    [CBUILD_CLEAR] = "\x1B[0m",
    [CBUILD_INFO] = "\x1B[34m[INFO]",
    [CBUILD_DEBUG] = "\x1B[32m[DEBUG]",
    [CBUILD_WARN] = "\x1B[33m[WARNING]",
    [CBUILD_ERROR] = "\x1B[31m[ERROR]",
};

void cbuild_log(enum cbuild_log_level log_level, char *format, ...)
{
  printf("%s%s ", cbuild_log_level_strs[log_level],
         cbuild_log_level_strs[CBUILD_CLEAR]);

  va_list ap;
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  printf("\n");
}

int __cbuild_rebuild_yourself(char *cbuild_source, char *cbuild_target, char *argv[])
{
#ifndef CBUILD_ENABLE_CARGPARSE
    if (!cbuild_file_exists("cargparse.h"))
#endif
    if (!cbuild_target_is_older_than_source(cbuild_target, cbuild_source)
        && !cbuild_target_is_older_than_source(cbuild_target,
                                               cbuild_header_file_name))
    {
        return 0;
    }

    cbuild_str_builder str_builder_old = { 0 };
    cbuild_str_builder_append_cstr(&str_builder_old, cbuild_target);
    cbuild_str_builder_append_cstr(&str_builder_old, ".old");
    char *rename_cbuild_to = cbuild_str_builder_to_cstr(&str_builder_old);
    if (cbuild_rename(cbuild_target, rename_cbuild_to))
        return 1;

    cbuild_command build_command = { 0 };
    cbuild_command_add_args(&build_command, "cc");
    cbuild_command_add_args(&build_command, "-o", cbuild_target,
            cbuild_source);

#ifndef CBUILD_BOOTSTRAP
    cbuild_bootstrap_first_step(cbuild_source, cbuild_target,
            &build_command);
#elif CBUILD_BOOTSTRAP >= 1
    char bootstrap_macro[50] = { 0 };
    sprintf(bootstrap_macro, "-DCBUILD_BOOTSTRAP=%d", cbuild_bootstrap_step);
    cbuild_command_add_args(&build_command, bootstrap_macro);
#endif /* CBUILD_ENABLE_CARGPARSE */

#ifdef CBUILD_ENABLE_CARGPARSE
    cbuild_command_add_args(&build_command, "-DCBUILD_ENABLE_CARGPARSE");
#endif /* CBUILD_ENABLE_CARGPARSE */

    if (cbuild_command_exec_sync(&build_command))
    {
        cbuild_log(CBUILD_ERROR, "Could not rebuild cbuild");
        if (cbuild_rename(rename_cbuild_to, cbuild_target))
        {
            cbuild_log(CBUILD_ERROR, "Could not restore %s",
                       cbuild_target);
        }
        return 1;
    }

    free(rename_cbuild_to);

    cbuild_command run_command = { 0 };
    cbuild_command_add_arg(&run_command, cbuild_target);
    for (size_t i = 1; argv[i] != NULL; i++)
        cbuild_command_add_arg(&run_command, argv[i]);
    exit(cbuild_command_exec_sync(&run_command));
}

#endif /* ! CBUILD_IMPLEMENTATION */
#endif /* ! CBUILD_H */
