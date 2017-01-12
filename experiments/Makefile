JOBS := $(shell nproc)
CLANG_HASH=${HOME}/w/clang-hash
MUSL = ${CLANG_HASH}/hash-projects/musl
POSTGRESQL = ${CLANG_HASH}/hash-projects/postgresql


all:
	@echo "Look at the makefile"

INC = ./incremental_rebuild.py -s -d ../versuchung-data --jobs $(JOBS) \
	-v --clang_hash-clone-url ${CLANG_HASH}


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
