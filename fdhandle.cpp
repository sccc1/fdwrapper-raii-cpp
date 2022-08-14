#include <utility>
#include <unistd.h>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>


class FdHandle
{
    int m_fd;

    public:
    FdHandle(const char* path, int flags, int mode = S_IRUSR | S_IWUSR) {
        m_fd = open(path, flags, mode);
        if (m_fd == -1)
            throw std::system_error(errno, std::generic_category(), "Cannot open File:Error");
    }

    explicit FdHandle(int fd = -1): m_fd(fd) {}

    FdHandle(const FdHandle& hdl) = delete;
/* Alternate implementation if it makes sense to copy a fd handle
 * { 
        // Copy Constructor
        std::cerr<<"Copy Ctor!"<<std::endl;
        m_fd = dup(hdl.get());
        if(m_fd == -1) 
            throw std::system_error(errno, std::generic_category(), "Cannot open File:Error");
    }
*/    
    FdHandle(FdHandle&& hdl) noexcept:
        m_fd(hdl.m_fd) {
             hdl.m_fd = -1;
    }
    // It would have been better (e.g. more expressive) to write `: m_fd(boost::exchange(hdl.m_fd, -1)) {}`.

    FdHandle& operator=(FdHandle&& hdl) noexcept {
        FdHandle tmp(std::move(hdl));
        swap(tmp);
        return *this;
    }

    FdHandle& operator=(const FdHandle& hdl) = delete;

    // `get`, `release`, `reset` as for `unique_ptr`:
    int get() const noexcept { return m_fd; }
    operator int() const noexcept { return get(); }
    int release() noexcept { auto ret = m_fd; m_fd = -1; return ret; }
    // It would have been better to write `{ return boost::exchange(m_fd, -1); }`.
    void reset(int fd = -1) { FdHandle(fd).swap(*this); } // TODO: `noexcept` ?

    ~FdHandle() {
        if(m_fd >= 0)
            close(m_fd);
    }

    void swap(FdHandle& rhs) noexcept {
        using std::swap;
        swap(m_fd, rhs.m_fd);
    }
};

void swap(FdHandle& lhs, FdHandle& rhs) noexcept { return lhs.swap(rhs); }

void report_error(const char *context, int l_errno) {
    std::cout.flush();
    std::cerr << context << " (errno " << l_errno << ": " << strerror(l_errno) << ") !\n";
}

int main(int argc, char *argv[])
{
    std::cerr<<argc<<std::endl;
    std::cerr<<argv[1]<<std::endl;
    FdHandle fd("./test", O_CREAT | O_RDWR | O_TRUNC);
    //FdHandle fd2(fd); Define copy ctor else this is a compilation error
    // FdHandle fd3 = fd; Define copy assignment operator overload else compilation error
    FdHandle fd2(std::move(fd));
    FdHandle fd3; fd3 = std::move(fd2);

    const FdHandle fd0(open("LICENSE", O_RDONLY));
    if (fd0 < 0) { report_error("Cannot open input file", errno); return 1; }

    const size_t bufsize = 64; // `64 * 1024`, would be better, but we are testing with a small file.
    char buf[bufsize];
    for(;;) {
        const ssize_t n0 = read(fd0, buf, bufsize);
        if (n0 < 0) { report_error("Read error", errno); return 1; }
        const ssize_t n1 = write(fd3, buf, n0);
        // TODO: Consider partial writes ?
        if (n1 != n0) { report_error("Write error", errno); return 1; }
        if (n0 < bufsize) break;
    }

    std::cout << "TestHasPassed.\n";

    return 0;
}
