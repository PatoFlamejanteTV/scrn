#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <cstring> // for memcpy

const int CONSOLE_WIDTH = 240;
const int CONSOLE_HEIGHT = 80;
const std::string ASCII_RAMP = "@%#*+=-:. ";

// Current implementation: String concatenation
void current_impl(const std::vector<unsigned char>& src_data, std::string& buffer, const std::vector<char>& lookup_table) {
    buffer.clear();
    // buffer is reserved outside

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

// Proposed implementation: Direct pointer write
void proposed_impl(const std::vector<unsigned char>& src_data, std::vector<char>& buffer, const std::vector<char>& lookup_table) {
    // We assume buffer is already sized to max capacity
    char* ptr = buffer.data();
    size_t cursor = 0;

    for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
        const size_t row_offset = static_cast<size_t>(y) * CONSOLE_WIDTH * 4;

        // Unroll inner loop slightly? Or just direct access
        for (int x = 0; x < CONSOLE_WIDTH; ++x) {
            const size_t pixel_offset = row_offset + (x * 4);
            const unsigned char b = src_data[pixel_offset];
            const unsigned char g = src_data[pixel_offset + 1];
            const unsigned char r = src_data[pixel_offset + 2];

            const unsigned int gray = (static_cast<unsigned int>(r) * 13933 +
                                       static_cast<unsigned int>(g) * 46871 +
                                       static_cast<unsigned int>(b) * 4732) >> 16;

            ptr[cursor++] = lookup_table[gray];
        }
        ptr[cursor++] = '\n';
    }
    // Null terminate if needed, or just track size.
    // std::cout.write doesn't need null terminator.
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

    // Setup buffers
    std::string string_buffer;
    string_buffer.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

    std::vector<char> vector_buffer((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

    const int ITERATIONS = 2000;

    // Warmup
    current_impl(src_data, string_buffer, lookup_table);
    proposed_impl(src_data, vector_buffer, lookup_table);

    // Benchmark Current
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        current_impl(src_data, string_buffer, lookup_table);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_current = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Benchmark Proposed
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        proposed_impl(src_data, vector_buffer, lookup_table);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_proposed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Current (string+=): " << duration_current / ITERATIONS << " us per frame\n";
    std::cout << "Proposed (ptr write): " << duration_proposed / ITERATIONS << " us per frame\n";
    std::cout << "Speedup: " << (double)duration_current / duration_proposed << "x\n";

    return 0;
}
