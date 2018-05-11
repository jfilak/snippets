Mocking hostname
================

In case you are running an old Docker which does not support setting hostname
for `docker build` you can use this library to fool tools like hostname and
uname.

We rely on Dynamic linker and it's feature to preload Elf objects before any
other. You might have heared about the environment variable LD_PRELOAD.

The problem with environment variables is that that they can be easily cleared
or ignored. Hence, it is more reliable to put the mock library into
the file /etd/ld.so.preload.

Actually, in our case, we need to put two libraries into this file:
1. PATH/libmock_hostname.so
2. /usr/LIBARCH/libdl.so.2

We need libdl.so.2 because not all binaries are linked with -ldl but the mock
library uses functions from libdl.

The mock library wraps the glibc functions uname and gethostname and the
wrappers read hostname from `the environment variable HOSTNAME`, which you need
to update anyways, if you want to be sure that all tools uses the desired
hostname.

The only hostname information which is not covered is the special file
/proc/sys/kernel/hostname. We can wrap fopen and open and openat and so on to
open a memory stream instead but I have not come across a tool reading
the information directly from the special file yet.

Testing
-------

```
$ make
# Do no forget to update the environment variable HOSTNAME
$ sudo docker run -it --rm \
    -v $(realpath libmock_hostname.so):/usr/local/lib64/libmock_hostname.so \
    -v $(realpath ld.so.preload):/etc/ld.so.preload \
    -h bar \
    -e HOSTNAME=foo \
    --name mock_hostname centos "hostname"
```
