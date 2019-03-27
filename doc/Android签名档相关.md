## keysotre基本介绍

我们在对Apk签名时并没有直接指定私钥、公钥和数字证书，而是使用keystore文件，这些信息都包含在了keystore文件中。根据编码不同，keystore文件分为很多种，Android使用的是Java标准keystore格式JKS(Java Key Storage)，所以通过Android Studio导出的keystore文件是以.jks结尾的。

keystore使用的证书标准是X.509，X.509标准也有多种编码格式，常用的有两种：pem（Privacy Enhanced Mail）和der（Distinguished Encoding Rules）。jks使用的是der格式，Android也支持直接使用pem格式的证书进行签名，我们下面会介绍。



X.509的签名内容

![img](C:\Users\Administrator\Desktop\分析\签名档内容)



## keysotre的生成

Keytool 是一个Java数据证书的管理工具 ,Keytool将密钥（key）和证书（certificates）存在一个称为keystore的文件中在keystore里，包含两种数据:密钥实体（Key entity）-密钥（secret key）或者是私钥和配对公钥（采用非对称加密）可信任的证书实体（trusted certificate entries）-只包含公钥.

keytool.exe 工具位置 
**<u>到 JDK 安装目录 JDK\bin\keytool.exe</u>**

ps:可以在powershell或cmd 里检测是否安装了 JDK .(输入Java -version 就可以了)















