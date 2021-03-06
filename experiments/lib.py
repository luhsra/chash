import os
import fnmatch
import time
from versuchung.execute import shell, CommandFailed, shell_failok
import logging
import tempfile

def read_hash_directory(hash_dir, remove_keys = []):
    """Read in all records from a hash dir
    """
    ret = []
    for root, dirnames, filenames in os.walk(hash_dir):
        for filename in fnmatch.filter(filenames, '*.info'):
            with open(os.path.join(root, filename)) as fd:
                data = "[%s]" % (",".join(fd.readlines()))
                data = eval(data)
                for record in data:
                    for key in remove_keys:
                        del record[key]
                ret.extend(data)
                print len(ret)
    return ret


class ClangHashHelper:
    def project_name(self):
        return os.path.basename(self.metadata['project-clone-url'])

    def setup_compiler_paths(self, clang_path):
        if "ccache" in self.mode.value:
            cache_dir = os.path.join(self.tmp_directory.path, "ccache")
            os.mkdir(cache_dir)
            os.environ["CCACHE_DIR"] = cache_dir
        if "clang-hash" in self.mode.value:
            cache_dir = os.path.join(self.tmp_directory.path, "clang-hash-cache")
            os.mkdir(cache_dir)
            os.environ["CLANG_HASH_CACHE"] = cache_dir

        if self.mode.value == "normal":
            CC = os.path.join(clang_path, "build/wrappers/clang-normal")
        elif self.mode.value == "clang-hash":
        #    CC = os.path.join(clang_path, "build/wrappers/clang-hash-stop")
        #elif self.mode.value == "clang-hash-collect":
            CC = os.path.join(clang_path, "build/wrappers/clang-hash-collect")
        elif self.mode.value == "ccache-clang-hash":
            CC = os.path.join(clang_path, "build/wrappers/clang-ccache-hash-stop")
        elif self.mode.value == "ccache":
            CC = os.path.join(clang_path, "build/wrappers/clang-ccache")
        else:
            raise RuntimeError("Not a valid mode")

        os.environ['CC'] = CC
        self.CC = CC

    def call_configure(self, path):
        if self.project_name() == "postgresql":
            shell("cd %s; mkdir -p build; cd build; ../configure --enable-depend", path)
        elif self.project_name() in ("musl", "bash"):
            shell_failok("cd %s; ./configure", path)
        elif self.project_name() in ("samba"):
            # Samba does not do optimizations if the argv[0] of the compiler is unknown. The default is -O2 for gcc. Therefore, we also use that.
            shell_failok("cd %s; ADDITIONAL_CFLAGS=-O2 ./buildtools/bin/waf configure", path)
        elif self.project_name() in ("cpython",):
            shell("cd %s; mkdir -p build build/Modules;", path)
            shell("cd %s; cp -u Modules/Setup.dist build/Modules/Setup", path)
            shell("cd %s; cd build; ../configure", path)

        elif self.project_name() in ('mbedtls'):
            shell("cd %s; mkdir -p build; cd build; cmake .. -DCMAKE_C_COMPILER=$CC -DENABLE_PROGRAMS=OFF", path)
        elif self.project_name() in ('lua',):
            # This is an ugly hack to make it possible to override the
            # CC variable from the outsite.
            with open("%s/makefile" % path) as fd:
                content = fd.readlines()
            content += "\nCC=%s\n" % (os.environ["CC"])

            with open("%s/makefile" % path, "w") as fd:
                fd.write("".join(content))

        else:
            raise RuntimeError("Not a valid project")

    def call_reconfigure(self, path):
        if self.project_name() in ('lua','mbedtls'):
            self.call_configure(path)
        if self.project_name() in ('cpython',):
            shell("cd %s; mkdir -p build/Modules; cp -u Modules/Setup.dist build/Modules/Setup", path)
            shell_failok("cd %s/build; make config.status;", path)
        if self.project_name() == "postgresql":
            shell_failok("cd %s/build; make config.status", path)
        if self.project_name() == "bash":
            shell_failok("cd %s; make config.status", path)
        if self.project_name() == "samba":
            shell_failok("cd %s; ADDITIONAL_CFLAGS=-O2 ./buildtools/bin/waf reconfigure", path)


    def call_make(self, path):
        if self.project_name() in ("mbedtls", "cpython", "postgresql"):
            return shell("cd %s/build; make -k -j %s", path, str(self.jobs.value))
        else:
            return shell("cd %s; make -k -j %s", path, str(self.jobs.value))

    def ccache_hits(self):
        (lines, _) = shell("ccache -s")
        ccache_hits = 0
        ccache_misses = 0
        for line in lines:
            if "cache hit" in line and "rate" not in line:
                ccache_hits += int(line[line.index(")")+1:].strip())
            if "cache miss" in line:
                ccache_misses += int(line[line.index("miss")+4:].strip())
        return ccache_hits, ccache_misses, "\n".join(lines)

    def rebuild(self, path, info, fail_ok=False):
        if "ccache" in self.mode.value:
            shell("ccache --zero-stats")

        if "clang-hash" in self.mode.value:
            hash_log = tempfile.NamedTemporaryFile()
            os.environ["CLANG_HASH_LOGFILE"] = hash_log.name
            os.environ["CLANG_HASH_OUTPUT_DIR"] = path # TODO: create new mode 'collect'
            os.environ["HASH_VERBOSE"] = '1'

        # Recompile!
        start_time = time.time()
        try:
            ret = self.call_make(path)
        except CommandFailed as e:
            if not fail_ok:
                raise
            else:
                info['failed'] = True
                ret = ("", 1)
        end_time = time.time()

        build_time = int((end_time - start_time) * 1e9)
        info['build-time'] = build_time
        #info['build-log'] = ret[0]

        # Record Cache misses and hits
        if "ccache" in self.mode.value:
            ccache_hits, ccache_misses, log = self.ccache_hits()
            info['ccache-log'] = log
            info['ccache-hits'] = ccache_hits
            info['ccache-misses'] = ccache_misses

        if "clang-hash" in self.mode.value:
            log = hash_log.read()
            info['clang-hash-log'] = log
            info['clang-hash-hits'] = log.count("H")
            info['clang-hash-misses'] = log.count("M")
            hash_log.close()


        logging.info("Rebuild done[%s]: %s s; failed=%s",
                     info.get("filename") or info.get("commit"),
                     build_time / 1e9,
                     info.get("failed", False))

        return info
