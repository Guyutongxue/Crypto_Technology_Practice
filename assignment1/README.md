# 《密码应用技术实践》作业 1

## 介绍

一个使用商密算法的文件加解密工具，起名为 `encbox`。根据作业要求，其工作原理为：
- 使用 SM2 算法生成公私钥对；
- 生成临时秘钥并使用公钥加密保存；
- 使用临时秘钥，用 SM4，CBC 和 PKCS 填充模式加密给定文件夹；
- 使用私钥解密秘钥以解密给定文件夹；
- 支持使用 SM3 算法生成文件签名以校验正确性。

## 使用文档

```
encbox command [args...]
encbox --help
```

- `command` 子命令，见下文；
- `--help` 打印帮助信息。

### 例子

```sh
mkdir test
echo Hello > test/a.txt

./encbox keygen
./encbox encrypt test
./encbox decrypt test

cat test/a.txt # got "Hello"
```

### `keygen` 公私钥对生成

```
encbox keygen [--output=<filename>] [--force]
encbox keygen --help
```

生成 SM2 公私钥对。

- `--output`（可选）设置存放秘钥的文件路径。默认私钥路径为 `id_key`。公钥路径总是在其后添加 `.pub`，如 `id_key.pub`。
- `--force` 强制覆盖已有文件。
- `--help` 打印帮助信息。

### `encrypt` 执行加密操作

对给定文件夹执行加密操作。

```
encbox encrypt [--input=]<path> [--output=<path>] [--pkey=<path>]
               [--key-path=<path>] [--no-verify] [--seed=<number>] [--force]
encbox encrypt --help
```

- `--input` 要加密的文件夹。
- `--output` 密文输出路径，默认原位覆盖。
- `--pkey` 加密临时秘钥的公钥文件路径，默认为 `id_key.pub`。
- `--key-path` 加密秘钥的存储路径，默认存放在输出文件夹下的 `.encbox.key`。
- `--no-verify` 不在密文中生成用于验证文件的签名值。
- `--seed` 设置产生临时秘钥的随机种子，不应使用。
- `--force` 强制覆盖已有文件。
- `--help` 打印帮助信息。

### `decrypt` 执行解密操作

对给定文件夹执行加密操作。

```
encbox encrypt [--input=]<path> [--output=<path>] [--pkey=<path>]
               [--key-path=<path>] [--no-verify] [--force]
encbox encrypt --help
```

- `--input` 要解密的文件夹。
- `--output` 明文输出路径，默认原位覆盖。
- `--pkey` 解密临时秘钥的私钥文件路径，默认为 `id_key`。
- `--key-path` 临时秘钥的存储路径，默认读取在输入文件夹下的 `.encbox.key`。
- `--no-verify` 不验证密文中用于验证文件的签名值。
- `--force` 强制覆盖已有文件。
- `--help` 打印帮助信息。

## 开发说明

- 本项目使用 C++ 编程语言，遵循 C++23 标准，已通过 GCC 13.1.0 和 MSVC 17.3.0 的编译。
- 本项目使用 [xmake](https://xmake.io/) 作为构建管理工具和包管理器。
- 本项目使用了以下第三方库：
    - [GmSSL](https://github.com/guanzhi/GmSSL)
    - [Boost](https://www.boost.org/)

安装 xmake 后，本项目可用命令如下构建：

```
xmake f
xmake b
```

请确保使用最新版本的编译器。本项目未在 Apple/Darwin/aarch64 平台体系测试。

## 代码说明

出于减少代码量和敏捷开发的考虑，不设置头文件与源文件的分离，即仅头文件（Header-only）组织各模块。

- `cli.hpp` 使用 Boost.Program_Options 解析命令行参数，返回解析结果；
- `main.cpp` 使用 `std::visit` 将命令行解析结果派发到各个子命令的负责模块；
- `keygen.hpp` 提供 GmSSL SM2 模块生成公私钥对功能，以及其读取功能：
    - 公私钥文件有特定格式：使用特定文件头、文件尾，并以 base64 文本形式存储；
    - 读取公私钥文件时检查格式。
- `encrypt.hpp` 提供文件加解密流程：
    - 首先读取公钥或私钥以加解密临时秘钥；
    - 遍历目标中的每个文件，写入文件头或读取并校验文件头；
    - 可选地使用 GmSSL SM3 模块生成或校验签名；
    - 对每个文件，使用临时秘钥和 GmSSL SM4 模块加密或解密；模式和填充算法由 GmSSL 提供。
- `fs.hpp` 提供文件读取和写入的便利函数：
    - 使用 `<filesystem>` 标准库；
    - `traverseDirectory` 提供递归遍历文件系统树的功能。
- `rand.hpp` 生成高质量随机数，用于生成种子或临时秘钥：
    - 使用 `<random>` 标准库；
- `util.hpp` 提供若干工具函数：
    - POD 到字节流的相互转换；
    - base64 编码与解码。

本项目使用 C++ 异常处理例外工作流。

本项目大量使用概念、范围、`std::format` 等现代 C++ 语言特性和标准库设施。
