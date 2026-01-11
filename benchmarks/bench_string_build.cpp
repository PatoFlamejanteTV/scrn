#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>

const int CONSOLE_WIDTH = 240;
const int CONSOLE_HEIGHT = 80;
const int FRAMES = 1000;

// Mock source data
std::vector<unsigned char> src_data(CONSOLE_WIDTH * CONSOLE_HEIGHT * 4);
std::vector<char> gray_lookup(256);

void init_data() {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& b : src_data) b = static_cast<unsigned char>(dist(rng));
    for (int i = 0; i < 256; ++i) gray_lookup[i] = (char)(' ' + (i % 90));
}

void bench_append() {
    std::string buffer;
    buffer.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

    auto start = std::chrono::high_resolution_clock::now();

    for (int f = 0; f < FRAMES; ++f) {
        buffer.clear();
        for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
            const size_t row_offset = static_cast<size_t>(y) * CONSOLE_WIDTH * 4;
            for (int x = 0; x < CONSOLE_WIDTH; ++x) {
                const size_t pixel_offset = row_offset + (x * 4);
                // Simulate calculation
                unsigned char r = src_data[pixel_offset];
                unsigned char g = src_data[pixel_offset+1];
                unsigned char b = src_data[pixel_offset+2];
                unsigned int gray = (r + g + b) / 3; // simplified

                buffer += gray_lookup[gray];
            }
            buffer += '\n';
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Append method: " << elapsed.count() << "s" << std::endl;
}

void bench_direct_write() {
    std::string buffer;
    size_t line_len = CONSOLE_WIDTH + 1;
    size_t total_size = line_len * (CONSOLE_HEIGHT - 1);
    buffer.resize(total_size);

    auto start = std::chrono::high_resolution_clock::now();

    for (int f = 0; f < FRAMES; ++f) {
        // Assume size is already correct or resize once
        if (buffer.size() != total_size) buffer.resize(total_size);
        char* ptr = &buffer[0];

        for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
            const size_t src_row_offset = static_cast<size_t>(y) * CONSOLE_WIDTH * 4;
            const size_t dst_row_offset = y * line_len;

            for (int x = 0; x < CONSOLE_WIDTH; ++x) {
                const size_t pixel_offset = src_row_offset + (x * 4);
                unsigned char r = src_data[pixel_offset];
                unsigned char g = src_data[pixel_offset+1];
                unsigned char b = src_data[pixel_offset+2];
                unsigned int gray = (r + g + b) / 3;

                ptr[dst_row_offset + x] = gray_lookup[gray];
            }
            ptr[dst_row_offset + CONSOLE_WIDTH] = '\n';
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Direct write method: " << elapsed.count() << "s" << std::endl;
}

int main() {
    init_data();
    bench_append();
    bench_direct_write();
    return 0;
}
