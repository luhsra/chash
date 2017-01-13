#!/usr/bin/env python2

import os
import sys
import logging
import time

tmp_path = "%s/git/versuchung/src"% os.environ["HOME"]
if os.path.exists(tmp_path):
    sys.path.append(tmp_path)

from versuchung.experiment import Experiment
from versuchung.types import String, Bool,Integer
from versuchung.files import File, Directory
from versuchung.archives import GitArchive
from versuchung.execute import shell
from versuchung.tex import DatarefDict

class IncrementalCompilation(Experiment):
    inputs = {
        "clang_hash": GitArchive("/home/stettberger/w/clang-hash/"),
        "project": GitArchive("/home/stettberger/w/clang-hash/hash-projects/musl",
                              shallow=True),
        "touch-only": Bool(False),
        "mode": String("normal"),
        "jobs": Integer(4),
    }
    outputs = {
        "stats": File("summary.dict"),
    }

    def setup_compiler_paths(self, clang_path):
        if self.mode.value == "normal":
            CC = os.path.join(clang_path, "build/wrappers/clang-normal")
        elif self.mode.value == "clang-hash":
            CC = os.path.join(clang_path, "build/wrappers/clang-hash-stop")
        elif self.mode.value == "ccache":
            cache_dir = os.path.join(self.tmp_directory.path, "ccache")
            os.mkdir(cache_dir)
            os.environ["CCACHE_DIR"] = cache_dir
            CC = os.path.join(clang_path, "build/wrappers/clang-ccache")
        else:
            raise RuntimeError("Not a valid mode")

        os.environ['CC'] = CC
        self.CC = CC

    def call_configure(self, path):
        if self.project_name() == "postgresql":
            shell("cd %s; ./configure --enable-depend", path)
        elif self.project_name() in ("musl", "cpython", "bash", "waf"):
            shell("cd %s; ./configure", path)
        elif self.project_name() in ('mbedtls'):
            shell("cd %s; cmake . -DCMAKE_C_COMPILER=$CC", path)
        elif self.project_name() in ('lua',):
            pass
        else:
            raise RuntimeError("Not a valid project")

    def call_make(self, path, cause = ""):
        return shell("cd %s; make -j %s", path, str(self.jobs.value))

    def get_sources(self, path):
        ret = []
        for root, dirnames, filenames in os.walk(path):
            for filename in filenames:
                if filename.endswith(('.h','.c')):
                    ret.append(os.path.join(root, filename))
        if self.project_name() == "musl":
            # We do not touch headers that are external, since they
            # are untouchable.
            ret = [x for x in ret if x.endswith(".c") or "internal" in x ]
        return sorted(ret)

    def touch(self, path):
        if self.touch_only.value:
            os.utime(path, None)
        else:
            with open(path) as fd:
                content = fd.read()
            if ";\n" in content:
                content = content.replace(";\n", ";;\n", 1)
                with open(path, "w") as fd:
                    fd.write(content)
            else:
                os.utime(path, None)

    def rebuild(self, path, cause):
        info = {'filename': cause}
        self.build_info['builds'].append(info)

        # We export the RUN ID to the clang wrapper script, so it can
        # include it into the object file records

        # Recompile!
        start_time = time.time()
        ret = self.call_make(path, cause)
        end_time = time.time()

        # Call the lines that include the full compiler path. This
        # number is not useful, if -j N was done....
        # Therefore, we do not record it.
        compiler_calls = len([1 for x in ret[0]
                              if x.startswith(self.CC) and '-c' in x])
        # Account only nano seconds, everywhere
        build_time = int((end_time - start_time) * 1e9)
        info['build-time'] = build_time
        logging.info("Rebuild done[%s]: %s s; CC() = %d ", cause,
                     build_time / 1e9,
                     compiler_calls)
        return ret

    def run(self):
        # Determine the mode
        modes = ('normal', 'ccache', 'clang-hash')
        if not self.mode.value in modes:
            raise RuntimeError("Mode can only be one of: %s"%modes)


        logging.info("Build the Clang-Hash Plugin")
        with self.clang_hash as cl_path:
            shell("cd %s; mkdir build; cd build; cmake ..; make -j 4", cl_path)

        # Project name
        logging.info("Cloning project... %s", self.project_name())
        self.build_info = {"project-name": self.project_name(),
                           "commit-hash": self.metadata["project-hash"],
                           'builds': []}
        with self.project as src_path:
            # First, we redirect all calls to the compiler to our
            # clang hash wrapper
            self.setup_compiler_paths(cl_path)

            # Count the number of files
            sources = list(self.get_sources(src_path))
            nr_files = len(sources)
            logging.info("#files: %d", nr_files)
            self.build_info['file-count'] = nr_files

            # Initial build of the given project
            self.call_configure(src_path)
            self.rebuild(src_path, "FRESH_BUILD")

            # Iterate over all files
            for fn in sources:
                self.touch(fn)
                self.rebuild(src_path, fn)

        # Output the summary of this build into the statistics file.
        with open(self.stats.path, "w+") as fd:
            fd.write(repr(self.build_info))

    def project_name(self):
        return os.path.basename(self.metadata['project-clone-url'])

    def variant_name(self):
        mod = "append"
        if self.metadata['touch-only']:
            mod = "touch"
        return "%s-%s-%s"%(self.project_name(), mod, self.metadata['mode'])

    def symlink_name(self):
        return "%s-%s"%(self.title, self.variant_name())


if __name__ == "__main__":
    experiment = IncrementalCompilation()
    dirname = experiment(sys.argv)
