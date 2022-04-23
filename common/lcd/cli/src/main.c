#include <stdio.h>
#include <libmmlcd.h>

#include "cli.h"

int main(int argc, const char **argv){
    if(0 > liblcd_init()){
        fprintf(stderr, "Failed to initialize libmmlcd\n");
        return 1;
    }

    int res = cli_parse(argc, argv);

    liblcd_deinit();

    return res;
}