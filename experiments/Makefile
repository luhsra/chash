JOBS := $(shell nproc)
CLANG_HASH=${HOME}/w/clang-hash
MUSL = ${CLANG_HASH}/hash-projects/musl
POSTGRES = ${CLANG_HASH}/hash-projects/postgresql


all:
	@echo "Look at the makefile"

incremental_rebuild_postgres_normal:
	./incremental_rebuild.py -d ../versuchung-data --mode normal \
		--jobs $(JOBS) -v \
		--project-clone-url ${POSTGRES} \
		--clang_hash-clone-url ${CLANG_HASH}

incremental_rebuild_postgres_hash:
	./incremental_rebuild.py -d ../versuchung-data --mode clang-hash \
		--jobs $(JOBS) -v \
		--project-clone-url ${POSTGRES} \
		--clang_hash-clone-url ${CLANG_HASH}
