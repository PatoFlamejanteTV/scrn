#include <vector>
#include <iostream>
#include <cassert>
#include <cstring>

// Mock Windows API
size_t last_wiped_size = 0;
void* last_wiped_ptr = nullptr;

void SecureZeroMemory(void* ptr, size_t cnt) {
    last_wiped_ptr = ptr;
    last_wiped_size = cnt;
    std::memset(ptr, 0, cnt);
}

// Vulnerable SecureBuffer (as in src/main.cpp)
class SecureBuffer {
    std::vector<unsigned char> buffer_;
public:
    SecureBuffer() = default;
    ~SecureBuffer() {
        if (!buffer_.empty()) {
            SecureZeroMemory(buffer_.data(), buffer_.size());
        }
    }
    // Prevent accidental copying which would leave unwiped duplicates
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

    void resize(size_t new_size) {
        if (new_size < buffer_.size()) {
            // Wipe the data we are about to discard
            SecureZeroMemory(buffer_.data() + new_size, buffer_.size() - new_size);
        }
        buffer_.resize(new_size);
    }
    unsigned char* data() { return buffer_.data(); }
    const unsigned char* data() const { return buffer_.data(); }
    size_t size() const { return buffer_.size(); }
    size_t capacity() const { return buffer_.capacity(); }
};

int main() {
    SecureBuffer sb;
    sb.resize(10);
    // Fill with pattern
    for(int i=0; i<10; ++i) sb.data()[i] = 0xAA;

    void* old_ptr = sb.data();
    size_t old_size = sb.size();
    size_t old_cap = sb.capacity();

    std::cout << "Old ptr: " << old_ptr << ", size: " << old_size << ", cap: " << old_cap << std::endl;

    // Reset spy
    last_wiped_ptr = nullptr;
    last_wiped_size = 0;

    // Resize to trigger reallocation (usually doubling size works)
    size_t new_size = old_cap * 2 + 10;
    std::cout << "Resizing to: " << new_size << std::endl;
    sb.resize(new_size);

    void* new_ptr = sb.data();
    std::cout << "New ptr: " << new_ptr << std::endl;

    if (new_ptr == old_ptr) {
        std::cout << "Reallocation did not happen. Test invalid. Try increasing size more." << std::endl;
        return 1;
    }

    // Verification: Did we wipe the old pointer?
    if (last_wiped_ptr == old_ptr && last_wiped_size == old_size) {
        std::cout << "SUCCESS: Old memory wiped." << std::endl;
        return 0;
    } else {
        std::cout << "FAILURE: Old memory NOT wiped. Last wiped ptr: " << last_wiped_ptr << ", size: " << last_wiped_size << std::endl;
        return 1; // Return error code
    }
}
