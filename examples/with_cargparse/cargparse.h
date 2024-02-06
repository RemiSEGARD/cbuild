#ifndef CARGPARSE_H
#define CARGPARSE_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @def CARG_LOCATION is a macro that should be equal to a path leading to the
 *      file containing all the ARGUMENT macros. Ideally an absolute path.
 */
#ifndef CARG_LOCATION
#  error "CARG_LOCATION is not defined"
#endif

/**
 * @brief Necessary typedef in order to get string arguments
 */
typedef char *cstr;

/**
 * @brief array of strings
 */
typedef struct
{
    char **strs; ///< The strings
    size_t size; ///< The number of strings
    size_t capacity; ///< The capacity of strs
} cargparse_str_vector;

/**
 * @brief shorthand for easier argument declaration, maybe not necessary...
 */
typedef cargparse_str_vector str_vec;

/**
 * @brief Macro used to declare the arguments
 *
 * @details This macro is redefined multiple times in order to do different
 *          things when including CARG_LOCATION, such as registering the arguments,
 *          getting their stringified type, CLI name or description...
 */
#define ARGUMENT(NAME, TYPE, DEFAULT_VALUE, ARGS, DESC) TYPE NAME;
#include CARG_LOCATION
#undef ARGUMENT

/**
 * @brief adds a string to a str_vector
 *
 * @param vec the vector
 * @param str the string
 */
void cargparse_str_vector_add_str(cargparse_str_vector *vec, char *str);
/**
 * @brief get the string at a given index in the vector
 *
 * @param vec the vector
 * @param index the index
 */
char *cargparse_str_vector_get(cargparse_str_vector *vec, size_t index);

/**
 * @brief string view implemention
 */
typedef struct
{
    const char *str; ///< the string, possibly not NULL terminated
    size_t size; ///< the size of the string
} cargparse_str_view;

/**
 * @brief compares two string views
 * 
 * @param s1 the first string view
 * @param s2 the other string view
 */
int cargparse_str_view_eq(const cargparse_str_view *s1,
                          const cargparse_str_view *s2);
/**
 * @brief creates a string view from a NULL terminated C string
 *
 * @param cstr the C string
 */
cargparse_str_view cargparse_str_view_from_cstr(const char *cstr);

/**
 * @brief splits the string into multiple string views. When there is not more
 *        possible slices, the pointer insed the return value will be equal to
 *        NULL
 *
 * @param ptr a pointer to a C string, will be modified in order to move inside
 *        the string and remember the last position
 * @param delim a string containing all possible delimiters to delimit slices
 */
cargparse_str_view cargparse_cstr_split(char **ptr, const char *delim);

/**
 * @brief a function type to parse an argument and set the value in a given
 *        pointer
 */
typedef int(*cargparse_parse_type_arg_f)(char *, void *);
/**
 * @brief an item inside a argument hashmap
 */
typedef struct cargparse_arg_map_item
{
    cargparse_str_view name; ///< The CLI argument
    cargparse_parse_type_arg_f parse_function; ///< the function to parse the 
                                               ///value
    void *data; ///< a pointer to the argument's data
    int needs_value; ///< Wether the argument needs to consume an argument for
                     ///its value
    struct cargparse_arg_map_item *next; ///< a pointer to handle collisions
                                         ///inside the map
} cargparse_arg_map_item ;

/**
 * @brief gets an arg_map_item inside the global argument map
 */
cargparse_arg_map_item *
cargparse_arg_map_item_get(const cargparse_str_view *arg_name);

/**
 * @brief argument hash map type
 */
typedef struct
{
    cargparse_arg_map_item **items; ///< all the map items
    size_t capacity; ///< the length of the items array
} cargparse_arg_map;


/**
 * @brief all possible argument parsing errors
 */
typedef enum
{
    CARGPARSE_NO_ERROR,
    CARGPARSE_UNKNOWN_ARG,
    CARGPARSE_WRONG_VALUE_TYPE,
    CARGPARSE_MISSING_VALUE,
    CARGPARSE_PRINT_HELP,
} cargparse_error;

/**
 * @brief checks if two C strings are equal
 *
 * @param s1 the first string
 * @param s2 the other string
 */
static inline int cargparse_streq(const char *s1, const char *s2);

/**
 * @brief creates a argument map item and adds it to the global argument map
 *
 * @param arg the CLI argument
 * @param parse_function the function to parse the argument's value
 * @param data the data pointing to the argument's data
 */
int cargparse_register_arg(const cargparse_str_view *arg,
                           cargparse_parse_type_arg_f parse_function,
                           void *data);
/**
 * @brief prints the help message and exits
 *
 * @param error the error that lead to calling this function
 * @param the argument that caused the error
 */
void cargparse_print_help(cargparse_error error, const char *wrong_arg);

/**
 * @brief functions used to parse a given type of argument
 */
int cargparse_parse_str_vec_arg(char *arg, void *data);
int cargparse_parse_bool_arg(char *arg, void *data);
int cargparse_parse_cstr_arg(char *arg, void *data);
int cargparse_parse_int_arg(char *arg, void *data);

/**
 * @brief setups the arguments default value and registers them inside the map
 *
 * @param usage_str the usage string to print in the help message
 */
int cargparse_setup_args(char *usage_str);

/**
 * @brief shifts the arguments and returns the shifted argument
 */
char *cargparse_shift_args(int *argc, char **argv[]);
/**
 * @brief gets the name of the argument inside a string
 *
 * @param str the string the get the name from
 * @param is_equal_arg is set to 1 if the name is followed by an '=' character
 */
cargparse_str_view cargparse_get_arg_name(const char *str, int *is_equal_arg);
/**
 * @brief parses a list of "small" arguments (-xa...)
 *
 * @param arg the list of arguments
 * @param argc pointer to the remaining number of argument
 * @param argv pointer to the remaining arguments
 */
int cargparse_parse_small_arg(char *arg, int *argc, char **argv[]);
/**
 * @brief parses an argument
 *
 * @param arg the argument to parse
 * @param argc the remaming number of arguments
 * @param argv the array of remaining arguments
 */
int cargparse_parse_argument(char *arg, int *argc, char **argv[]);
/**
 * @brief parses CLI arguments and sets the values of arguments
 *
 * @param pointer to main's argc, may get modified
 * @param pointer to main's argv, may get modified
 */
int cargparse_parse_args(int *argc, char **argv[]);

#ifdef CARGPARSE_IMPLEMENTATION /* CARGPARSE_IMPLEMENTATION */

#undef ARGUMENT
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

#include <stdlib.h>
#include <string.h>

/**
 * @brief shorthand for strcmp(s1, s2) == 0
 */
static inline int cargparse_streq(const char *s1, const char *s2)
{
    return strcmp(s1, s2) == 0;
}

void cargparse_str_vector_add_str(cargparse_str_vector *vec, char *str)
{
    if (vec->capacity == 0 && vec->strs == NULL)
    {
        vec->strs = calloc(8, sizeof(char *));
        vec->capacity = 8;
    }
    if (vec->size == vec->capacity)
    {
        vec->strs = realloc(vec->strs, vec->capacity * 2 * sizeof(char *));
        vec->capacity *= 2;
    }
    vec->strs[vec->size++] = str;
}

char *cargparse_str_vector_get(cargparse_str_vector *vec, size_t index)
{
    return vec->strs[index];
}

int cargparse_str_view_eq(const cargparse_str_view *s1,
                          const cargparse_str_view *s2)
{
    if (s1->size != s2->size)
        return 0;
    return strncmp(s1->str, s2->str, s1->size) == 0;
}

cargparse_str_view cargparse_str_view_from_cstr(const char *cstr)
{
    cargparse_str_view res = { .str = cstr, .size = strlen(cstr) };
    return res;
}

cargparse_str_view cargparse_cstr_split(char **ptr, const char *delim)
{
    cargparse_str_view sv = { .str = NULL, .size = 0 };
    while (**ptr != '\0' && strchr(delim, **ptr) != NULL)
        *ptr += 1;
    if (**ptr == '\0')
        return sv;

    sv.str = *ptr;
    while (**ptr != '\0' && strchr(delim, **ptr) == NULL)
    {
        *ptr += 1;
        sv.size += 1;
    }
    return sv;
}

cargparse_arg_map cargparse_args_data = { 0 };

static size_t cargparse_hash_str_view(const cargparse_str_view *sv)
{
    size_t hash = 0;
    for (size_t i = 0; i < sv->size; i++)
        hash = (hash ^ sv->str[i]) + sv->str[i];
    return hash;
}

int cargparse_register_arg(const cargparse_str_view *arg,
                           cargparse_parse_type_arg_f parse_function,
                           void *data)
{
    cargparse_arg_map_item *item = calloc(1, sizeof(cargparse_arg_map_item));
    item->name = *arg;
    item->parse_function = parse_function;
    item->needs_value = parse_function != cargparse_parse_bool_arg;
    item->data = data;
    size_t h = cargparse_hash_str_view(arg) % cargparse_args_data.capacity;
    if (cargparse_args_data.items[h] == NULL)
    {
        cargparse_args_data.items[h] = item;
        return 0;
    }
    item->next = cargparse_args_data.items[h];
    cargparse_args_data.items[h] = item;
    return 0;
}

char *cargparse_usage_string = NULL;

int cargparse_setup_args(char *usage_str)
{
    cargparse_usage_string = usage_str;
    cargparse_args_data.items = calloc(10, sizeof(cargparse_arg_map_item));
    cargparse_args_data.capacity = 10;

#define ARGUMENT(NAME, TYPE, DEFAULT_VALUE, ARGS, DESC)                        \
    do {                                                                       \
        NAME = DEFAULT_VALUE;                                                  \
        char *str = ARGS;                                                      \
        cargparse_str_view sv = { 0 };                                         \
        while ((sv = cargparse_cstr_split(&str, "|")).str != NULL)             \
            cargparse_register_arg(&sv, cargparse_parse_##TYPE##_arg, &NAME);  \
    } while (0);
#include CARG_LOCATION
#undef ARGUMENT
    return 0;
}

static char *error_string[] =
{
    [CARGPARSE_UNKNOWN_ARG] = "Unknown argument",
    [CARGPARSE_WRONG_VALUE_TYPE] = "Wrong value type for",
    [CARGPARSE_MISSING_VALUE] = "Missing argument value for",
};

void cargparse_print_help(cargparse_error error, const char *wrong_arg)
{
    static char *args_args[] = {
#define ARGUMENT(NAME, TYPE, DEFAULT_VALUE, ARGS, DESC) ARGS,
        "help",
#include CARG_LOCATION
#undef ARGUMENT
    };
    static char *args_types[] = {
#define ARGUMENT(NAME, TYPE, DEFAULT_VALUE, ARGS, DESC) #TYPE,
        "",
#include CARG_LOCATION
#undef ARGUMENT
    };
    static char *args_descs[] = {
#define ARGUMENT(NAME, TYPE, DEFAULT_VALUE, ARGS, DESC) DESC,
        "prints this message",
#include CARG_LOCATION
#undef ARGUMENT
    };

    if (wrong_arg != NULL && error != CARGPARSE_PRINT_HELP)
        printf("%s `%s'\n", error_string[error], wrong_arg);

    if (cargparse_usage_string != NULL)
        printf("USAGE: %s\n", cargparse_usage_string);

    puts("OPTIONS:");
    for (size_t i = 0; i < sizeof(args_args) / sizeof(*args_args); i++)
    {
        int printed = 0;
        cargparse_str_view sv = { 0 };
        char *arg = args_args[i];
        printed += printf(" ");
        while ((sv = cargparse_cstr_split(&arg, "|")).str != NULL)
        {
            if (printed > 1)
                printed += printf(",");
            if (sv.size == 1)
                printed += printf("-");
            else
                printed += printf("--");
            printed += fwrite(sv.str, sizeof(char), sv.size, stdout);
        }
        printf("%*s ", 30 - printed, "");
        if (args_types[i][0] != '\0')
            printf("(%s) ", args_types[i]);
        printf("%s\n", args_descs[i]);
    }

    exit(error == CARGPARSE_PRINT_HELP ? 0 : 1);
}

int cargparse_parse_str_vec_arg(char *arg, void *data)
{
    if (arg == NULL)
        return CARGPARSE_MISSING_VALUE;
    cargparse_str_vector vector = { 0 };
    char *str = strtok(arg, ",");
    cargparse_str_vector_add_str(&vector, str);
    while ((str = strtok(NULL, ",")) != NULL)
        cargparse_str_vector_add_str(&vector, str);
    cargparse_str_vector *dest = data;
    *dest = vector;
    return CARGPARSE_NO_ERROR;
}

int cargparse_parse_bool_arg(__attribute((unused))char *arg, void *data)
{
    bool *dest = data;
    *dest = !*dest;
    return CARGPARSE_NO_ERROR;
}

int cargparse_parse_cstr_arg(char *arg, void *data)
{
    if (arg == NULL)
        return CARGPARSE_MISSING_VALUE;
    const char **dest = data;
    *dest = arg;
    return CARGPARSE_NO_ERROR;
}

int cargparse_parse_int_arg(char *arg, void *data)
{
    if (arg == NULL)
        return CARGPARSE_MISSING_VALUE;
    int *dest = data;
    char *endptr = NULL;
    *dest = (int)strtol(arg, &endptr, 10);
    return *endptr == '\0' ? CARGPARSE_NO_ERROR : CARGPARSE_WRONG_VALUE_TYPE;
}

char *cargparse_shift_args(int *argc, char **argv[])
{
    if (*argc == 0)
        return NULL;
    char *res = (*argv)[0];
    *argc -= 1;
    *argv += 1;
    return res;
}

cargparse_arg_map_item *
cargparse_arg_map_item_get(const cargparse_str_view *arg_name) {
    size_t h = cargparse_hash_str_view(arg_name) % cargparse_args_data.capacity;
    if (cargparse_args_data.items[h] == NULL)
        return NULL;
    cargparse_arg_map_item *item = cargparse_args_data.items[h];
    while (item != NULL && !cargparse_str_view_eq(arg_name, &item->name))
        item = item->next;
    return item;
}

cargparse_str_view cargparse_get_arg_name(const char *str, int *is_equal_arg)
{
    cargparse_str_view res = { .str = str + 1, .size = 0};
    if (str[0] != '-')
        return res;
    if (str[1] == '-')
        res.str += 1;
    while (res.str[res.size] != '\0' && res.str[res.size] != '=')
        res.size += 1;

    *is_equal_arg = res.str[res.size] == '=';
    return res;
}

int cargparse_parse_small_arg(char *arg, int *argc, char **argv[])
{
    cargparse_str_view arg_name = { .str = arg + 1, .size = 1 };
    while (arg_name.str[0] != '\0')
    {
        cargparse_arg_map_item *arg_item =
                cargparse_arg_map_item_get(&arg_name);
        if (arg_item == NULL)
            return CARGPARSE_UNKNOWN_ARG;
        if (arg_item->needs_value)
            arg = cargparse_shift_args(argc, argv);
        cargparse_error error = arg_item->parse_function(arg, arg_item->data);
        if (error)
            return error;
        arg_name.str += 1;
    }
    return CARGPARSE_NO_ERROR;
}

int cargparse_parse_argument(char *arg, int *argc, char **argv[])
{
    if (arg[1] != '-')
        return cargparse_parse_small_arg(arg, argc, argv);
    int is_equal_arg = 0;
    cargparse_str_view arg_name =
        cargparse_get_arg_name(arg, &is_equal_arg);
    if (strncmp(arg_name.str, "help", arg_name.size) == 0)
        return CARGPARSE_PRINT_HELP;
    cargparse_arg_map_item *arg_item = cargparse_arg_map_item_get(&arg_name);
    if (arg_item == NULL)
        return CARGPARSE_UNKNOWN_ARG;

    if (!is_equal_arg && arg_item->needs_value)
        arg = cargparse_shift_args(argc, argv);
    else if (is_equal_arg)
        arg = strchr(arg, '=') + 1;

    return arg_item->parse_function(arg, arg_item->data);
}

int cargparse_parse_args(int *argc, char **argv[])
{
    char **argv_begin = *argv;
    int final_argc = 1;
    cargparse_shift_args(argc, argv);
    char *arg = NULL;
    while ((arg = cargparse_shift_args(argc, argv)) != NULL)
    {
        if (cargparse_streq(arg, "--"))
            break;
        if (arg[0] != '-')
        {
            argv_begin[final_argc++] = arg;
            continue;
        }

        cargparse_error error = cargparse_parse_argument(arg, argc, argv);
        if (error)
            cargparse_print_help(error, arg);
    }
    while ((arg = cargparse_shift_args(argc, argv)) != NULL)
        argv_begin[final_argc++] = arg;
    argv_begin[final_argc] = NULL;
    *argc = final_argc;
    *argv = argv_begin;
    return 0;
}

#endif /* CARGPARSE_IMPLEMENTATION */
#endif /* ! CARGPARSE_H */
