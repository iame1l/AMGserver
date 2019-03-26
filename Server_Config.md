[TOC]

# DB & AServer & MServer

- 双E5-2620,2T机械硬盘，Win Server 2012系统。

  ```
  ST3-S1001-4U035
  103.103.71.14:61970
  103.103.71.16:61970
  103.103.71.20:61970 //main
  administrator
  DBS0313)#!#
  eil
  USR0313)#!#
  ```

- DB 账号信息

  - 用户名：phqpdb
  - 密码：2019January)!@@
  - 数据库：DB_phqp & phqpDB

- 端口

  - AServer 13025
  - MServer 3015

- 本地用户和组

  - lusrmgr.msc

# Game & AI & Rsource Download

- 双E5-2620,1T机械硬盘，Win Server 2012系统。

  ```
  ST2-S1001-4U163
  103.112.28.128:61970
  103.112.28.90:61970 //main
  27.50.51.201:61970
  administrator
  GRS0313)#!#
  eil
  USR0313)#!#
  ```

- 端口

  - Resource Download 8081
  - Game & AI 5601-5700

# WEB

- E3-1230，240G SSD，30G防御，win. 2012系统

  ```
  HK1-S1008-22 
  103.107.10.9:61970 //main
  administrator
  WEB0313)#!#
  EIL
  USR0313)#!#
  ```

- 端口

  - Web 80

- 网站账号信息

  - Admin phpq 888888
  - Ananly admin 888888
  - Agency admin 888888

- PowerShell 执行命令 启用 IIS 服务

  - ```
    Install-WindowsFeature –ConfigurationFilePath DeploymentConfigTemplate_IIS.xml
    ```

# FuJian

- ```
  14.192.11.100:61970 //main
  14.192.11.121
  14.192.11.95
  administrator	
  FJJ0313)#!#
  eil
  USR0313)#!#
  ```

- DB 账号信息
  - 用户名：phqpdbfj
  - 密码：2019January)!@*
  - 数据库：DB_phqp

- 网站账号信息

  - Admin phpqfj 888888
  - Ananly admin 888888
  - Agency admin 888888

# Test Server

- E5-2620 2.0Ghz 16G 1TB 

  ```
  27.50.49.214:61970 //main
  27.50.49.215
  27.50.49.219
  administrator
  TES0313)#!#
  eil
  USR0313)#!#
  ```

- DB 账号信息

  - 用户名 sql
  - 密码 sql
  - 数据库 sql

- 网站账号信息

  - Admin admin 888888
  - Ananly admin 888888
  - Agency admin 888888

