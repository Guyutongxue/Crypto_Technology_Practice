# 《密码应用技术实践》作业 2

## 介绍

一个生成 X.509 证书的程序；使用 SM2 椭圆算法作为公钥密码算法，使用 SM3-with-SM2 作为签名算法。

## 使用文档

直接运行即可生成符合题目要求的证书：
- `root.crt` 为自签名根证书；
- `sub.crt` 为使用 `root.crt` 作为颁发者的子证书。

同时，生成 `root` `sub` 两个文件夹保存公私钥对。

## 开发说明

- 本项目使用 C++ 编程语言，遵循 C++23 标准，**仅支持 GNU/Linux 平台**，已通过 GCC 13.1.0 编译。
- 本项目使用 [xmake](https://xmake.io/) 作为构建管理工具和包管理器。
- 本项目使用了以下第三方库：
    - [GmSSL](https://github.com/guanzhi/GmSSL)
    - [asn1c](http://lionet.info/asn1c/)

安装 xmake 后，本项目可用命令如下构建：

```
xmake f
xmake b
```

## 代码说明

`main.cpp` 调用两次 `createCertificate` 创建证书，调用 `verifyCertificate` 验证证书签名。

在 `createCertificate` 中，使用 `asn1c` 通过 `x509.asn1` 定义生成的语法解析及生成器来构造 ASN.1 结构。通过这些构造器，可以直接用赋值、便利宏或便利函数来完成证书的创建。

使用 GmSSL 提供的 `sm2_point_to_uncompressed_octets` 写入公钥信息（未压缩格式）。在创建完 `tbsCertificate` 子结构后，使用 `der_encode` 获取 DER 编码版本数据，使用 `sm3_digest` 生成摘要，最后用 `sm2_sign` 获取签名。

在 `verifyCertificate` 中，使用 `ber_decode` 解析出 `tbsCertificate` 子结构，获取其 DER 编码数据并生成摘要，最后用 `sm2_verify` 验证签名。

此程序使用 C++ 异常处理例外工作流；使用了概念、约束 STL 算法等现代 C++ 语言特性和标准库设施。
