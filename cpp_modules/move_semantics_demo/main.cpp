#include <algorithm>
#include <cstddef>
#include <iostream>
#include <utility>

class Buffer
{
public:
    explicit Buffer(std::size_t size)
        : size_(size),
          data_(size_ > 0 ? new int[size_] : nullptr)
    {
        std::cout
            << "[CONSTRUCTOR] size="
            << size_
            << '\n';
    }

    ~Buffer()
    {
        delete[] data_;

        std::cout
            << "[DESTRUCTOR] size="
            << size_
            << '\n';
    }

    // 拷贝构造
    Buffer(const Buffer &other)
        : size_(other.size_),
          data_(other.size_ > 0
                    ? new int[other.size_]
                    : nullptr)
    {
        if (size_ > 0)
        {
            std::copy(
                other.data_,
                other.data_ + size_,
                data_);
        }

        std::cout
            << "[COPY CONSTRUCTOR] size="
            << size_
            << '\n';
    }

    // 拷贝赋值
    Buffer &operator=(const Buffer &other)
    {
        if (this == &other)
        {
            return *this;
        }

        int *newData =
            other.size_ > 0
                ? new int[other.size_]
                : nullptr;

        if (other.size_ > 0)
        {
            std::copy(
                other.data_,
                other.data_ + other.size_,
                newData);
        }

        delete[] data_;

        data_ = newData;
        size_ = other.size_;

        std::cout
            << "[COPY ASSIGNMENT] size="
            << size_
            << '\n';

        return *this;
    }

    // 移动构造
    Buffer(Buffer &&other) noexcept
        : size_(other.size_),
          data_(other.data_)
    {
        other.size_ = 0;
        other.data_ = nullptr;

        std::cout
            << "[MOVE CONSTRUCTOR] size="
            << size_
            << '\n';
    }

    // 移动赋值
    Buffer &operator=(Buffer &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        delete[] data_;

        size_ = other.size_;
        data_ = other.data_;

        other.size_ = 0;
        other.data_ = nullptr;

        std::cout
            << "[MOVE ASSIGNMENT] size="
            << size_
            << '\n';

        return *this;
    }

    std::size_t size() const
    {
        return size_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

private:
    std::size_t size_{0};
    int *data_{nullptr};
};

int main()
{
    std::cout
        << "===== COPY ====="
        << '\n';

    Buffer buffer1(1024);

    Buffer buffer2 =
        buffer1;

    std::cout
        << "buffer1 size="
        << buffer1.size()
        << '\n';

    std::cout
        << "buffer2 size="
        << buffer2.size()
        << '\n';

    std::cout
        << "\n===== MOVE ====="
        << '\n';

    Buffer buffer3 =
        std::move(buffer1);

    std::cout
        << "buffer1 empty="
        << std::boolalpha
        << buffer1.empty()
        << '\n';

    std::cout
        << "buffer3 size="
        << buffer3.size()
        << '\n';

    std::cout
        << "\n===== ASSIGNMENT ====="
        << '\n';

    Buffer buffer4(10);

    buffer4 =
        std::move(buffer2);

    std::cout
        << "buffer2 empty="
        << buffer2.empty()
        << '\n';

    std::cout
        << "buffer4 size="
        << buffer4.size()
        << '\n';

    return 0;
}