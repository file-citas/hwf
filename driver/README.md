### Getting LLVM and Dr. Checker

`python setup.py`

### Building the Kernel

Dr. Checker generates compilation flags for Clang by capturing normal compilation process. You can build the kernel (*without* `-j` option) and capture the process as follows.

```
cd msm
export CROSS_COMPILE=...
export ARCH=arm64
make marlin_defconfig
make V=1 > makeout.txt 2>&1
```

### Generating Bitcode (.bc) Files

Use absolute paths, because the script changes CWD back and forth.

```
python build.py -l <output dir where .bc files are generated> -k <kernel source base dir> -m <path to makeout.txt> -g <$CROSS_COMPILE, i.e., compiler used to generate makeout.txt>
```

### Compiling Device Drivers using Clang

TBD
