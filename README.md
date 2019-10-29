# Aosp's unique_fd wrapper

This repo just wrappers fdsan interfaces, so we can also run our own unique_fd on Android 10+ or lower.

Android 10 增加了 [fdsan](https://android.googlesource.com/platform/bionic/+/master/docs/fdsan.md )，它可以帮助您更轻松地查找和修复文件描述符所有权方面的问题。

这个repo只是对aosp's的fdsan封装了一下，以兼容Android旧版本。

## fdsan 检测到的错误

与错误处理文件描述符所有权相关的错误（通常表现为“use-after-close”和“double-close”）类似于内存分配“use-after-free”和“double-free”错误，但通常更难以诊断和修复。“fdsan”会尝试通过强制执行文件描述符所有权来检测和/或防止文件描述符误管理。

Android 的 fdsan 文件描述符排错程序有助于捕获包含 use-after-close 和 double-close 等[文件描述符的常见错误]((https://source.android.com/devices/tech/debug/native-crash#fdsan))。
```
pid: 32315, tid: 32315, name: crasher64  >>> crasher64 <<<
signal 35 (), code -1 (SI_QUEUE), fault addr --------
Abort message: 'attempted to close file descriptor 3, expected to be unowned, actually owned by FILE* 0x7d8e413018'
    x0  0000000000000000  x1  0000000000007e3b  x2  0000000000000023  x3  0000007fe7300bb0
    x4  3033313465386437  x5  3033313465386437  x6  3033313465386437  x7  3831303331346538
    x8  00000000000000f0  x9  0000000000000000  x10 0000000000000059  x11 0000000000000034
    x12 0000007d8ebc3a49  x13 0000007fe730077a  x14 0000007fe730077a  x15 0000000000000000
    x16 0000007d8ec9a7b8  x17 0000007d8ec779f0  x18 0000007d8f29c000  x19 0000000000007e3b
    x20 0000000000007e3b  x21 0000007d8f023020  x22 0000007d8f3b58dc  x23 0000000000000001
    x24 0000007fe73009a0  x25 0000007fe73008e0  x26 0000007fe7300ca0  x27 0000000000000000
    x28 0000000000000000  x29 0000007fe7300c90
    sp  0000007fe7300860  lr  0000007d8ec2f22c  pc  0000007d8ec2f250

backtrace:
      #00 pc 0000000000088250  /bionic/lib64/libc.so (fdsan_error(char const*, ...)+384)
      #01 pc 0000000000088060  /bionic/lib64/libc.so (android_fdsan_close_with_tag+632)
      #02 pc 00000000000887e8  /bionic/lib64/libc.so (close+16)
      #03 pc 000000000000379c  /system/bin/crasher64 (do_action+1316)
      #04 pc 00000000000049c8  /system/bin/crasher64 (main+96)
      #05 pc 000000000008021c  /bionic/lib64/libc.so (_start_main)
```
您可以通过回溯中出现的 fdsan_error（如果有的话）以及特定的中止消息，将此中止与其他类型的中止区分开来。

您可以使用 crasher fdsan_file 或 crasher fdsan_dir 来重现这类崩溃问题的实例。