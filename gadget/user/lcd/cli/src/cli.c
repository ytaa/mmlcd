#include "cli.h"
#include "liblcd.h"
#include "liblcd_ipc.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* return codes */
#define CLI_RES_INVALID_ARGS 1

/* command strings */
const char * cli_cmd_strings[liblcd_ipc_cmd_count][2u] = {
    {"print", "p"},
    {"clear", "c"},
    {"backlight", "bl"},
};

/* private functions declarations */

void cli_print_usage(const char *program_name);

liblcd_ipc_cmd cli_string_to_cmd(const char *string);

int cli_handle_cmd_print(int argc, const char **argv);

int cli_handle_cmd_clear();

int cli_handle_cmd_backlight(int argc, const char **argv);

/* public functions definitions */

int cli_parse(int argc, const char **argv){
    if(2 > argc){
        cli_print_usage(argv[0]);
        return CLI_RES_INVALID_ARGS;
    }

    liblcd_ipc_cmd cmd = cli_string_to_cmd(argv[1]);

    switch(cmd){
        case liblcd_ipc_cmd_print:{
            cli_handle_cmd_print(argc, argv);
            break;
        }
        case liblcd_ipc_cmd_clear:{
            cli_handle_cmd_clear();
            break;
        }
        case liblcd_ipc_cmd_backlight:{
            cli_handle_cmd_backlight(argc, argv);
            break;
        }
        default:{
            cli_print_usage(argv[0]);
            return CLI_RES_INVALID_ARGS;
        }
    }

    return 0;
}

/* private functions definitions */

void cli_print_usage(const char *program_name){
    printf("usage: %s command arguments\n", program_name);
}

liblcd_ipc_cmd cli_string_to_cmd(const char *string){
    uint32_t cmd_idx = 0u;

    for(cmd_idx = 0u; cmd_idx < (uint32_t)liblcd_ipc_cmd_count; ++cmd_idx){
        if(strcmp(string, cli_cmd_strings[cmd_idx][0u]) == 0 || 
           strcmp(string, cli_cmd_strings[cmd_idx][1u]) == 0 ){
            break;
        }
    }

    return (liblcd_ipc_cmd)cmd_idx;
}

int cli_handle_cmd_print(int argc, const char **argv){
    if(3 > argc){
        cli_print_usage(argv[0]);
        return CLI_RES_INVALID_ARGS;
    }

    uint8_t line = 0u;
    uint8_t pos = 0u;

    if(4 <= argc){
        line = atoi(argv[3]);
    }
    if(5 <= argc){
        pos = atoi(argv[4]);
    }

    return liblcd_display_string_pos(argv[2], line, pos);
}

int cli_handle_cmd_clear(){
    return liblcd_clear();
}

int cli_handle_cmd_backlight(int argc, const char **argv){
    if(3 > argc){
        cli_print_usage(argv[0]);
        return CLI_RES_INVALID_ARGS;
    }

    bool enable = false;

    if(strcmp(argv[2], "on") == 0 || strcmp(argv[2], "1") == 0){
        enable = true;
    }
    else if(strcmp(argv[2], "off") == 0 || strcmp(argv[2], "0") == 0){
        enable = false;
    }
    else{
        cli_print_usage(argv[0]);
        return CLI_RES_INVALID_ARGS;
    }

    return liblcd_backlight(enable);
}