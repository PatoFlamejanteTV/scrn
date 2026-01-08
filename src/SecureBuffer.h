#ifndef SECURE_BUFFER_H
#define SECURE_BUFFER_H

#include <vector>
#include <memory>
#include <new>

#ifdef _WIN32
#include <windows.h>

// Custom allocator that securely wipes memory on deallocation
template <class T>
struct SecureAllocator {
    using value_type = T;

    SecureAllocator() = default;
    template <class U> SecureAllocator(const SecureAllocator<U>&) {}

    T* allocate(size_t n) {
        if (n > static_cast<size_t>(-1) / sizeof(T)) throw std::bad_alloc();
        if (auto p = static_cast<T*>(::operator new(n * sizeof(T)))) return p;
        throw std::bad_alloc();
    }

    void deallocate(T* p, size_t n) {
        if (p) {
            SecureZeroMemory(p, n * sizeof(T));
            ::operator delete(p);
        }
    }
};

template <class T, class U>
bool operator==(const SecureAllocator<T>&, const SecureAllocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const SecureAllocator<T>&, const SecureAllocator<U>&) { return false; }

// Secure wrapper for sensitive memory that wipes data on destruction and reallocation
class SecureBuffer {
    std::vector<unsigned char, SecureAllocator<unsigned char>> buffer_;
public:
    SecureBuffer() = default;

    // Prevent accidental copying which would leave unwiped duplicates
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

    // Explicit destructor is not strictly needed for the buffer content itself
    // because the allocator handles it, but we can keep it for defense in depth.

    void resize(size_t new_size) {
        if (new_size < buffer_.size()) {
            // Wipe the data we are about to discard (shrinking case)
            SecureZeroMemory(buffer_.data() + new_size, buffer_.size() - new_size);
        }
        buffer_.resize(new_size);
    }

    unsigned char* data() { return buffer_.data(); }
    const unsigned char* data() const { return buffer_.data(); }
    size_t size() const { return buffer_.size(); }
    size_t capacity() const { return buffer_.capacity(); }
};

#else
// Fallback for non-Windows platforms (no secure wipe)
class SecureBuffer {
    std::vector<unsigned char> buffer_;
public:
    void resize(size_t new_size) { buffer_.resize(new_size); }
    unsigned char* data() { return buffer_.data(); }
    const unsigned char* data() const { return buffer_.data(); }
    size_t size() const { return buffer_.size(); }
    size_t capacity() const { return buffer_.capacity(); }
};
#endif

#endif // SECURE_BUFFER_H
