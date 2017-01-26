import os
import fnmatch
import time
from versuchung.execute import shell, CommandFailed
import logging

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

        if self.mode.value == "normal":
            CC = os.path.join(clang_path, "build/wrappers/clang-normal")
        elif self.mode.value == "clang-hash":
            CC = os.path.join(clang_path, "build/wrappers/clang-hash-stop")
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
            shell("cd %s; ./configure --enable-depend", path)
        elif self.project_name() in ("musl", "bash", "samba"):
            shell("cd %s; ./configure", path)
        elif self.project_name() in ("cpython",):
            shell("cd %s; mkdir -p build build/Modules;", path)
            shell("cd %s; cp -u Modules/Setup.dist build/Modules/Setup", path)
            shell("cd %s; cd build; ../configure", path)

        elif self.project_name() in ('mbedtls'):
            shell("cd %s; mkdir build; cd build; cmake .. -DCMAKE_C_COMPILER=$CC -DENABLE_PROGRAMS=OFF", path)
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
        if self.project_name() in ('lua',):
            self.call_configure(path)
        if self.project_name() in ('cpython',):
            shell("cd %s; mkdir -p build/Modules; cp -u Modules/Setup.dist build/Modules/Setup", path)


    def call_make(self, path):
        if self.project_name() in ("mbedtls", "cpython"):
            return shell("cd %s/build; make -j %s", path, str(self.jobs.value))
        else:
            return shell("cd %s; make -j %s", path, str(self.jobs.value))

    def rebuild(self, path, info, fail_ok=False):
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

        # Account only nano seconds, everywhere
        build_time = int((end_time - start_time) * 1e9)
        info['build-time'] = build_time
        logging.info("Rebuild done[%s]: %s s; failed=%s",
                     info.get("filename") or info.get("commit"),
                     build_time / 1e9,
                     info.get("failed", False))
        return info
