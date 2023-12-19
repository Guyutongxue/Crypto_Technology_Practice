

## 生成自签名证书并添加根信任

```sh
# 生成证书
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha256 -days 365

# 添加信任（Ubuntu）
sudo cp cert.pem /usr/local/share/ca-certificates/localhost.crt
sudo update-ca-certificates
```
