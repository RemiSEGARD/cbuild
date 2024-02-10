#define CBUILD_IMPLEMENTATION
#define CARGPARSE_HEADER "examples/with_cargparse/cargparse.h"

#define CBUILD_CUSTOM_ARGS \
    do \
        cbuild_write_argument("run_toto", "bool", "false", "toto", "should toto be run at the end?");\
    while (0)

#include "../../cbuild.h"

int main(int argc, char *argv[])
{
    CBUILD_REBUILD_YOURSELF(argc, argv);

#if CBUILD_BOOTSTRAP == 1
    cargparse_setup_args("./cbuild [OPTIONS...]");
    cargparse_parse_args(&argc, &argv);

    static cbuild_target toto_o = CBUILD_TARGET("toto.o",
            "cc -Wall -Werror -c -o %t %s",
            CBUILD_MAKE_FILE_HEADER("toto.h"),
            CBUILD_MAKE_FILE_SOURCE("toto.c"));

    static cbuild_target toto = CBUILD_TARGET("toto",
            "cc -o %t %s",
            CBUILD_MAKE_TARGET_SOURCE(&toto_o));

    cbuild_str_vector_add_strs(&toto.command, "cc", "-Wall", "-Werror");

    int rebuilt = 0;
    if (clean)
    {
        cbuild_clean_target(&toto);
        return 0;
    }
    if (cbuild_build_target(&toto, &rebuilt, always_compile))
    {
        cbuild_log(CBUILD_ERROR, "Could not build target %s", toto.target_file);
        return 1;
    }
    if (!rebuilt)
    {
        cbuild_log(CBUILD_INFO, "No need to rebuild `%s'", toto.target_file);
    }

    if (run_toto)
    {
        cbuild_command exec_toto = { 0 };
        cbuild_command_add_arg(&exec_toto, "./toto");
        return cbuild_command_exec_sync(&exec_toto);
    }
#endif /* CBUILD_BOOTSTRAP == 1 */
    return 0;
}

