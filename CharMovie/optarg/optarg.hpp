/*
optarg 2.0.0
Copyright (C) 2021, GrapeJuice.
GitHub: https://github.com/GrapeJuicer/optarg
*/
#define _CRT_SECURE_NO_WARNINGS
#pragma once

// include ---------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h> // strchr()
#include <stdbool.h>
#include <wctype.h> // iswalnum()

// Definition -----------------------------------------------

// termination
#define OPT_END    {0, 0, 0, 0}     // for 'struct option'
#define DOCOPT_END {-1, 0, 0, 0, 0} // for 'struct docoption

// safe free macro
#define SFREE(ptr) { free(ptr); ptr = NULL; }

// for argument 'usage_arg' in printHelp()
#define DEFAULT_USAGE "[OPTION] ..."

/*
struct docstyle style = {
    .indent     = "  ",
    .separator  = " , ",
    .margin     = "        ",
    .doc_width  = 40
};
*/

// for argument 'style' in printHelp()
#define DEFAULT_STYLE (struct docstyle){ .indent = "  ", .separator = " , ", .margin = "        ", .doc_width = 40 }

// Structure ---------------------------------------------

struct optarg
{
    int    opt; // option value
    char * arg; // option's argument
};

struct docoption
{
    const int    val;        // option value (like id)
    const int    short_name; // option short name (like 'h')
    const char * long_name;  // option long name (like 'help')
    const int    has_arg;    // option has argument: no_argument, required_argument, optional_argument
    const char * help_msg;   // description for option. it is used when you use 'printHelp()'
};

struct docstyle
{
    const char * indent;    // default: "  "
    const char * separator; // default: " , "
    const char * margin;    // default: "        "
    int          doc_width; // default: 40
};

// prototype -----------------------------------

// optarg.cpp

int getopt_once(
    int                   argc,        // number of arguments
    char *                argv[],      // arguments
    const char *          shortopts,   // short options
    const struct option * longopts,    // long options
    struct optarg         findopts[],  // found options
    int                   len_findopts // findopts array size
);

// getopt_flex.cpp

int getopt_flex(
    int                      argc,         // number of arguments
    char **                  argv,         // arguments
    const struct docoption * docopts,      // options
    struct optarg *          findopts,     // found options and arguments
    size_t                   findopts_size // findopts array size
);

int  __initOpts            (char **shortopts, size_t shortopts_size, struct option **longopts, size_t longopts_size);
void __flushOpts           (char *shortopts, struct option *longopts);
int  __optSize             (const struct docoption *opts);
int  __generateLongOption  (const struct docoption *docopts, size_t docopts_size, struct option *longopts, size_t longopts_size);
int  __generateShortOption (const struct docoption *docopts, size_t docopts_size, char *shortopts, size_t shortopts_size);
int  __calShortOptsLength  (const struct docoption *docopts, size_t docopts_size);
int  __convertOption       (const struct docoption *docopts, struct option **longopts, char **shortopts);
int  __isEnd               (const struct docoption opt);

// getopt_help.cpp

int printHelp(
    const struct docoption * docopts,   // options
    const char *             progname,  // program name. normally, argv[0]
    const char *             usage_arg, // usage argument. normally, "[OPTION] ..."
    const struct docstyle    style,     // help message style
    const char *             header,    // header. ex: "Optional Usage: ..."
    const char *             footer     // footer. ex: "Copyright (C) 2021 GrapeJuice."
);

void printVersion(
    const char * progname,  // program name. normally, argv[0]
    const char * version,   // version. ex: "1.0.0"
    const char * postscript // postscript. ex: "Copyright (C) 2020 GrapeJuice."
);

void __printUsage               (const char *progname, const char *usage_arg);
int  __shortOptionCommandLength (const struct docoption *docopts, int docopts_size);
size_t  __longOptionCommandLength  (const struct docoption *docopts, int docopts_size);
//int  __optionCommandLength      (const struct docoption *docopts, int docopts_size);
int  __printOptionCommand       (const struct docoption opt, size_t slen, size_t llen, struct docstyle style);
void __printOptionDocs          (const char *docs, size_t indent, size_t min_line_size);
size_t  __calIndentSize            (size_t slen, size_t llen, const struct docstyle style);

