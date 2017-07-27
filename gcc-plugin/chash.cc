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
#include "Hash.h"

#include "gcc-common.h"
#include "chash.h"
#include "tree.h"

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
extern "C" void chash_ast_hash_execute(void *gcc_data, void*) {
    tree fndecl = (tree)gcc_data;
    
    int code = TREE_CODE(fndecl);
    if(code==FUNCTION_DECL){
        tree rType = TREE_TYPE(DECL_RESULT(fndecl)),   
            arguments = DECL_ARGUMENTS(fndecl),
            body = DECL_SAVED_TREE(fndecl);
        printf("Function decl:\n");
        printf("Name:\t\t\t\t\t%s\n", IDENTIFIER_POINTER(DECL_NAME(fndecl)));
        printf("Type of return value:\t\t\t%s\n", get_tree_code_name(TREE_CODE(rType)));
        tree_statement_list_node* tmp= STATEMENT_LIST_HEAD(body);
        // printf("bodyparsing\n");
        // while(tmp!=0){
        //     int i=0;
        //     tree tmp2=TREE_OPERAND(tmp->stmt,i);
        //     while(tmp2!=0)
        //     {
        //         printf("%s\n", get_tree_code_name(TREE_CODE(tmp2)));
        //         tmp2 = TREE_OPERAND(tmp->stmt,++i);
        //     }
        //     tmp= tmp->next;
        // }
        // printf("bodyparsing\n");
        printf("<%s ", get_tree_code_name(TREE_CODE(body)));
        printf("%p\n", body);
        printf("\ttype <%s ", get_tree_code_name(TREE_CODE(TREE_TYPE(body))));
        printf("%p\n", TREE_TYPE(body));
        printf("\t\talign %i ", TYPE_ALIGN(TREE_TYPE(body)));
        printf("canonical type %p\n", TYPE_CANONICAL(TREE_TYPE(body)));
        printf("\t\tpointer_to_this <%s ", get_tree_code_name(TREE_CODE(TYPE_POINTER_TO(TREE_TYPE(body)))));
        printf("%p>>\n", TYPE_POINTER_TO(TREE_TYPE(body)));

        printf("\t%s ", TREE_SIDE_EFFECTS(body)?"side-effects":"");
        printf("head %p ", tmp);
        printf("tail %p ", STATEMENT_LIST_TAIL(body));
        printf("stmts %p ", tmp->stmt);
        printf("%p\n\n", STATEMENT_LIST_TAIL(body)->stmt);
        // printf("%p\n", debug_tree(TREE_SIDE_EFFECTS(body)));
        debug_tree(body);
        // debug_tree(COMPOUND_STMT(body));
        // if(body != 0){
        //     printf("****\n");
        //     //printf(":\t\t\t%s\n", get_tree_code_name(TREE_CODE(body)));
        //     // tree chain = BLOCK_VARS(body);
        //     // // chain = chain ? BLOCK_CHAIN(chain) : 0;
        //     // printf("%s\n", chain == 0 ? "no chain": "chain");
        //     // while (chain != 0){
        //     //     printf("%s %p ", get_tree_code_name(TREE_CODE(chain)), chain);
        //     //     chain = TREE_CHAIN(chain);
        //     // }
        //     printf("\n");
        // }
        

        // while(1){ // Will only run once. Special attributes.
        //     if (DECL_FUNCTION_SPECIFIC_TARGET (fndecl)){
        //         printf("%s\n", "1");
        //         break;
        //     }
        //     if (DECL_FUNCTION_SPECIFIC_OPTIMIZATION (fndecl)){
        //         printf("%s\n", "2");
        //         break;
        //     }
        //     if (DECL_DECLARED_INLINE_P (fndecl)){
        //         printf("%s\n", "inlined");
        //         break;
        //     }
        //     if (DECL_BUILT_IN (fndecl)){
        //         printf("%s\n", "4");
        //         break;
        //         }
        //     if (DECL_STATIC_CHAIN (fndecl)){
        //         printf("%s\n", "5");
        //         break;
        //     }
        //     if (decl_is_tm_clone (fndecl)){
        //         printf("%s\n", "6");
        //         break;
        //     }
        //     printf("No special Attributes\n");
        //     break;
        // }

        // // while(body != 0 ){
        // //     printf(":\t\t\t%s\n", IDENTIFIER_POINTER(DECL_NAME(body)));
        // //     body = TREE_CHAIN(body);
        // // }
        // while(arguments!=0){ // Parse all arguments of the fndecltion
        //     printf("%s\n", get_tree_code_name(TREE_CODE(DECL_ARG_TYPE(arguments))));
        //     arguments = TREE_CHAIN(arguments);
        // }
        printf("\n");
    }
        // // int dExt = DECL_EXTERNAL(node->decl);
        // // printf("fndecltion is undefined?\t\t\t%s\n", dExt == 1 ? "true" : dExt == 0 ? "false" : "cHash Error");
        // // int tPub = TREE_PUBLIC(node->decl);
        // // printf("fndecltion has external linkage?\t\t%s\n", tPub == 1 ? "true" : tPub == 0 ? "false" : "cHash Error");
        // // int tStatic = TREE_STATIC(node->decl);
        // // printf("fndecltion has been defined?\t\t%s\n", tStatic == 1 ? "true" : tStatic == 0 ? "false" : "cHash Error");
        // // int tVol = TREE_THIS_VOLATILE(node->decl);
        // // printf("fndecltion does not return normally?\t%s\n", tVol == 1 ? "true" : tVol == 0 ? "false" : "cHash Error");
        // // int tRO = TREE_READONLY(node->decl);
        // // printf("fndecltion can only read its arguments?\t%s\n", tRO == 1 ? "true" : tRO == 0 ? "false" : "cHash Error");
        // // int dPureP = DECL_PURE_P(node->decl);
        // // printf("fndecltion can only read its arguments, but may read globally?\t%s\n", dPureP == 1 ? "true" : dPureP == 0 ? "false" : "cHash Error");
        // // int dVirP = DECL_VIRTUAL_P(node->decl);
        // // printf("fndecltion is virtual?\t\t\t%s\n", dVirP == 1 ? "true" : dVirP == 0 ? "false" : "cHash Error");
        // // int dArt = DECL_ARTIFICIAL(node->decl);
        // // printf("fndecltion was implicitly generated by the compiler, rather than explicitly declared. In addition to implicitly generated class member fndecltions, this macro holds for the special fndecltions created to implement static initialization and destruction, to compute run-time type information, and so forth.\t%s\n", dArt == 1 ? "true" : dArt == 0 ? "false" : "o no");
        // // printf("%p\n", DECL_fndeclTION_SPECIFIC_TARGET(node->decl)); // ??
        // // printf("%p\n", DECL_fndeclTION_SPECIFIC_OPTIMIZATION(node->decl)); // ??
        // // printf("Mangled name of the fndecltion.\t\t%p\n", DECL_ASSEMBLER_NAME(node->decl)); // ??
        // // printf("First parameter type.\t\t\t%p\n", DECL_ARGUMENTS(node->decl)); // ??
        // // printf("First parameter type.\t\t\t%p\n", DECL_ARGUMENTS(node->decl)); // ??
        // // printf("fndecltion return type.\t\t\t%p\n", DECL_RESULT(node->decl)); // ??
        // // printf("fndecltion return type.\t\t\t%p\n", TYPE_CANONICAL(DECL_RESULT(node->decl))); // ??
        // // woher bekomme ich vergleichswerte?
        // // printf("null: %i, enum: %i, bool: %i, int: %i, real: %i, pointer: %i, reference: %i, fixed point: %i, complex: %i, void: %i, fndecltion: %i\n", NULL_TREE, ENUMERAL_TYPE, BOOLEAN_TYPE, INTEGER_TYPE, REAL_TYPE, POINTER_TYPE, REFERENCE_TYPE, FIXED_POINT_TYPE, COMPLEX_TYPE, VOID_TYPE, fndeclTION_TYPE);
        // // printf("fndecltion body.\t\t\t\t%p\n", DECL_SAVED_TREE(node->decl)); // ??
        // // printf("fndecltion type.\t\t\t\t%p\n", &(TREE_TYPE(node->decl))); // ??
        // printf("\n");

       
        //debug_tree(((union tree_node *)(const union tree_node *)(node->decl)));
    //}
}


void hash_expr(tree expr) {
    if (TREE_CODE(expr) == BIND_EXPR) {
        TREE_TYPE(expr);
        
    }
}
/*
 * Initialization fndecltion of this plugin: the very heart & soul.
 */
int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *version)
{
    const char * plugin_name = info->base_name;
    struct register_pass_info chash_ast_hash_info;
    (void) chash_ast_hash_info;


    if (!plugin_default_version_check(version, &gcc_version)) {
        error(G_("incompatible gcc/plugin versions"));
        return 1;
    }

    // Initialize types and the multiverse info structures.
    // FIXME: Initialize Strucutres at beginning of Translation Unit
    // register_callback(plugin_name, PLUGIN_START_UNIT, mv_info_init, &mv_ctx);

    // Register plugin information
    register_callback(plugin_name, PLUGIN_INFO, NULL, &chash_plugin_info);

    // Register callbacks.
    register_callback (plugin_name,
		        PLUGIN_PRE_GENERICIZE,
		        &chash_ast_hash_execute,
                NULL);
    // Register pass: generate chash pass
    //chash_ast_hash_info.pass = make_chash_ast_hash_pass();
    //chash_ast_hash_info.reference_pass_name = "gimplify";
    //chash_ast_hash_info.ref_pass_instance_number = 0;
    //chash_ast_hash_info.pos_op = PASS_POS_INSERT_BEFORE;
    //register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &chash_ast_hash_info);

    // Finish off the generation of multiverse info
    // register_callback(plugin_name, PLUGIN_FINISH_UNIT, mv_info_finish, &mv_ctx);

    return 0;
}
