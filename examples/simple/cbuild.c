#define CBUILD_IMPLEMENTATION
#include "../../cbuild.h"

#include <string.h>

int main(int argc, char *argv[])
{
    CBUILD_REBUILD_YOURSELF(argc, argv);

    static cbuild_target toto_o = CBUILD_TARGET("toto.o",
            "cc -Wall -Werror -c -o %t %s",
            CBUILD_MAKE_FILE_HEADER("toto.h"),
            CBUILD_MAKE_FILE_SOURCE("toto.c"));

    static cbuild_target toto = CBUILD_TARGET("toto",
            "cc -Wall -Werror -o %t %s",
            CBUILD_MAKE_TARGET_SOURCE(&toto_o));

    if (argc > 1 && strcmp("--clean", argv[1]) == 0)
    {
        cbuild_clean_target(&toto);
        return 0;
    }
    else if (cbuild_build_target(&toto, NULL, 0))
    {
        cbuild_log(CBUILD_ERROR, "Could not build target %s", toto.target_file);
        return 1;
    }
    cbuild_command exec_toto = { 0 };
    cbuild_command_add_arg(&exec_toto, "./toto");
    return cbuild_command_exec_sync(&exec_toto);
}

