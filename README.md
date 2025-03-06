# C/CXX 小项目代码管理、模块代码测试

使用bash脚本抓取`src`文件夹下的源文件，并生成`makefile`，快速编译源文件并生成可执行文件

## 优点
- 使用纯 bash 脚本管理，脚本中使用的也是通用工具，安装了 `core-utils` 的linux系统都可以使用。
- 自动抓取源文件夹（src）下所有符合条件的源文件，而不用修改任何东西
- 脚本生成的makefile及编译中间文件都放在单独的文件夹中（如`build_host_asan`），不会污染`src`文件夹
- 为项目生成`.clangd`文件及`compile_comands.json`，可使用`clangd`作为LSP（Language Server Protocol），为你的IDE提供代码补全、定义跳转、查找引用、语法检查、修正建议等等
- 支持交叉编译，并使用 `qemu` 工具在本地快速验证模块


## 快速入门

```shell
# create source file
$ cat > src/main.cpp <<EOF
#include <stdio.h>
int main(int argc, char* argv[])
{
    printf("hello!\n");
    return 0;
}
EOF

# compile and run program
$ ./compile.sh

```

## 创建多个子文件夹

项目默认将`src`文件夹加入`include`目录，用户可创建多个子文件夹，更清晰的管理项目源文件
```shell
#
$ mkdir -p src/submodule1
$ mkdir -p src/submodule2
$ mkdir -p src/submodule3

# 
$ touch src/submodule1/my_mod1.h
$ touch src/submodule2/my_mod2.h
$ touch src/submodule3/my_mod3.h

# create source file
$ cat > src/main.cpp <<EOF
#include <stdio.h>
#include "submodule1/my_mod1.h"
#include "submodule2/my_mod2.h"
#include "submodule3/my_mod3.h"

int main(int argc, char* argv[])
{
    printf("hello!\n");
    return 0;
}
EOF

#
$ tree src
src
├── main.cpp
├── submodule1
│   └── my_mod1.h
├── submodule2
│   └── my_mod2.h
└── submodule3
    └── my_mod3.h

4 directories, 4 files

# compile and run program
$ ./compile.sh

```

## 防止`src`中的源文件被抓取
有时候想保留源文件在`src`目录中，但又不想让其参与编译，可以将该文件加入到`builder/.glob_rule_ignore`规则当中

```shell

# create ignore source file
$ cat > src/main.cxx <<EOF
#include <stdio.h>
int main(int argc, char* argv[])
{
    printf("Plz ignore me!!! I dont want to be compiled.\n");
    return 0;
}
EOF

# add rules to builder/.glob_rule_ignore, so src/main.cxx will not be glob
$ realpath -m --relative-to=$(dirname $(realpath builder/.glob_rule_ignore)) src/main.cxx | xargs -I {} echo "echo {}" >> builder/.glob_rule_ignore


# compile and run program
$ ./compile.sh

```

## 添加依赖库引用（本机）

该环节介绍的添加依赖方式仅适用于本机，并可以使用`pkg-config`工具。


- 查看系统已安装依赖库: 使用`pkg-config --list-all`查看所有安装的开发包，以`curl`为例：
```shell
# 查找 curl
$ pkg-config --list-all | grep curl
libcurl                        libcurl - Library to transfer files with HTTP, FTP, etc.

# 检查 libcurl 可能使用到的依赖
$ pkg-config libcurl --print-requires-private
libidn2
zlib
libbrotlidec
libzstd
mit-krb5-gssapi
openssl
libpsl
libssh2
libnghttp2
libnghttp3

```

- 添加依赖文件：在`builder`文件夹下生成`lib_curl.mk`文件
```shell
# 
$ echo "\$(call pkgconf_add, libcurl)" | tee builder/libcurl.mk
#
$ pkg-config libcurl --print-requires-private | xargs -I {} echo "\$(call pkgconf_add, {})" | tee -a builder/libcurl.mk
$(call pkgconf_add, libidn2)
$(call pkgconf_add, zlib)
$(call pkgconf_add, libbrotlidec)
$(call pkgconf_add, libzstd)
$(call pkgconf_add, mit-krb5-gssapi)
$(call pkgconf_add, openssl)
$(call pkgconf_add, libpsl)
$(call pkgconf_add, libssh2)
$(call pkgconf_add, libnghttp2)
$(call pkgconf_add, libnghttp3)
```

- 在`builder/makefile.defs.mk`中引用该文件
```
# makefile.defs.mk
-include ${rpathd}/compile_flags_begin.mk

# ...
# add following line!!!
-include ${rpathd}/libcurl.mk
#...

-include ${rpathd}/compile_flags_end.mk
```

- 测试
```shell
#
$ cat >src/main.cpp <<EOF
#include <curl/curl.h>
int main(int argc, char* argv[])
{
    auto curl = curl_easy_init();
    if (curl)
    {
        curl_easy_cleanup(curl);
    }
    return 0;
}
EOF

#
$ ./compile.sh

```


## 添加依赖库引用（交叉编译）
对于交叉编译，建议直接将依赖库安装到`SYSROOT`中，由于`SYSROOT`环境变量已经导出，故可以直接在`builder/*.mk`中使用
```shell
#
$ cat > builder/lib_crosscompile_test.mk <<EOF
INCLUDES += -I\${SYSROOT}/usr/local/include
LDFLAGS += -L\${SYSROOT}/usr/local/lib
LIBS +=
EOF

```

- 在`builder/makefile.defs.mk`中引用该文件
```
# makefile.defs.mk
-include ${rpathd}/compile_flags_begin.mk

# ...
# add following line!!!
-include ${rpathd}/lib_crosscompile_test.mk
#...

-include ${rpathd}/compile_flags_end.mk
```


## 编译环境


以下环境变量会被编译过程中引用
- CC : compiler that compile '*.c' source files
- CXX : compile that compile '*.cpp' source files
- [AR](https://sourceware.org/binutils/docs/binutils/ar.html) : archive objects into .a file
- CFLAGS : compiler flags for CC
- CXXFLAGS : compiler flags for CXX
- INCLUDES : flags such as `-I/usr/include` or `-include config.h`, so that external headers for source file could be located.
- CPPFLAGS : macro defines such as `-DDEBUG` or `-DDEBUG=0` for all source file, deprecates, it is recommend to setup macro defines by including global config file.
- LDFLAGS : flags such as `-L/usr/lib`, so that external libraries for the project could be located
- LIBS : flags such as `-lz` or `-l:libz.a`, which specifies the libraries being used for the project

通常，`CPPFLAGS`并不建议使用，推荐使用全局的头文件，如`config.h`。
`INCLUDES`，`LDFLAGS`，`LIBS`不建议添加到环境变量当中，这些应该添加到`builder/lib*.mk`文件中，并在`builder/makefile.defs.mk`中使用`include`引用。
故编译环境文件中配置`CC`, `CXX`, `CFLAGS`, `CXXFLAGS`即可

> 编译相关的环境变量可参考文件`toolchain/env_template_for_toolchain.sh`


### 选择已有编译环境
具体的编译环境文件为`toolchain/toolchain_${TARGET_HOST}.sh`，使用对应的环境会将生成的makefile及obj等中间文件放到`build_${TARGET_HOST}`文件夹中。
- toolchain/toolchain_host.sh 为本地Release编译环境
- toolchain/toolchain_host_asan.sh 为本地Debug编译环境（默认），并使用AddressSanitizer检查内存问题

示例：
```shell
# 查看存在的环境
$ ls -l toolchain/toolchain_*.sh
-rw-r--r-- 1 li li 740 Mar  5 18:01 toolchain/toolchain_host_asan.sh
-rw-r--r-- 1 li li 712 Mar  5 18:01 toolchain/toolchain_host.sh
-rw-r--r-- 1 li li 802 Mar  5 18:01 toolchain/toolchain_zkswe_z20.sh

# 设置 TARGET_HOST=host ，使用 toolchain_host.sh 环境
$ TARGET_HOST=host ./compile.sh

# 生成的 makefile 及 其他文件都放入到 build_host 中
$ tree build_host
build_host
├── build.log
├── c_compiler
├── compile_commands.json
├── cppflags
├── cppflags_strip
├── cppflags_strip_lines
├── cxx_compiler
├── exec
├── makefile
├── objs
│   ├── main.cpp.cxx.ii
│   ├── main.cpp.cxx.o
│   ├── main.cpp.cxx.s
│   └── main.cpp.d
├── objs.whole.a
└── src.mk

2 directories, 15 files

```



### 新建编译环境

根据需求，创建编译环境文件`toolchain/toolchain_${TARGET_HOST}.sh`

#### 创建最小编译环境文件
事实上，`toolchain/env_template_for_toolchain.sh` 文件中会自动补全本机的环境变量，所以最小编译环境文件中直接`source toolchain/env_template_for_toolchain.sh`即可

```shell
# 创建最小编译环境文件
$ cat > toolchain/toolchain_host_min.sh <<EOF
#!/bin/bash
CUR_DIR=\$(dirname \$(realpath \${BASH_SOURCE}))
source \${CUR_DIR}/env_template_for_toolchain.sh
EOF

# 测试
$ TARGET_HOST=host_min ./compile.sh

# 查看 build_host_min
$ tree build_host_min
build_host_min
├── build.log
├── c_compiler
├── compile_commands.json
├── cppflags
├── cppflags_strip
├── cppflags_strip_lines
├── cxx_compiler
├── exec
├── makefile
├── objs
│   ├── main.cpp.cxx.o
│   └── main.cpp.d
├── objs.whole.a
└── src.mk

2 directories, 13 files

```

环境变量可能存在污染，故建议在编译环境文件中重置相关变量
```shell
# 创建最小编译环境文件
$ cat > toolchain/toolchain_host_min.sh <<EOF
#!/bin/bash
CUR_DIR=\$(dirname \$(realpath \${BASH_SOURCE}))

CFLAGS=""
CXXFLAGS=""
INCLUDES=""
CPPFLAGS=""
LDFLAGS=""
LIBS=""

source \${CUR_DIR}/env_template_for_toolchain.sh
EOF
```

#### 根据需求修改参数

添加调试参数，便于开发阶段调试并定位问题
```shell
#
$ cat > toolchain/toolchain_host_asan.sh <<EOF
#!/bin/bash
CUR_DIR=\$(dirname \$(realpath \${BASH_SOURCE}))

CFLAGS=
CFLAGS="\${CFLAGS} -fPIC"
CFLAGS="\${CFLAGS} -fdata-sections -ffunction-sections"
# 便于调试
CFLAGS="\${CFLAGS} -fno-omit-frame-pointer"
# 保留编译中间结果，如 *.i *.ii 文件，可检查宏定义展开是否正常
CFLAGS="\${CFLAGS} -save-temps=obj"
# 不做优化，保留debug symbols，同时使能 address sanitizer
CFLAGS="\${CFLAGS} -O0 -g -fsanitize=address"

CXXFLAGS=\${CXXFLAGS="\${CFLAGS} -fexceptions"}

INCLUDES=""

CPPFLAGS=""

LDFLAGS="-Wl,--gc-sections"
# 使能 address sanitizer
LIBS="-lasan"

source \${CUR_DIR}/env_template_for_toolchain.sh

EOF
```

#### 添加 gcc 交叉编译环境文件
直接参考`builder/toolchain_arm-linux-gnueabihf.sh`，以下变量需要确保正确
- TOOLCHAIN_BIN_DIR : gcc 所在路径
- TOOLCHAIN_TRIPLE : 
- SYSROOT : 如果实际sysroot与编译工具内置的默认sysroot不一样，需要设置，否则留空
- SYSTEM_PROCESSOR : 使用`qemu`模拟并运行交叉编译的程序时，需要正确设置，会调用
- CFLAGS : 需要正确配置，特别是 float-abi 和 fpu 等等，与板子上的程序使用的flag保持一致

gcc交叉编译环境文件`builder/toolchain_arm-linux-gnueabihf.sh`样例：
```bash
#!/bin/bash
CUR_DIR=$(dirname $(realpath ${BASH_SOURCE}))

# FIXME: important for setup PATH environment
TOOLCHAIN_BIN_DIR=$(dirname $(which arm-linux-gnueabihf-gcc))

# FIXME: important for setup env: CROSS_COMPILE CC CXX AR
TOOLCHAIN_TRIPLE=arm-linux-gnueabihf

# FIXME: setup if sysroot is differ from result of: ${TOOLCHAIN_TRIPLE}-gcc --print-sysroot
SYSROOT=

#
SYSTEM_NAME=Linux

# FIXME: important for qemu, need to setup properly for running cross-compiled program at localhost
SYSTEM_PROCESSOR=arm

# FIXME: important !!!
CFLAGS="-mfloat-abi=hard -mfpu=vfp"


CXXFLAGS="${CFLAGS}"

INCLUDES=""

CPPFLAGS=""

LDFLAGS=""

LIBS=""

# EXPORT_TOOLCHAIN=false
source ${CUR_DIR}/env_template_for_toolchain.sh


```

