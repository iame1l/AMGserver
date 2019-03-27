## mfc上socket的API

```C++
struct  hostent {
        char    FAR * h_name;           /* official name of host */
        char    FAR * FAR * h_aliases;  /* alias list */
        short   h_addrtype;             /* host address type */
        short   h_length;               /* length of address */
        char    FAR * FAR * h_addr_list; /* list of addresses */
#define h_addr  h_addr_list[0]          /* address, for backward compat */
};
```





```C++
int PASCAL FAR gethostname(char FAR *name, int namelen); //返回一个本地的主机名缓冲区,

struct hostent *gethostbyname(const char *name);//读取形参里的hostname 是否成功
```

