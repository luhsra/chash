JOBS := $(shell nproc)
CLANG_HASH=${HOME}/w/clang-hash
MUSL = ${CLANG_HASH}/hash-projects/musl
POSTGRESQL = ${CLANG_HASH}/hash-projects/postgresql
CPYTHON = ${CLANG_HASH}/hash-projects/cpython
MBEDTLS = ${CLANG_HASH}/hash-projects/mbedtls --project-ref development



all:
	@echo "Look at the makefile"

INC = ./incremental_rebuild.py -s -d ../versuchung-data --jobs $(JOBS) \
	-vv --clang_hash-clone-url ${CLANG_HASH} --dummy


inc_postgres_normal:
	${INC} --project-clone-url ${POSTGRESQL} --mode normal

inc_postgres_ccache:
	${INC} --project-clone-url ${POSTGRESQL} --mode ccache

inc_postgres_hash:
	${INC} --project-clone-url ${POSTGRESQL} --mode clang-hash

inc_musl_normal:
	${INC} --project-clone-url ${MUSL} --mode normal

inc_musl_ccache:
	${INC} --project-clone-url ${MUSL} --mode ccache

inc_musl_hash:
	${INC} --project-clone-url ${MUSL} --mode clang-hash

inc_cpython_normal:
	${INC} --project-clone-url ${CPYTHON} --mode normal

inc_cpython_ccache:
	${INC} --project-clone-url ${CPYTHON} --mode ccache

inc_cpython_hash:
	${INC} --project-clone-url ${CPYTHON} --mode clang-hash

inc_mbedtls_normal:
	${INC} --project-clone-url ${MBEDTLS} --mode normal

inc_mbedtls_ccache:
	${INC} --project-clone-url ${MBEDTLS} --mode ccache

inc_mbedtls_hash:
	${INC} --project-clone-url ${MBEDTLS} --mode clang-hash
