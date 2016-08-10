{{A}}
{{B}}

int foo() {
    return 23;
}

/*
 * check-name: Different Compiler Invocations
 * compile-flags-A:
 * compile-flags-B: -ffunction-sections
 */
