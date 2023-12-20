# 作业三（大作业）基于 C-S 的文件加密工具

## 介绍

一个可以加解密本地文件的程序；但秘钥管理由单独的服务器实现。

## 使用文档

### 开发说明

- 本项目使用 C++ 编程语言，遵循 C++23 标准，仅在 GNU/Linux 平台上测试。不保证 Windows 与 macOS 的兼容性。
- 本项目使用 [xmake](https://xmake.io/) 作为构建管理工具和包管理器。
- 本项目使用了以下第三方库：
    - [argparse](https://github.com/p-ranav/argparse)
    - [cpp-httplib](https://github.com/yhirose/cpp-httplib)
    - [OpenSSL](https://www.openssl.org/)
    - [SQLiteCpp](https://github.com/srombauts/SQLiteCpp/)
    - [TOML++](https://marzer.github.io/tomlplusplus/)

安装 xmake 后，本项目可用命令如下构建：

```
xmake f
xmake b
```

生成的 `hw3c` 为客户端程序，`hw3s` 为服务器端程序。

### 生成自签名证书并添加根信任

```sh
# 生成证书
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes

# 添加信任（Ubuntu）
sudo cp cert.pem /usr/local/share/ca-certificates/localhost.crt
sudo update-ca-certificates
```

### 运行服务器端程序

```sh
# 确保上一步生成的证书文件 cert.pem 和私钥文件 key.pem 位于工作路径下
./hw3s
```

此时服务器监听本地 8080 端口的 HTTPS 请求。

### 运行客户端程序

编写配置文件 `config.toml`：

```toml
user = "guyutongxue"

[server]
host = "localhost"
port = 8080
```

运行客户端程序（将配置文件放在工作路径下 `config.toml`，或者用 `-c` 参数指定）：

```sh
# 注册用户
./hw3c register

# 加密文件
./hw3c encrypt -i input.txt -o encrypted.bin

# 解密文件
./hw3c decrypt -i encrypted.bin -o output.txt
```

## 架构与代码说明

- 客户端与服务器端使用 HTTPS 通信，SSL 保证内容完整性和保密性；
- 服务器端使用 SQLite 持久化存储数据，包括用户信息和公私钥对；
- 客户端在注册时请求服务器生成公私钥对；加密时请求公钥；解密时发送密文，服务器端解密并返回。

具体代码上：
- 由于 OpenSSL 和 GmSSL 存在命名冲突，因此此项目使用 OpenSSL EVP 模块原生实现商密算法加解密，相关文件位于 `common/sm2.hpp` `common/sm4.hpp`；
- 客户端使用 argparse 解析命令行，使用 TOML++ 读取配置文件（`client/cli.hpp` `client/config.hpp`）；
- 客户端根据命令行命令，可选地使用 cpp-httplib 向服务器发送 HTTPS 请求并等待响应（`client/web.hpp`）；
- 服务器端使用 cpp-httplib 设置对应的处理函数（`server/handler.hpp`）；HTTPS 接口参考了 RESTful 风格；
- 服务器端使用 SQLiteCpp 进行数据库读写（`server/db.hpp`）；
- 客户端加解密文件的方式与作业 1 类似，设置了文件头以存储魔数、IV、加密后对称秘钥；为简化实现未设置哈希校验。参见 `client/crypt.hpp`。

此项目不设置头文件与源文件分离；使用 C++ 异常处理例外工作流；使用了概念、范围、约束 STL 算法、`std::format` 等现代 C++ 语言特性和标准库设施。
