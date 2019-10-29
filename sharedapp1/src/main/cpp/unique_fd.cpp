#include "unique_fd.h"
#include <dlfcn.h>
#include <pthread.h>
#include <chrono>
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "unique_fd"


typedef bool (*fdsan_exchange_owner_function)(int fd, uint64_t expected_tag, uint64_t new_tag);

typedef bool (*fdsan_close_with_tag_function)(int fd, uint64_t tag);

static fdsan_exchange_owner_function fdsan_exchange_owner_type_handle = nullptr;
static fdsan_close_with_tag_function fdsan_close_with_tag_type_handle = nullptr;

static void resolve_fdsan_interfaces_once() {
    // todo: check api >= 29
    static pthread_once_t thread;
    pthread_once(&thread, [] {
        void *libcHandle = nullptr;
        libcHandle = dlopen("libc.so", RTLD_LAZY);
        fdsan_exchange_owner_type_handle = reinterpret_cast<fdsan_exchange_owner_function>(dlsym(
                libcHandle, "android_fdsan_exchange_owner_tag"));
        if (!fdsan_exchange_owner_type_handle) {
            LOGE("android_fdsan_exchange_owner_tag not found in libc.so");
            return;
        }
        fdsan_close_with_tag_type_handle = reinterpret_cast<fdsan_close_with_tag_function>(dlsym(
                libcHandle, "android_fdsan_close_with_tag"));
        if (!fdsan_exchange_owner_type_handle) {
            LOGE("android_fdsan_close_with_tag not found in libc.so");
        }
    });
}

void fdsan_exchange_owner_tag_wrapper(int fd, uint64_t old_tag, uint64_t new_tag) {
    resolve_fdsan_interfaces_once();
    if (fdsan_exchange_owner_type_handle) {
        fdsan_exchange_owner_type_handle(fd, old_tag, new_tag);
    }
}

int fdsan_close_with_tag_wrapper(int fd, uint64_t tag) {
    resolve_fdsan_interfaces_once();
    if (fdsan_close_with_tag_type_handle) {
        // fdsan will close this fd for us
        return fdsan_close_with_tag_type_handle(fd, tag);
    }
    return ::close(fd);
}

void unique_fd::exchange_tag(int fd, uint64_t old_tag, uint64_t new_tag) {
    fdsan_exchange_owner_tag_wrapper(fd, old_tag, new_tag);
}

int unique_fd::close(int fd, uint64_t tag) {
    return fdsan_close_with_tag_wrapper(fd, tag);
}

uint64_t unique_fd::tag() {
    // The obvious choice of tag to use is the address of the object.
    return reinterpret_cast<uint64_t>(this);
}

unique_fd &unique_fd::operator=(unique_fd &&move) noexcept {
    if (this == &move) {
        return *this;
    }

    reset();

    if (move.fd_ != -1) {
        fd_ = move.fd_;
        move.fd_ = -1;

        // Acquire ownership from the moved-from object.
        exchange_tag(fd_, move.tag(), tag());
    }

    return *this;
}

unique_fd::unique_fd(unique_fd &&move) {
    *this = std::move(move);
}
