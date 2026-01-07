#include <iostream>
#include <vector>
#include <cstring>
#include <cassert>

// Define spy variables
size_t last_wiped_size = 0;
void* last_wiped_ptr = nullptr;

// We need to define main as something else to include the file
#define main src_main
#include "../src/main.cpp"
#undef main

int main() {
    SecureBuffer sb;
    sb.resize(10);
    // Fill with pattern
    for(int i=0; i<10; ++i) sb.data()[i] = 0xAA;

    void* old_ptr = sb.data();
    size_t old_size = sb.size();

    std::cout << "Old ptr: " << old_ptr << ", size: " << old_size << std::endl;

    // Reset spy
    last_wiped_ptr = nullptr;
    last_wiped_size = 0;

    // Resize to trigger reallocation
    size_t new_size = 1000;
    std::cout << "Resizing to: " << new_size << std::endl;
    sb.resize(new_size);

    void* new_ptr = sb.data();
    std::cout << "New ptr: " << new_ptr << std::endl;

    if (new_ptr == old_ptr) {
        std::cout << "Reallocation did not happen. Test invalid." << std::endl;
        return 1;
    }

    // Verification: Did we wipe the old pointer?
    if (last_wiped_ptr == old_ptr && last_wiped_size == old_size) {
        std::cout << "SUCCESS: Old memory wiped." << std::endl;
        return 0;
    } else {
        std::cout << "FAILURE: Old memory NOT wiped. Last wiped ptr: " << last_wiped_ptr << ", size: " << last_wiped_size << std::endl;
        return 1;
    }
}
