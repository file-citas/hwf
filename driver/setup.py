import os
import multiprocessing

LLVM_RELEASE_URL="http://releases.llvm.org"
LLVM_RELEASE_VER="5.0.0"
LLVM_SRC="llvm-" + LLVM_RELEASE_VER + ".src"
CLANG_SRC="cfe-" + LLVM_RELEASE_VER + ".src"
COMPILERRT_SRC="compiler-rt-" + LLVM_RELEASE_VER + ".src"
DR_CHECKER_GIT_URL="https://github.com/ucsb-seclab/dr_checker.git"
DR_CHECKER_GIT_BRANCH="speedy"
KERNEL_MSM_GIT_URL="https://android.googlesource.com/kernel/msm"

def download(file):
    if not os.path.isfile(file):
        os.system("wget " + "/".join((LLVM_RELEASE_URL, LLVM_RELEASE_VER, file)))

def untar(tarball, src, dest):
    if not os.path.isdir(dest):
        if not os.path.isdir(src):
            os.system("tar -xvf " + tarball)
        os.system("mv " + src + " " + dest)

def compile_llvm():
    if not os.path.isdir("install"):
        os.mkdir("install")
    install_dir = os.path.abspath("install")
    os.chdir("llvm")
    if not os.path.isdir("build"):
        os.mkdir("build")
    os.chdir("build")
    llvm_cmake_options=["-DLLVM_ENABLE_ASSERTIONS=On", "-DCMAKE_INSTALL_PREFIX=" + install_dir]
    os.system("cmake .. " + " ".join(llvm_cmake_options))
    os.system("make -j" + str(multiprocessing.cpu_count() / 2))
    os.chdir("..")
    os.chdir("..")

def pull_kernel():
    if not os.path.exists("msm"):
        branch = "android-msm-marlin-3.18-nougat-mr2.3"
        os.system("git clone -b " + branch + " --single-branch " + KERNEL_MSM_GIT_URL)

def pull_dr_checker():
    if not os.path.exists("dr_checker"):
        os.system("git clone -b " + DR_CHECKER_GIT_BRANCH + " --single-branch " + DR_CHECKER_GIT_URL)

def main():
    download(LLVM_SRC + ".tar.xz")
    download(CLANG_SRC + ".tar.xz")
    download(COMPILERRT_SRC + ".tar.xz")
    untar(LLVM_SRC + ".tar.xz", LLVM_SRC, "llvm-" + LLVM_RELEASE_VER)
    untar(CLANG_SRC + ".tar.xz", CLANG_SRC, "llvm-" + LLVM_RELEASE_VER + "/tools/clang")
    untar(COMPILERRT_SRC + ".tar.xz", COMPILERRT_SRC, "llvm-" + LLVM_RELEASE_VER + "/projects/compiler-rt")
    compile_llvm()
    pull_kernel()
    pull_dr_checker()

if __name__ == "__main__":
    main()
