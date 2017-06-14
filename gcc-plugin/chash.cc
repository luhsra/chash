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
    struct cgraph_node *node;

    debug_printf("Hello World\n");

    FOR_EACH_DEFINED_FUNCTION(node) {
         print_full_node(*node);
    }

    return 0;
}
static void print_full_node(struct cgraph_node &node){
    std::string fname = IDENTIFIER_POINTER(DECL_NAME(node.decl));
    debug_printf("PTR: %s\n", fname.c_str());
    printf("order\t%i\n", node.order);
// struct cgraph_edge * 	callers
// struct cgraph_edge * 	indirect_calls
    printf("call_site_hash\t%i\n", node.call_site_hash);
// tree 	former_clone_of
    printf("%i", node.ipa_transforms_to_apply);
// vec< ipa_opt_pass > 	ipa_transforms_to_apply
// struct cgraph_local_info 	local
// struct cgraph_global_info 	global
// struct cgraph_rtl_info 	rtl
// struct cgraph_clone_info 	clone
// struct cgraph_thunk_info 	thunk
// gcov_type 	count
// int 	count_materialization_scale
// int 	uid
// unsigned int 	profile_id
// unsigned 	used_as_abstract_origin: 1
// unsigned 	lowered: 1
// unsigned 	process: 1
// ENUM_BITFIELD(node_frequency)
// frequency unsigned 	only_called_at_startup: 1
// unsigned 	only_called_at_exit: 1
// unsigned 	tm_clone: 1
// unsigned 	dispatcher_function: 1
// ENUM_BITFIELD(symtab_type)
// type ENUM_BITFIELD(ld_plugin_symbol_resolution)
// resolution unsigned 	definition: 1
// unsigned 	alias: 1
// unsigned 	weakref: 1
// unsigned 	cpp_implicit_alias: 1
// unsigned 	analyzed: 1
// unsigned 	externally_visible: 1
// unsigned 	force_output: 1
// unsigned 	forced_by_abi: 1
// unsigned 	unique_name: 1
// unsigned 	used_from_other_partition: 1
// unsigned 	in_other_partition: 1
// unsigned 	address_taken: 1
// int 	order
// tree 	decl
// symtab_node 	next
// symtab_node 	previous
// symtab_node 	next_sharing_asm_name
// symtab_node 	previous_sharing_asm_name
// symtab_node 	same_comdat_group
// struct ipa_ref_list 	ref_list
// tree 	alias_target
// struct lto_file_decl_data * 	lto_file_data
// PTR 	aux
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
