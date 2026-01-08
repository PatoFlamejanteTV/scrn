#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

const int CONSOLE_WIDTH = 240;
const int CONSOLE_HEIGHT = 80;
const std::string ASCII_RAMP = "@%#*+=-:. ";

// Current optimized implementation (from main.cpp)
void current_optimized(const std::vector<unsigned char>& src_data, std::string& buffer, const std::vector<char>& lookup_table) {
    buffer.clear();
    // Assuming buffer is already reserved outside.

    for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
        const size_t row_offset = static_cast<size_t>(y) * CONSOLE_WIDTH * 4;
        for (int x = 0; x < CONSOLE_WIDTH; ++x) {
            const size_t pixel_offset = row_offset + (x * 4);
            const unsigned char b = src_data[pixel_offset];
            const unsigned char g = src_data[pixel_offset + 1];
            const unsigned char r = src_data[pixel_offset + 2];

            const unsigned int gray = (static_cast<unsigned int>(r) * 13933 +
                                       static_cast<unsigned int>(g) * 46871 +
                                       static_cast<unsigned int>(b) * 4732) >> 16;

            buffer += lookup_table[gray];
        }
        buffer += '\n';
    }
}

// Proposed Optimization: Direct Write
void direct_write(const std::vector<unsigned char>& src_data, std::string& buffer, const std::vector<char>& lookup_table) {
    // Calculate total size needed: (Width + newline) * Height (minus 1 for status bar, but logic in main.cpp handles status bar separately)
    // In main.cpp loops: CONSOLE_HEIGHT - 1 lines.
    // Each line: CONSOLE_WIDTH chars + 1 newline.
    size_t required_size = (CONSOLE_WIDTH + 1) * (CONSOLE_HEIGHT - 1);

    // Resize buffer to exact size (overwriting content is fine as we will replace it all)
    // Using resize() ensures size() is correct for printing.
    if (buffer.size() != required_size) {
        buffer.resize(required_size);
    }

    // Direct pointer access
    char* out_ptr = &buffer[0];
    const unsigned char* src_ptr = src_data.data();

    for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
        const size_t row_offset = static_cast<size_t>(y) * CONSOLE_WIDTH * 4;
        for (int x = 0; x < CONSOLE_WIDTH; ++x) {
            const size_t pixel_offset = row_offset + (x * 4);

            // Access optimization: src_ptr is already raw data
            const unsigned char b = src_ptr[pixel_offset];
            const unsigned char g = src_ptr[pixel_offset + 1];
            const unsigned char r = src_ptr[pixel_offset + 2];

            const unsigned int gray = (static_cast<unsigned int>(r) * 13933 +
                                       static_cast<unsigned int>(g) * 46871 +
                                       static_cast<unsigned int>(b) * 4732) >> 16;

            *out_ptr++ = lookup_table[gray];
        }
        *out_ptr++ = '\n';
    }
}

int main() {
    // Setup data
    std::vector<unsigned char> src_data(CONSOLE_WIDTH * CONSOLE_HEIGHT * 4);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& byte : src_data) byte = static_cast<unsigned char>(dist(rng));

    // Precompute lookup table
    std::vector<char> lookup_table(256);
    for (int i = 0; i < 256; ++i) {
        lookup_table[i] = ASCII_RAMP[(i * (ASCII_RAMP.length() - 1)) / 255];
    }

    std::string result_current;
    result_current.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);
    std::string result_direct;
    result_direct.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

    const int ITERATIONS = 5000;

    // Benchmark Current
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        current_optimized(src_data, result_current, lookup_table);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_current = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Benchmark Direct Write
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        direct_write(src_data, result_direct, lookup_table);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_direct = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Current: " << duration_current / ITERATIONS << " us per frame\n";
    std::cout << "Direct Write: " << duration_direct / ITERATIONS << " us per frame\n";
    std::cout << "Speedup: " << (double)duration_current / duration_direct << "x\n";

    if (result_current != result_direct) {
        std::cerr << "Mismatch! Output differs.\n";
        // std::cerr << "Current size: " << result_current.size() << "\n";
        // std::cerr << "Direct size: " << result_direct.size() << "\n";
        return 1;
    } else {
        std::cout << "Verification: Output matches.\n";
    }

    return 0;
}
