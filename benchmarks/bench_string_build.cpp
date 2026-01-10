#include <iostream>
#include <string>
#include <vector>
#include <chrono>

const int CONSOLE_WIDTH = 240;
const int CONSOLE_HEIGHT = 80;
const int ITERATIONS = 1000;

void bench_append() {
    std::string buffer;
    buffer.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        buffer.clear();
        for (int y = 0; y < CONSOLE_HEIGHT; ++y) {
            for (int x = 0; x < CONSOLE_WIDTH; ++x) {
                buffer += 'x';
            }
            buffer += '\n';
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Append method: " << diff.count() << " s\n";
}

void bench_direct_write() {
    std::string buffer;
    buffer.resize((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        // In the real app we wouldn't resize every frame if size is constant,
        // but let's assume we just overwrite.
        // If dimensions change, we'd resize. Here we assume constant dimensions.
        char* ptr = &buffer[0];
        for (int y = 0; y < CONSOLE_HEIGHT; ++y) {
            for (int x = 0; x < CONSOLE_WIDTH; ++x) {
                *ptr++ = 'x';
            }
            *ptr++ = '\n';
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Direct write method: " << diff.count() << " s\n";
}

int main() {
    bench_append();
    bench_direct_write();
    return 0;
}
