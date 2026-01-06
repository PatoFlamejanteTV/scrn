#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

const int CONSOLE_WIDTH = 240;
const int CONSOLE_HEIGHT = 80;
const std::string ASCII_RAMP = "@%#*+=-:. ";

// Baseline implementation
void baseline(const std::vector<unsigned char>& src_data, std::string& output) {
    output.clear(); // We start with empty string, but reallocation might happen if we don't reserve/reuse correctly in the loop context
    // In the real app, "std::string ascii_frame" is declared INSIDE the loop, so it's fresh every time.
    // To simulate that cost, we should probably construct it here, or just clear it but not reserve?
    // Actually, the current code DOES reserve: "ascii_frame.reserve(...)".

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
    output = ascii_frame;
}

// Optimization: Lookup table + Buffer reuse
void optimized(const std::vector<unsigned char>& src_data, std::string& buffer, const std::vector<char>& lookup_table) {
    buffer.clear();
    // Assuming buffer is already reserved outside.

    for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
        const size_t row_offset = static_cast<size_t>(y) * CONSOLE_WIDTH * 4;
        for (int x = 0; x < CONSOLE_WIDTH; ++x) {
            const size_t pixel_offset = row_offset + (x * 4);
            // We can also optimize pixel access by pointers? Maybe later.
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

    std::string result_baseline;
    std::string result_optimized;
    result_optimized.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

    const int ITERATIONS = 1000;

    // Benchmark Baseline
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        baseline(src_data, result_baseline);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_baseline = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Benchmark Optimized
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        optimized(src_data, result_optimized, lookup_table);
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_optimized = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Baseline: " << duration_baseline / ITERATIONS << " us per frame\n";
    std::cout << "Optimized: " << duration_optimized / ITERATIONS << " us per frame\n";
    std::cout << "Speedup: " << (double)duration_baseline / duration_optimized << "x\n";

    return 0;
}
