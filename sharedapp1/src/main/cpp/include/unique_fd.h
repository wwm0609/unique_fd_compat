#pragma once

#include <unistd.h>

__BEGIN_DECLS
class unique_fd {
public:
    unique_fd() = default;

    explicit unique_fd(int fd) {
        reset(fd);
    }

    unique_fd(const unique_fd &copy) = delete;

    unique_fd(unique_fd &&move);

    ~unique_fd() {
        reset();
    }

    operator int() {
        return fd_;
    }

    unique_fd &operator=(const unique_fd &copy) = delete;

    unique_fd &operator=(unique_fd &&move) noexcept;

    int get() { return fd_; }

    int release() {
        if (fd_ == -1) {
            return -1;
        }

        int fd = fd_;
        fd_ = -1;

        // Release ownership.
        exchange_tag(fd, tag(), 0);
        return fd;
    }

    void reset(int new_fd = -1) {
        if (fd_ != -1) {
            close(fd_, tag());
            fd_ = -1;
        }

        if (new_fd != -1) {
            fd_ = new_fd;

            // Acquire ownership of the presumably unowned fd.
            exchange_tag(fd_, 0, tag());
        }
    }

private:

    uint64_t tag();

    void exchange_tag(int fd, uint64_t old_tag, uint64_t new_tag);

    int close(int fd, uint64_t tag);

    int fd_ = -1;
};
__END_DECLS

