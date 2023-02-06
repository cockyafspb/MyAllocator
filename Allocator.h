#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <limits>
#include <iostream>
#include <vector>

namespace alloc {
    template<typename T = void>
    class Allocator {
    public:
        using allocator_type = Allocator<T>;
        using value_type = T;
        using reference = T &;
        using const_reference = const T &;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = T *;
        using const_pointer = const T *;
        using void_pointer = void *;
        using const_void_pointer = const void *;

        template<typename Other>
        struct rebind {
            using other = Allocator<Other>;
        };

        explicit Allocator(size_t size)
                : size(size) {
            mapped_memory.resize(size * sizeof(T));
            used_memory.resize(size);
        }

        Allocator() {
            size = 1024 * 50;
            mapped_memory.resize(size);
            used_memory.resize(size);
        }

        template <typename U>
        explicit Allocator(const Allocator<U> &other) noexcept: size(other.get_size()), mapped_memory(other.get_mapped_memory()), used_memory(other.get_used_memory()) {}

        template<class ...Args>
        pointer construct(pointer ptr, Args ...args) {
            return new(ptr) T(std::forward<Args>(args)...);
        }

        void destroy(pointer ptr) {
            deallocate(ptr, 1);
            ptr->~T();
        }

        pointer allocate(size_type n) {
            size_type pos = find_position(n);
            for (int i = 0; i < n; ++i) {
                used_memory[pos + i] = true;
            }
            return (pointer) &mapped_memory[pos * sizeof(value_type)];
        }

        void deallocate(void_pointer address, size_type n) {
            const auto ptr = static_cast<const std::byte *>(address);

            const auto begin = &mapped_memory[0];

            if (ptr >= begin) {
                const size_t pos = (ptr - begin) / sizeof(T);
                const size_t end = std::min(n, size - pos);

                for (int i = 0; i < end; ++i) {
                    used_memory[pos + i] = false;
                }
            }
        }


        size_type get_size() const noexcept{
            return size;
        }

        std::vector<std::byte> get_mapped_memory() const noexcept{
            return mapped_memory;
        }

        std::vector<bool> get_used_memory() const noexcept{
            return used_memory;
        }
    private:
        //fields
        std::vector<std::byte> mapped_memory;
        std::vector<bool> used_memory;
        size_type size;

        //func
        size_type find_position(size_type n) {
            int cur = 0;
            for (size_t i = 0; i < size; ++i) {
                if (cur == n) {
                    return i - n;
                }

                if (!used_memory[i]) {
                    ++cur;
                } else {
                    cur = 0;
                }
            }

            throw std::bad_alloc{};
        }
    };

    template<class T, class U>
    bool operator==(const Allocator<T> &, const Allocator<U> &) noexcept { return true; }

    template<class T, class U>
    bool operator!=(const Allocator<T> &, const Allocator<U> &) noexcept { return false; }

}


#endif //ALLOCATOR_H
