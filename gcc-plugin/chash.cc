/*
 * Copyright 2017 by Christian Dietrich <dietrich@sra.uni-hannover.de>
 *
 * This GCC plugin adds cHash support, as described by [1,2].
 *
 * [1] cHash project website
 *     https://lab.sra.uni-hannover.de/Research/cHash
 *
 * [2] cHash: Detection of Redundant Compilations via AST Hashing
 *     https://lab.sra.uni-hannover.de/Research/cHash/usenix-atc-2017.html
 */

#include <algorithm>
#include <assert.h>


#include "gcc-common.h"
#include "chash.h"

int plugin_is_GPL_compatible;

struct plugin_info chash_plugin_info = {
    .version = "42",
    .help = "cHash AST hashing\n"
};

#define dump_file stderr

#define debug_printf(args...) do {              \
        if (dump_file) {                        \
            fprintf(dump_file, args);           \
        }                                       \
    } while(0)


/*
 * Return the call graph node of fndecl.
 */
struct cgraph_node *get_fn_cnode(const_tree fndecl)
{
    gcc_assert(TREE_CODE(fndecl) == FUNCTION_DECL);
#if BUILDING_GCC_VERSION <= 4005
    return cgraph_get_node(CONST_CAST_TREE(fndecl));
#else
    return cgraph_get_node(fndecl);
#endif
}


/*
 * Pass to calculate the AST hash
 */
static unsigned int chash_ast_hash_execute() {
    using namespace ipa_icf;

    debug_printf("Hello World\n");

    return 0;
}


#define PASS_NAME chash_ast_hash
#define NO_GATE
#include "gcc-generate-simple_ipa-pass.h"

/*
 * Initialization function of this plugin: the very heart & soul.
 */
int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *version)
{
    const char * plugin_name = info->base_name;
    struct register_pass_info chash_ast_hash_info;


    if (!plugin_default_version_check(version, &gcc_version)) {
        error(G_("incompatible gcc/plugin versions"));
        return 1;
    }

    // Initialize types and the multiverse info structures.
    // FIXME: Initialize Strucutres at beginning of Translation Unit
    // register_callback(plugin_name, PLUGIN_START_UNIT, mv_info_init, &mv_ctx);

    // Register plugin information
    register_callback(plugin_name, PLUGIN_INFO, NULL, &chash_plugin_info);

    // Register pass: generate chash pass
    chash_ast_hash_info.pass = make_chash_ast_hash_pass();
    chash_ast_hash_info.reference_pass_name = "*free_lang_data";
    chash_ast_hash_info.ref_pass_instance_number = 0;
    chash_ast_hash_info.pos_op = PASS_POS_INSERT_BEFORE;
    register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &chash_ast_hash_info);

    // Finish off the generation of multiverse info
    // register_callback(plugin_name, PLUGIN_FINISH_UNIT, mv_info_finish, &mv_ctx);

    return 0;
}
