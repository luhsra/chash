JOBS := $(shell nproc)
CLANG_HASH=${HOME}/w/clang-hash

SAMBA = ${CLANG_HASH}/hash-projects/samba
BASH = ${CLANG_HASH}/hash-projects/bash
LUA = ${CLANG_HASH}/hash-projects/lua
MUSL = ${CLANG_HASH}/hash-projects/musl
POSTGRESQL = ${CLANG_HASH}/hash-projects/postgresql
CPYTHON = ${CLANG_HASH}/hash-projects/cpython
MBEDTLS = ${CLANG_HASH}/hash-projects/mbedtls --project-ref development

all:
	@echo "Look at the makefile"

define inc_template
inc_$(shell echo $1 | tr '[:upper:]' '[:lower:]' )_$2:
	./incremental_rebuild.py -s \
		-d ../versuchung-data --jobs $(JOBS) \
		-v --clang_hash-clone-url ${CLANG_HASH} \
		--project-clone-url ${$1} --mode $2

inc_$(shell echo $1 | tr '[:upper:]' '[:lower:]' ): inc_$(shell echo $1 | tr '[:upper:]' '[:lower:]' )_$2
endef

$(foreach project,MUSL POSTGRESQL CPYTHON MBEDTLS LUA BASH SAMBA,\
	$(foreach mode,normal ccache clang-hash,\
		$(eval $(call inc_template,$(project),$(mode)))))
