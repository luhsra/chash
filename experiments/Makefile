JOBS := $(shell nproc)
CLANG_HASH=${HOME}/clang-hash

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
int_$3:
	./timing-internal.py -s \
	-d ../versuchung-data --jobs $(JOBS) \
	-v --clang_hash-clone-url ${CLANG_HASH} \
	--project-clone-url ${$1} --cflags "-O0"

inc_$3_$2_append:
	./incremental_rebuild.py -s \
		-d ../versuchung-data --jobs $(JOBS) \
		-v --clang_hash-clone-url ${CLANG_HASH} \
		--project-clone-url ${$1} --mode $2

inc_$3_$2_touch:
	./incremental_rebuild.py -s \
		-d ../versuchung-data --jobs $(JOBS) \
		-v --clang_hash-clone-url ${CLANG_HASH} \
		--project-clone-url ${$1} --mode $2 --touch-only true

inc_$3: inc_$3_$2_touch inc_$3_$2_append
inc_$3_touch:  inc_$3_$2_touch
inc_$3_append: inc_$3_$2_append
inc: inc_$3
endef

$(foreach project,MUSL POSTGRESQL CPYTHON MBEDTLS LUA BASH SAMBA,\
	$(foreach mode,normal ccache clang-hash,\
		$(eval $(call inc_template,$(project),$(mode),$(shell echo $(project) | tr '[:upper:]' '[:lower:]' )))))

define hist_template
hist_$3_$2:
	./historical_build.py -s \
		-d tmp --jobs $(JOBS) \
		-v --clang_hash-clone-url ${CLANG_HASH} \
		--project-clone-url ${$1} --mode $2 --commits 500 -v

hist_$3: hist_$3_$2
hist_$2: hist_$3_$2

endef

$(foreach project,LUA MBEDTLS SAMBA MUSL POSTGRESQL CPYTHON BASH,\
	$(foreach mode,normal ccache clang-hash ccache-clang-hash,\
		$(eval $(call hist_template,$(project),$(mode),$(shell echo $(project) | tr '[:upper:]' '[:lower:]' )))))


define global_hist_template
glob_hist_$3_$2:
	./global_historical_build.py -s \
		-d tmp --jobs $(JOBS) \
		-v --clang_hash-clone-url ${CLANG_HASH} \
		--project-clone-url ${$1} --mode $2 --commits 2 -v

glob_hist_$3: glob_hist_$3_$2
glob_hist_$2: glob_hist_$3_$2

endef

$(foreach project,LUA,\
	$(foreach mode,clang-hash,\
		$(eval $(call global_hist_template,$(project),$(mode),$(shell echo $(project) | tr '[:upper:]' '[:lower:]' )))))


analyze:
	./analyze_results.py -d ../versuchung-data -s \
		--incremental '../versuchung-data/IncrementalCompilation-*' --dummy
