#include <string.h>
#define CBUILD_IMPLEMENTATION
#include "../../cbuild.h"

int main(int argc, char *argv[])
{
    CBUILD_REBUILD_YOURSELF(argc, argv);

    static cbuild_target file1_o = CBUILD_TARGET("file1.o",
            "cc -c -o %t %s",
            CBUILD_MAKE_FILE_SOURCE("file1.c")
            );
    static cbuild_target file2_o = CBUILD_TARGET("file2.o",
            "cc -c -o %t %s",
            CBUILD_MAKE_FILE_SOURCE("file2.c")
            );
    static cbuild_target file3_o = CBUILD_TARGET("file3.o",
            "cc -c -o %t %s",
            CBUILD_MAKE_FILE_SOURCE("file3.c")
            );
    static cbuild_target file4_o = CBUILD_TARGET("file4.o",
            "cc -c -o %t %s",
            CBUILD_MAKE_FILE_SOURCE("file4.c")
            );
    static cbuild_target file5_o = CBUILD_TARGET("file5.o",
            "cc -c -o %t %s",
            CBUILD_MAKE_FILE_SOURCE("file5.c")
            );
    static cbuild_target main_o = CBUILD_TARGET("main.o",
            "cc -c -o %t %s",
            CBUILD_MAKE_FILE_SOURCE("main.c")
            );
    static cbuild_target _main = CBUILD_TARGET("main",
            "cc -o %t %s",
            CBUILD_MAKE_TARGET_SOURCE(&file1_o),
            CBUILD_MAKE_TARGET_SOURCE(&file2_o),
            CBUILD_MAKE_TARGET_SOURCE(&file3_o),
            CBUILD_MAKE_TARGET_SOURCE(&file4_o),
            CBUILD_MAKE_TARGET_SOURCE(&file5_o),
            CBUILD_MAKE_TARGET_SOURCE(&main_o)
            );

    if (argc > 1 && strcmp(argv[1], "--clean") == 0)
    {
        cbuild_clean_target(&_main);
        return 0;
    }

    if (cbuild_multiprocess_build_target(&_main, NULL, 0, 4))
    {
        cbuild_log(CBUILD_ERROR, "Could not build target `main'");
        return 1;
    }
    return 0;
}
