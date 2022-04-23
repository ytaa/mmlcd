#include <stdio.h>
#include <liblcd.h>

#include "cli.h"

int main(int argc, const char **argv){
    if(0 > liblcd_init()){
        fprintf(stderr, "Failed to initialize liblcd\n");
        return 1;
    }

    cli_parse(argc, argv);

    liblcd_deinit();

    return 0;
}