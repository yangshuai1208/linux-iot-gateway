#include <iostream>
#include <utility>

#include <fcntl.h>
#include <unistd.h>

class ScopedFd
{
public:
    explicit ScopedFd(int fd=-1)
        :fd_(fd)
    {   
    }
    ~ScopedFd()
    {
        if(fd_>=0)
        {
            close(fd_);
        }
       
    }
    ScopedFd(const ScopedFd &)=delete;

    ScopedFd &operator=(const ScopedFd &)=delete;

    ScopedFd(ScopedFd &&other) noexcept
    {
        fd_=other.fd_;
        other.fd_=-1;
    }

    ScopedFd &operator =(ScopedFd &&other) noexcept
    {
        if(this!=&other)
    {
        if(fd_>=0)
        {
            close(fd_);
        }
       fd_=other.fd_;
       other.fd_=-1;
    }  
         return *this;
    }
    int get () const
    {
        return fd_;
    }
private:
    int fd_;
};
int main()
{
    int fd=open("practice/day02/text.txt",
                O_CREAT|O_WRONLY|O_TRUNC,
                0644);
    if(fd<0)
    {
        std::cerr<<"open failed\n";
        return 1;
    }
    ScopedFd file(fd);

    const char msg[]="hello Scoped\n";

    write(file.get(),msg,sizeof(msg)-1);

    return 0;
}



