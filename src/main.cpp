#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <memory>

// Platform-specific includes for clearing the console
#ifdef _WIN32
#include <windows.h>
#else
// On non-Windows platforms, this app will not compile with the GDI changes.
#endif

// --- Configuration ---

// Desired console dimensions for the ASCII art.
// Adjust these to match your terminal's size for best results.
const int CONSOLE_WIDTH = 360; // Doubled for higher resolution
const int CONSOLE_HEIGHT = 240;  // Doubled for higher resolution

// Characters from darkest to brightest. A longer ramp gives more detail.
const std::string ASCII_RAMP = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

// Desired frames per second.
const int TARGET_FPS = 60;

// --- End Configuration ---


/**
 * @brief Moves the console cursor to the top-left corner (0,0).
 * This is much faster than clearing the console with `system("cls")` or `system("clear")`,
 * reducing flicker.
 */
void reset_cursor() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD Position;
    Position.X = 0;
    Position.Y = 0;
    SetConsoleCursorPosition(hOut, Position);
#else
    // ANSI escape code for Linux/macOS
    std::cout << "\033[H";
#endif
}

#ifdef _WIN32
/**
 * @brief Captures the entire screen using the Windows GDI API.
 * @param buffer A vector to store the raw BGRA pixel data.
 * @param width Output parameter for the screen width.
 * @param height Output parameter for the screen height.
 * @return True on success, false on failure.
 */
bool captureScreenGDI(std::vector<unsigned char>& buffer, int& width, int& height) {
    // Get the device context for the entire screen.
    // GetDC(NULL) is the GDI equivalent of C#'s IntPtr.Zero for the screen.
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) return false;

    // Create a memory device context compatible with the screen DC.
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    if (!hMemoryDC) {
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);

    // Create a compatible bitmap to hold the screen capture.
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    if (!hBitmap) {
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Select the bitmap into the memory DC.
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // Perform the bit-block transfer from the screen to the memory DC.
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);

    // Setup the bitmap info structure to get the pixel data.
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // A negative height indicates a top-down DIB.
    bi.biPlanes = 1;
    bi.biBitCount = 32; // We want 32-bit BGRA format.
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    buffer.resize(width * height * 4);

    // Extract the pixel data from the bitmap.
    GetDIBits(hScreenDC, hBitmap, 0, (UINT)height, buffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // --- Cleanup GDI resources ---
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return true;
}

int main() {
    const auto frame_duration = std::chrono::milliseconds(1000 / TARGET_FPS);
    std::cout << "Starting screen capture... Press Ctrl+C to exit." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::vector<unsigned char> frame_buffer;
    int src_width = 0;
    int src_height = 0;

    while (true) {
        auto start_time = std::chrono::high_resolution_clock::now();

        if (!captureScreenGDI(frame_buffer, src_width, src_height)) {
            std::cerr << "Error: Failed to capture screen." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        const auto* src_data = frame_buffer.data();
        const float block_width = static_cast<float>(src_width) / CONSOLE_WIDTH;
        const float block_height = static_cast<float>(src_height) / CONSOLE_HEIGHT;

        std::string ascii_frame;
        ascii_frame.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

        for (int y = 0; y < CONSOLE_HEIGHT; ++y) {
            for (int x = 0; x < CONSOLE_WIDTH; ++x) {
                const int start_sx = static_cast<int>(x * block_width);
                const int start_sy = static_cast<int>(y * block_height);
                const int end_sx = static_cast<int>((x + 1) * block_width);
                const int end_sy = static_cast<int>((y + 1) * block_height);

                unsigned long long total_gray = 0;
                int pixel_count = 0;

                for (int sy = start_sy; sy < end_sy; ++sy) {
                    for (int sx = start_sx; sx < end_sx; ++sx) {
                        const auto pixel_offset = (sy * src_width + sx) * 4;
                        const unsigned char b = src_data[pixel_offset];
                        const unsigned char g = src_data[pixel_offset + 1];
                        const unsigned char r = src_data[pixel_offset + 2];
                        total_gray += static_cast<unsigned char>(0.2126 * r + 0.7152 * g + 0.0722 * b);
                        pixel_count++;
                    }
                }

                if (pixel_count > 0) {
                    const unsigned char avg_gray = total_gray / pixel_count;
                    const int ramp_index = (avg_gray * (ASCII_RAMP.length() - 1)) / 255;
                    ascii_frame += ASCII_RAMP[ramp_index];
                } else {
                    ascii_frame += ' ';
                }
            }
            ascii_frame += '\n';
        }

        reset_cursor();
        std::cout << ascii_frame << std::flush;

        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        if (elapsed < frame_duration) {
            std::this_thread::sleep_for(frame_duration - elapsed);
        }
    }
    return 0;
}
#else
int main() {
    std::cerr << "This application has been configured to use the Windows GDI API and will only run on Windows." << std::endl;
    return 1;
}
#endif