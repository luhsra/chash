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
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "tree.h"
#include "gimple.h"
#include "cfghooks.h"
#include "tree-pass.h"
#include "ssa.h"
#include "cgraph.h"
#include "diagnostic-core.h"
#include "fold-const.h"
#include "varasm.h"
#include "tree-nested.h"
#include "gimplify.h"
#include "gimple-iterator.h"
#include "gimplify-me.h"
#include "tree-cfg.h"
#include "tree-into-ssa.h"
#include "value-prof.h"
#include "profile.h"
#include "tree-cfgcleanup.h"
#include "params.h"

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
    gcc_assert (TREE_CODE (fndecl) == FUNCTION_DECL);

    printf("-----------------------------------------------------\n");
    debug_tree(fndecl); //== \/
    printf("****\n");
    debug_tree(DECL_SAVED_TREE (fndecl));

        // enum tree_code code = TREE_CODE(node->decl);
        // if(code==FUNCTION_DECL){
        //     tree func = node->decl,                     //function decl
        //         rType = TREE_TYPE(DECL_RESULT(func)),   
        //         arguments = DECL_ARGUMENTS(func),
        //         body = DECL_SAVED_TREE(func);
        //         //if (!body) body = DECL_INITIAL(func);       
        //     printf("Function decl:\n");
        //     printf("Name:\t\t\t\t\t%s\n", IDENTIFIER_POINTER(DECL_NAME(func)));
        //     printf("Type of return value:\t\t\t%s\n", get_tree_code_name(TREE_CODE(rType)));
        //     printf("%s\n", body==0?"leer":"nicht leer");
        //     if(body != 0){
        //         printf("****\n");
        //         debug_tree(body);
                
        //         //printf(":\t\t\t%s\n", get_tree_code_name(TREE_CODE(body)));
        //         tree chain = BLOCK_VARS(body);
        //         chain = chain ? BLOCK_CHAIN(chain) : 0;
        //         printf("%s\n", chain == 0 ? "no chain": "chain");
        //         while (chain != 0){
        //             printf("%s %p ", get_tree_code_name(TREE_CODE(chain)), chain);
        //             chain = TREE_CHAIN(chain);
        //         }
        //         printf("\n");
        //     }
            

        //     while(1){ // Will only run once. Special attributes.
        //         if (DECL_FUNCTION_SPECIFIC_TARGET (func)){
        //             printf("%s\n", "1");
        //             break;
        //         }
        //         if (DECL_FUNCTION_SPECIFIC_OPTIMIZATION (func)){
        //             printf("%s\n", "2");
        //             break;
        //         }
        //         if (DECL_DECLARED_INLINE_P (func)){
        //             printf("%s\n", "inlined");
        //             break;
        //         }
        //         if (DECL_BUILT_IN (func)){
        //             printf("%s\n", "4");
        //             break;
        //             }
        //         if (DECL_STATIC_CHAIN (func)){
        //             printf("%s\n", "5");
        //             break;
        //         }
        //         if (decl_is_tm_clone (func)){
        //             printf("%s\n", "6");
        //             break;
        //         }
        //         printf("No special Attributes\n");
        //         break;
        //     }

        //     // while(body != 0 ){
        //     //     printf(":\t\t\t%s\n", IDENTIFIER_POINTER(DECL_NAME(body)));
        //     //     body = TREE_CHAIN(body);
        //     // }
        //     while(arguments!=0){ // Parse all arguments of the function
        //         printf("%s\n", get_tree_code_name(TREE_CODE(DECL_ARG_TYPE(arguments))));
        //         arguments = TREE_CHAIN(arguments);
        //     }
        // }
        // // int dExt = DECL_EXTERNAL(node->decl);
        // // printf("Function is undefined?\t\t\t%s\n", dExt == 1 ? "true" : dExt == 0 ? "false" : "cHash Error");
        // // int tPub = TREE_PUBLIC(node->decl);
        // // printf("Function has external linkage?\t\t%s\n", tPub == 1 ? "true" : tPub == 0 ? "false" : "cHash Error");
        // // int tStatic = TREE_STATIC(node->decl);
        // // printf("Function has been defined?\t\t%s\n", tStatic == 1 ? "true" : tStatic == 0 ? "false" : "cHash Error");
        // // int tVol = TREE_THIS_VOLATILE(node->decl);
        // // printf("Function does not return normally?\t%s\n", tVol == 1 ? "true" : tVol == 0 ? "false" : "cHash Error");
        // // int tRO = TREE_READONLY(node->decl);
        // // printf("Function can only read its arguments?\t%s\n", tRO == 1 ? "true" : tRO == 0 ? "false" : "cHash Error");
        // // int dPureP = DECL_PURE_P(node->decl);
        // // printf("Function can only read its arguments, but may read globally?\t%s\n", dPureP == 1 ? "true" : dPureP == 0 ? "false" : "cHash Error");
        // // int dVirP = DECL_VIRTUAL_P(node->decl);
        // // printf("Function is virtual?\t\t\t%s\n", dVirP == 1 ? "true" : dVirP == 0 ? "false" : "cHash Error");
        // // int dArt = DECL_ARTIFICIAL(node->decl);
        // // printf("Function was implicitly generated by the compiler, rather than explicitly declared. In addition to implicitly generated class member functions, this macro holds for the special functions created to implement static initialization and destruction, to compute run-time type information, and so forth.\t%s\n", dArt == 1 ? "true" : dArt == 0 ? "false" : "o no");
        // // printf("%p\n", DECL_FUNCTION_SPECIFIC_TARGET(node->decl)); // ??
        // // printf("%p\n", DECL_FUNCTION_SPECIFIC_OPTIMIZATION(node->decl)); // ??
        // // printf("Mangled name of the function.\t\t%p\n", DECL_ASSEMBLER_NAME(node->decl)); // ??
        // // printf("First parameter type.\t\t\t%p\n", DECL_ARGUMENTS(node->decl)); // ??
        // // printf("First parameter type.\t\t\t%p\n", DECL_ARGUMENTS(node->decl)); // ??
        // // printf("Function return type.\t\t\t%p\n", DECL_RESULT(node->decl)); // ??
        // // printf("Function return type.\t\t\t%p\n", TYPE_CANONICAL(DECL_RESULT(node->decl))); // ??
        // // woher bekomme ich vergleichswerte?
        // // printf("null: %i, enum: %i, bool: %i, int: %i, real: %i, pointer: %i, reference: %i, fixed point: %i, complex: %i, void: %i, function: %i\n", NULL_TREE, ENUMERAL_TYPE, BOOLEAN_TYPE, INTEGER_TYPE, REAL_TYPE, POINTER_TYPE, REFERENCE_TYPE, FIXED_POINT_TYPE, COMPLEX_TYPE, VOID_TYPE, FUNCTION_TYPE);
        // // printf("Function body.\t\t\t\t%p\n", DECL_SAVED_TREE(node->decl)); // ??
        // // printf("Function type.\t\t\t\t%p\n", &(TREE_TYPE(node->decl))); // ??
        // printf("\n");

       
        //debug_tree(((union tree_node *)(const union tree_node *)(node->decl)));
    //}

/*
DECL_EXTERNAL

    This predicate holds if the function is undefined.
TREE_PUBLIC

    This predicate holds if the function has external linkage.
TREE_STATIC

    This predicate holds if the function has been defined.
TREE_THIS_VOLATILE

    This predicate holds if the function does not return normally.
TREE_READONLY

    This predicate holds if the function can only read its arguments.
DECL_PURE_P

    This predicate holds if the function can only read its arguments, but may also read global memory.
DECL_VIRTUAL_P

    This predicate holds if the function is virtual.
DECL_ARTIFICIAL

    This macro holds if the function was implicitly generated by the compiler, rather than explicitly declared. In addition to implicitly generated class member functions, this macro holds for the special functions created to implement static initialization and destruction, to compute run-time type information, and so forth.
DECL_FUNCTION_SPECIFIC_TARGET

    This macro returns a tree node that holds the target options that are to be used to compile this particular function or NULL_TREE if the function is to be compiled with the target options specified on the command line.
DECL_FUNCTION_SPECIFIC_OPTIMIZATION

    This macro returns a tree node that holds the optimization options that are to be used to compile this particular function or NULL_TREE if the function is to be compiled with the optimization options specified on the command line.
DECL_NAME

    This macro returns the unqualified name of the function, as an IDENTIFIER_NODE. For an instantiation of a function template, the DECL_NAME is the unqualified name of the template, not something like f<int>. The value of DECL_NAME is undefined when used on a constructor, destructor, overloaded operator, or type-conversion operator, or any function that is implicitly generated by the compiler. See below for macros that can be used to distinguish these cases.
DECL_ASSEMBLER_NAME

    This macro returns the mangled name of the function, also an IDENTIFIER_NODE. This name does not contain leading underscores on systems that prefix all identifiers with underscores. The mangled name is computed in the same way on all platforms; if special processing is required to deal with the object file format used on a particular platform, it is the responsibility of the back end to perform those modifications. (Of course, the back end should not modify DECL_ASSEMBLER_NAME itself.)

    Using DECL_ASSEMBLER_NAME will cause additional memory to be allocated (for the mangled name of the entity) so it should be used only when emitting assembly code. It should not be used within the optimizers to determine whether or not two declarations are the same, even though some of the existing optimizers do use it in that way. These uses will be removed over time.
DECL_ARGUMENTS

    This macro returns the PARM_DECL for the first argument to the function. Subsequent PARM_DECL nodes can be obtained by following the TREE_CHAIN links.
DECL_RESULT

    This macro returns the RESULT_DECL for the function.
DECL_SAVED_TREE

    This macro returns the complete body of the function.
TREE_TYPE

    This macro returns the FUNCTION_TYPE or METHOD_TYPE for the function.
*/
}

//#define PASS_NAME chash_ast_hash
//#define NO_GATE
//#include "gcc-generate-simple_ipa-pass.h"

void hash_expr(tree expr) {
    if (TREE_CODE(expr) == BIND_EXPR) {
        hash_type(TREE_TYPE(expr));
        
    }
}
/*
 * Initialization function of this plugin: the very heart & soul.
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
