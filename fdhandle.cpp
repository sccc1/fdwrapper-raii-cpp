#include <unistd.h>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>


class FdHandle
{
    int m_fd;
    void reset() { m_fd = -1;}
    
    public:
    FdHandle(const char* path, int flags, int mode = S_IRUSR | S_IWUSR) {
        m_fd = open(path, flags, mode);
        if (m_fd == -1)
            throw std::system_error(errno, std::generic_category(), "Cannot open File:Error");
    }

    FdHandle(const FdHandle& hdl) = delete;
/* Alternate implementation if it makes sense to copy a fd handle
 * { 
        // Copy Constructor
        std::cerr<<"Copy Ctor!"<<std::endl;
        m_fd = dup(hdl.getFd());
        if(m_fd == -1) 
            throw std::system_error(errno, std::generic_category(), "Cannot open File:Error");
    }
*/    
    FdHandle(FdHandle&& hdl) = delete;
/* Alternate implementation if it makes sense to move a fd handle
 * {
        // Move constructor
        std::cerr<<"Move Ctor!"<<std::endl;
        m_fd = dup(hdl.getFd());
        if(m_fd == -1)
            throw std::system_error(errno, std::generic_category(), "Cannot open File:Error");
        hdl.reset();
        close(hdl.getFd());
    }
*/
    FdHandle& operator=(FdHandle&& hdl) = delete;   // Move assignment
    FdHandle& operator=(const FdHandle& hdl) = delete;
    int getFd()const { return m_fd;}

    ~FdHandle() {
        if(m_fd != -1)
            close(m_fd);
    }

};

int main(int argc, char *argv[])
{
    std::cerr<<argc<<std::endl;
    std::cerr<<argv[1]<<std::endl;
    FdHandle fd("./test", O_CREAT | O_RDWR | O_TRUNC);
    FdHandle fd2(fd);
    return 0;
}
