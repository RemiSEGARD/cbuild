#define CBUILD_IMPLEMETATION
#include "../cbuild.h"

int main(int argc, char *argv[])
{
    CBUILD_REBUILD_YOURSELF(argc, argv);

    static cbuild_target toto_o = CBUILD_TARGET("toto.o",
            CBUILD_MAKE_FILE_SOURCE("toto.c"));

    cbuild_str_vector_add_strs(&toto_o.command, "cc", "-Wall", "-Werror", "-c");

    static cbuild_target toto = CBUILD_TARGET("toto",
            CBUILD_MAKE_TARGET_SOURCE(&toto_o));

    cbuild_str_vector_add_strs(&toto.command, "cc", "-Wall", "-Werror");

    if (cbuild_build_target(&toto, NULL))
    {
        cbuild_log(CBUILD_ERROR, "Could not build target %s", toto.target_file);
        return 1;
    }
    cbuild_command exec_toto = { 0 };
    cbuild_command_add_arg(&exec_toto, "./toto");
    return cbuild_command_exec_sync(&exec_toto);
}
