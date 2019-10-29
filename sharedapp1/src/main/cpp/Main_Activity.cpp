#include <jni.h>
#include <string>
#include <cinttypes>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cctype>
#include <fcntl.h>

#include <err.h>
#include <unistd.h>
#include <errno.h>


#include "unique_fd.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Main_Activity"

void closeUnownedFd(int fd) {
    // Check logcat output, you'll see:
    // E/fdsan: attempted to close file descriptor xx, expected to be unowned,
    // actually owned by native object of unknown type 0xffxxxxxx
    close(fd);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wwm_example_sharedapp1_MainActivity_testUniqueFd(JNIEnv *env, jclass clazz) {
    const char *path = "/data/data/com.wwm.example.sharedapp1/c.txt";
    unique_fd fd(open(path, O_CLOEXEC | O_WRONLY | O_CREAT));

    int ret = 0;
    if ((ret = write(fd, "foo", 3)) != 3) {
        LOGE("failed to write %s, ret=%d, fd=%d, error=%s", path, ret, fd.get(), strerror(errno));
    } else {
        LOGD("append \"foo\" to c.txt succeed");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wwm_example_sharedapp1_MainActivity_testCloseAnUnownedFd(JNIEnv *env, jclass clazz) {
    const char *path = "/data/data/com.wwm.example.sharedapp1/c.txt";
    unique_fd fd(open(path, O_APPEND | O_CLOEXEC | O_CREAT));
    closeUnownedFd(fd);
    LOGD("fd=%d", fd.get());
}