Build Nginx on Windows (no SSL)
---------------------------------------------------------------------------------------------------

Refer to
- http://nginx.org/en/docs/howto_build_on_win32.html
- http://stackoverflow.com/questions/21486482/compile-nginx-with-visual-studio/22649559#22649559

Software:
- Visual Studio 2015
- MSYS

Download nginx source code from https://github.com/nginx/nginx

Steps (Part I):
---------------------------------------------------------------------------------------------------
Install MSYS if not installed, to folder like D:\Green\MinGW

Open cmd, run the followings:
```
    "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
    D:\Green\MinGW\msys\1.0\msys.bat
    cd nginx
    mkdir -p objs logs
    
    tar -xzf ../../pcre-8.41.tar.gz
    tar -xzf ../../zlib-1.2.11.tar.gz
    tar -xzf ../../openssl-1.0.2n.tar.gz

    auto/configure \
        --with-cc=cl \
        --with-debug \
        --prefix= \
        --conf-path=conf/nginx.conf \
        --pid-path=logs/nginx.pid \
        --http-log-path=logs/access.log \
        --error-log-path=logs/error.log \
        --sbin-path=nginx.exe \
        --http-client-body-temp-path=temp/client_body_temp \
        --http-proxy-temp-path=temp/proxy_temp \
        --http-fastcgi-temp-path=temp/fastcgi_temp \
        --http-scgi-temp-path=temp/scgi_temp \
        --http-uwsgi-temp-path=temp/uwsgi_temp \
        --with-cc-opt=-DFD_SETSIZE=1024 \
        --with-pcre=objs/lib/pcre-8.41 \
        --with-zlib=objs/lib/zlib-1.2.11 \
        --with-openssl=objs/lib/openssl-1.0.2n \
        --with-openssl-opt=no-asm \
        --with-select_module \
        --with-http_ssl_module
    nmake -f objs/Makefile
```
    Copy nginx\objs\*.h and *.c into nginx\msvc\objs\

    nginx\src\os\win32\ngx_win32_config.h, for below lines at about line 155,
    add #if !defined(_WIN64) ... #endif:
```
    typedef int                 intptr_t;
    typedef u_int               uintptr_t;
```

    nginx\src\os\win32\ngx_win32_config.h, for below lines at about line 190, replace below first code block with the second.
```
    #ifndef __MINGW64_VERSION_MAJOR
    typedef int                 ssize_t;
    #endif
```
```
    #ifndef __MINGW64_VERSION_MAJOR
    #ifdef _WIN64
    typedef __int64             ssize_t;
    #else
    typedef int                 ssize_t;
    #endif
    #endif
```

Steps (Part II):
---------------------------------------------------------------------------------------------------
- Open Visual Studio 2015, create a empty project as nginx\msvc\nginx_vc14\nginx_vc14.vcxproj

- Add source files to the project:
  - src\core
  - src\event
  - src\event\modules
  - src\http
  - src\http\modules
  - src\http\modules\perl
  - src\mail
  - src\misc
  - src\os\win32

- Under Project's Configuration Properties (for All Configurations): C/C++ | Preprocessor,
  - add "Processor Definitions":
  ```
    FD_SETSIZE=1024
    _WINSOCK_DEPRECATED_NO_WARNINGS
  ```
- Under Project's Configuration Properties (for All Configurations): C/C++ | General,
    add "Additional Include Directories":
```
    ../../src/core
    ../../src/event
    ../../src/event/modules
    ../../src/mail
    ../../src/http
    ../../src/http/modules
    ../../src/http/modules/perl
    ../../src/os/win32
    ../../../zlib
    ../objs
    ../objs/lib/pcre-8.33/inc
```
- Under Project's Configuration Properties (for All Configurations): Linker | Input,
    add "Additional Dependencies":
```
    ws2_32.lib
    pcre3.lib
    libcryptoMD.lib
    libsslMD.lib
```
- Under Project's Configuration Properties (for All Configurations):
  - Linker | General,
  add "Additional Library Directories":
```
  ../../extern/pcre-8.33/lib/x64
  ../../extern/openssl/bin64
```
- Project Setting for DLLs<br>
  Project | Properties | Configuration Properties | Debugging | Working Directory: <br>
  Envrionment: PATH=$(PATH);../../extern/pcre-8.33/bin/x64;../../extern/openssl/bin64
