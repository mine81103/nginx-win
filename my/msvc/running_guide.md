
- For below eror:
```
  [emerg] 23508#20608: bind() to 0.0.0.0:80 failed (10013: An attempt was made to access a socket in a way forbidden by its access permissions)
```
  If the process occupying 80 port is "System" (process ID 4), do the following:
```
  open regedit,
  find HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\HTTP,
  Disable entry "Start"
```

- Make sure msvc/nginx_vc14 has below sub folders
  - conf/ (copy it from git repo's root folder if not existing)
  - html/ (copy it from git repo's root folder if not existing)
  - logs/
  - temp/

- Single process mode for debug, modify configuration:
```
  daemon off;
  master_process off;
```
