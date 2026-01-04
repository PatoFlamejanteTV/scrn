#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>
#include <algorithm>

const int CONSOLE_WIDTH = 240;
const int CONSOLE_HEIGHT = 80;
const std::string ASCII_RAMP = "@%#*+=-:.";

// Original implementation
std::string convert_original(const std::vector<unsigned char>& src_data) {
    std::string ascii_frame;
    ascii_frame.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

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

            const int ramp_index = (gray * (ASCII_RAMP.length() - 1)) / 255;
            ascii_frame += ASCII_RAMP[ramp_index];
        }
        ascii_frame += '\n';
    }
    return ascii_frame;
}

// Optimized implementation
std::string convert_optimized(const std::vector<unsigned char>& src_data) {
    // Lookup table initialization (would be static/global in real app)
    static char gray_lookup[256];
    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < 256; ++i) {
            gray_lookup[i] = ASCII_RAMP[(i * (ASCII_RAMP.length() - 1)) / 255];
        }
        initialized = true;
    }

    // Direct buffer write
    std::string ascii_frame;
    // Pre-calculate exact size including newlines
    const size_t stride = CONSOLE_WIDTH + 1; // +1 for newline
    const size_t total_size = stride * (CONSOLE_HEIGHT - 1); // Only processing CONSOLE_HEIGHT - 1 rows
    ascii_frame.resize(total_size);

    char* dest_ptr = &ascii_frame[0];
    const unsigned char* src_ptr = src_data.data();

    for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
        for (int x = 0; x < CONSOLE_WIDTH; ++x) {
            // Unrolling or simplified indexing
            const unsigned char b = src_ptr[0];
            const unsigned char g = src_ptr[1];
            const unsigned char r = src_ptr[2];
            src_ptr += 4; // BGRA stride

            const unsigned int gray = (static_cast<unsigned int>(r) * 13933 +
                                       static_cast<unsigned int>(g) * 46871 +
                                       static_cast<unsigned int>(b) * 4732) >> 16;

            *dest_ptr++ = gray_lookup[gray];
        }
        *dest_ptr++ = '\n';
    }
    return ascii_frame;
}

int main() {
    // Setup dummy data
    std::vector<unsigned char> data(CONSOLE_WIDTH * CONSOLE_HEIGHT * 4);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<unsigned char>(i % 256);
    }

    // Correctness check
    std::string res1 = convert_original(data);
    std::string res2 = convert_optimized(data);
    if (res1 != res2) {
        std::cout << "MISMATCH!\n";
        return 1;
    }

    const int ITERATIONS = 10000;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        volatile std::string s = convert_original(data);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Original: " << diff.count() << " s\n";

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        volatile std::string s = convert_optimized(data);
    }
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "Optimized: " << diff.count() << " s\n";

    return 0;
}
