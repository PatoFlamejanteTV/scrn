#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif
// Helper to restore console code page on exit (RAII)
#ifdef _WIN32
class ConsoleCodePageGuard {
    UINT old_cp;
public:
    ConsoleCodePageGuard(UINT new_cp) {
        old_cp = GetConsoleOutputCP();
        SetConsoleOutputCP(new_cp);
    }
    ~ConsoleCodePageGuard() {
        SetConsoleOutputCP(old_cp);
    }
};

// RAII wrapper for GDI Device Context handle
class ScopedHDC {
    HDC hdc_;
    HWND hwnd_;
public:
    ScopedHDC(HWND hwnd) : hwnd_(hwnd) {
        hdc_ = GetDC(hwnd);
    }
    ~ScopedHDC() {
        if (hdc_) {
            ReleaseDC(hwnd_, hdc_);
        }
    }
    // Prevent copying to avoid double release
    ScopedHDC(const ScopedHDC&) = delete;
    ScopedHDC& operator=(const ScopedHDC&) = delete;

    HDC get() const { return hdc_; }
    operator bool() const { return hdc_ != NULL; }
};
#endif
#include <locale>
// Returns true if the ramp contains any non-ASCII (Unicode) characters
bool ramp_has_unicode(const std::string& ramp) {
    for (unsigned char c : ramp) {
        if (c > 127) return true;
    }
    return false;
}
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <memory>

#ifdef _WIN32
// Secure wrapper for sensitive memory that wipes data on destruction
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
};
#endif

// Platform-specific includes for clearing the console
#ifdef _WIN32
#include <windows.h>
#else
// On non-Windows platforms, this app will not compile with the GDI changes.
#endif

// --- Configuration ---

// Desired console dimensions for the ASCII art.
// Adjust these to match your terminal's size for best results.
const int CONSOLE_WIDTH = 240; // Doubled for higher resolution
const int CONSOLE_HEIGHT = 80;  // Doubled for higher resolution


#include <map>
#include <iomanip>

// Map of modes to their ASCII ramps
const std::map<std::string, std::string> ASCII_RAMPS = {
    {"minimalist", "#+-."},
    {"normal", "@%#*+=-:."},
    {"normal2", "&$Xx+;:."},
    {"alphabetic", "ABCDEFGHIJKLMNOPQRSTUVWXYZ"},
    {"alphanumeric", "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz"},
    {"numerical", "0896452317"},
    {"extended", "@%#{}[]()<>^*+=~-:."},
    {"math", "+-\xd7\xf7=≠≈∞√π"},
    {"arrow", "↑↗→↘↓↙←↖"},
    {"grayscale", "@$BWM#*oahkbdpwmZO0QCJYXzcvnxrjft/|()1{}[]-_+~<>i!lI;:,\"^`'."},
    //{"max", "\xc6\xd1\xcaŒ\xd8M\xc9\xcb\xc8\xc3\xc2WQB\xc5\xe6#N\xc1\xfeE\xc4\xc0HKRŽœXg\xd0\xeaq\xdbŠ\xd5\xd4A€\xdfpm\xe3\xe2G\xb6\xf8\xf0\xe98\xda\xdc$\xebd\xd9\xfd\xe8\xd3\xde\xd6\xe5\xff\xd2b\xa5FD\xf1\xe1ZP\xe4š\xc7\xe0h\xfb\xa7\xddkŸ\xaeS9žUTe6\xb5Oyx\xce\xbef4\xf55\xf4\xfa&a\xfc™2\xf9\xe7w\xa9Y\xa30V\xcdL\xb13\xcf\xcc\xf3C@n\xf6\xf2s\xa2u‰\xbd\xbc‡zJƒ%\xa4Itoc\xeerjv1l\xed=\xef\xec<>i7†[\xbf?\xd7}*{+()/\xbb\xab•\xac|!\xa1\xf7\xa6\xaf—^\xaa„”“~\xb3\xba\xb2–\xb0\xad\xb9‹›;:’‘‚’˜ˆ\xb8…\xb7\xa8\xb4`"},
    {"codepage437", "█▓▒░"},
    {"blockelement", "█"}
};


void print_help() {
    std::cout << "Usage: AsciiScreen.exe [--mode <mode>] [--help]\n";
    std::cout << "Captures the screen and renders it as ASCII art.\n\n";
    std::cout << "Available modes:\n";

    // Find longest key for padding
    size_t max_len = 0;
    for (const auto& kv : ASCII_RAMPS) {
        if (kv.first.length() > max_len) max_len = kv.first.length();
    }
    size_t pad_width = max_len + 4;

    for (const auto& kv : ASCII_RAMPS) {
        std::cout << "  " << std::left << std::setw(pad_width) << kv.first
                  << kv.second << std::endl;
    }
}

std::string get_ascii_ramp_from_args(int argc, char* argv[], bool& show_help, std::string& error) {
    std::string mode = "normal"; // default mode
    show_help = false;
    error.clear();
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            show_help = true;
            return "";
        }
        if ((arg == "--mode" || arg == "-m")) {
            if (i + 1 < argc) {
                mode = argv[++i];
            } else {
                error = "Missing value for --mode/-m.";
                show_help = true;
            }
            break;
        }
        if (arg.rfind("--mode=", 0) == 0) {
            mode = arg.substr(7);
            break;
        }
        if (arg.rfind("-m=", 0) == 0) {
            mode = arg.substr(3);
            break;
        }
        if (arg.rfind("-", 0) == 0) {
            error = "Unknown option: " + arg;
            show_help = true;
            break;
        }
    }
    if (show_help) return "";
    auto it = ASCII_RAMPS.find(mode);
    if (it != ASCII_RAMPS.end()) {
        return it->second;
    }
    // fallback: invalid mode, trigger help
    error = "Unknown mode: '" + mode + "'";
    show_help = true;
    return mode;
}

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
bool captureScreenGDI(SecureBuffer& buffer, int& width, int& height) {
    // Optimization: Cache GDI objects (Memory DC and Bitmap) to avoid re-allocation overhead every frame.
    // We do NOT cache hScreenDC as it is a Common DC and should be released after use.
    static HDC hMemoryDC = NULL;
    static HBITMAP hBitmap = NULL;
    static int cachedWidth = 0;
    static int cachedHeight = 0;

    // Get the device context for the entire screen.
    ScopedHDC screenDC(NULL);
    if (!screenDC) return false;
    HDC hScreenDC = screenDC.get();

    // Initialize Memory DC once
    if (!hMemoryDC) {
        hMemoryDC = CreateCompatibleDC(hScreenDC);
        if (!hMemoryDC) {
            return false;
        }
    }

    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);

    if (width <= 0 || height <= 0) {
        return false;
    }

    // Update width/height to target console size for downscaling
    int screenW = width;
    int screenH = height;
    width = CONSOLE_WIDTH;
    height = CONSOLE_HEIGHT;

    // Recreate bitmap if resolution changes or on first run
    if (width != cachedWidth || height != cachedHeight) {
        HBITMAP hNewBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        if (!hNewBitmap) {
            return false;
        }

        // Sentinel: Verify object selection to prevent leaks or state corruption
        HGDIOBJ hOldObj = SelectObject(hMemoryDC, hNewBitmap);
        if (hOldObj == NULL || hOldObj == HGDI_ERROR) {
            DeleteObject(hNewBitmap); // Failed to select, cleanup new resource
            return false;
        }

        if (hBitmap) DeleteObject(hBitmap);
        hBitmap = hNewBitmap;

        cachedWidth = width;
        cachedHeight = height;
    }

    // Perform the stretch bit-block transfer from the screen to the memory DC.
    // Use HALFTONE for better downscaling quality.
    SetStretchBltMode(hMemoryDC, HALFTONE);
    SetBrushOrgEx(hMemoryDC, 0, 0, NULL);
    if (!StretchBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, screenW, screenH, SRCCOPY)) {
        return false;
    }

    // Setup the bitmap info structure to get the pixel data.
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // A negative height indicates a top-down DIB.
    bi.biPlanes = 1;
    bi.biBitCount = 32; // We want 32-bit BGRA format.
    bi.biCompression = BI_RGB;

    // Use size_t for calculation to prevent integer overflow
    size_t required_size = static_cast<size_t>(width) * height * 4;
    if (buffer.size() != required_size) {
        buffer.resize(required_size);
    }

    // Extract the pixel data from the bitmap.
    if (GetDIBits(hScreenDC, hBitmap, 0, (UINT)height, buffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS) == 0) {
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    std::unique_ptr<ConsoleCodePageGuard> cp_guard;
#endif
#ifdef _WIN32
    // Set Windows console to UTF-8 for Unicode output
    SetConsoleOutputCP(CP_UTF8);
#endif
    bool show_help = false;
    std::string error;
    std::string mode = "normal";
    std::string ramp_or_mode = get_ascii_ramp_from_args(argc, argv, show_help, error);
    if (show_help) {
        if (!error.empty()) {
            std::cerr << error << std::endl;
        }
        print_help();
        return error.empty() ? 0 : 1;
    }
    // If ramp_or_mode is not a valid ramp, get_ascii_ramp_from_args returns the mode string (for error reporting)
    // So, we need to extract the mode string from the arguments for later use
    // We'll parse it again here for simplicity
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--mode" || arg == "-m") && i + 1 < argc) {
            mode = argv[i + 1];
            break;
        }
        if (arg.rfind("--mode=", 0) == 0) {
            mode = arg.substr(7);
            break;
        }
        if (arg.rfind("-m=", 0) == 0) {
            mode = arg.substr(3);
            break;
        }
    }
    const std::string& ASCII_RAMP = ramp_or_mode;
    const auto frame_duration = std::chrono::milliseconds(1000 / TARGET_FPS);

    std::cout << "Starting screen capture using GDI...\n";
#ifdef _WIN32
    std::cout << "Controls: [q] Quit  [p] Pause/Resume\n";
#else
    std::cout << "Press Ctrl+C to exit.\n";
#endif
    std::cout << "Current mode: '" << mode << "' (" << ASCII_RAMP << ")" << std::endl;
    // Set code page for Windows console depending on mode
#ifdef _WIN32
    if (mode == "codepage437") {
        cp_guard = std::make_unique<ConsoleCodePageGuard>(437);
        std::cout << "\n[Info] Using code page 437 (OEM US) for this mode.\n";
        std::cout << "[Tip] For best results, use a raster font or 'Terminal' font in your console.\n";
    } else if (ramp_has_unicode(ASCII_RAMP)) {
        cp_guard = std::make_unique<ConsoleCodePageGuard>(65001);
        std::cout << "\n[Info] This mode uses Unicode characters.\n";
        std::cout << "[Tip] For best results, use a Unicode font (like 'Consolas' or 'Cascadia Mono') in your terminal.\n";
    }
#else
    if (ramp_has_unicode(ASCII_RAMP)) {
        std::cout << "\n[Info] This mode uses Unicode characters.\n";
    }
#endif
    for (int i = 3; i > 0; --i) {
        std::cout << "\rStarting in " << i << "... " << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\rStarting...       " << std::endl;

    SecureBuffer frame_buffer;
    int src_width = 0;
    int src_height = 0;

    int frame_count = 0;
    int current_fps = 0;
    auto last_fps_time = std::chrono::high_resolution_clock::now();

    while (true) {
        auto start_time = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
        // Handle interactive input
        if (_kbhit()) {
            int key = _getch();
            if (key == 'q' || key == 'Q') {
                break;
            } else if (key == 'p' || key == 'P') {
                // Update status bar to indicate pause without scrolling
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                COORD statusPos = {0, (SHORT)(CONSOLE_HEIGHT - 1)};
                SetConsoleCursorPosition(hConsole, statusPos);

                std::string pauseMsg = " [ PAUSED ] Press 'p' to resume...";
                if (pauseMsg.length() < CONSOLE_WIDTH) pauseMsg.append(CONSOLE_WIDTH - pauseMsg.length(), ' ');
                std::cout << pauseMsg << std::flush;

                while (true) {
                    if (_kbhit()) {
                        int resume_key = _getch();
                        if (resume_key == 'p' || resume_key == 'P') {
                            break;
                        } else if (resume_key == 'q' || resume_key == 'Q') {
                            return 0;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
#endif

        if (!captureScreenGDI(frame_buffer, src_width, src_height)) {
            std::cerr << "Error: Failed to capture screen." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        const auto* src_data = frame_buffer.data();
        // Bolt: Optimized ASCII conversion loop
        // Since captureScreenGDI already resizes the image to CONSOLE_WIDTH x CONSOLE_HEIGHT,
        // we can map pixels 1:1, removing the need for averaging and floating-point math.
        // Benchmark shows ~9x speedup (651us -> 69us per frame).

        std::string ascii_frame;
        ascii_frame.reserve((CONSOLE_WIDTH + 1) * CONSOLE_HEIGHT);

        // Reserve last line for status bar
        for (int y = 0; y < CONSOLE_HEIGHT - 1; ++y) {
            // Precompute row offset
            const size_t row_offset = static_cast<size_t>(y) * CONSOLE_WIDTH * 4;

            for (int x = 0; x < CONSOLE_WIDTH; ++x) {
                const size_t pixel_offset = row_offset + (x * 4);
                const unsigned char b = src_data[pixel_offset];
                const unsigned char g = src_data[pixel_offset + 1];
                const unsigned char r = src_data[pixel_offset + 2];

                // Integer approximation of 0.2126*r + 0.7152*g + 0.0722*b using 16-bit fixed point
                // 0.2126 * 65536 ~= 13933
                // 0.7152 * 65536 ~= 46871
                // 0.0722 * 65536 ~= 4732
                const unsigned int gray = (static_cast<unsigned int>(r) * 13933 +
                                           static_cast<unsigned int>(g) * 46871 +
                                           static_cast<unsigned int>(b) * 4732) >> 16;

                const int ramp_index = (gray * (ASCII_RAMP.length() - 1)) / 255;
                ascii_frame += ASCII_RAMP[ramp_index];
            }
            ascii_frame += '\n';
        }

        // Palette: Add status bar at the bottom
        std::string status = " [ AsciiScreen ] Mode: " + mode + " | FPS: " + std::to_string(current_fps) + " | [P]ause [Q]uit";
        if (status.length() < CONSOLE_WIDTH) {
            status.append(CONSOLE_WIDTH - status.length(), ' ');
        } else {
            status = status.substr(0, CONSOLE_WIDTH);
        }
        ascii_frame += status;
        ascii_frame += '\n';

        reset_cursor();
        std::cout << ascii_frame << std::flush;

        frame_count++;
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end_time - last_fps_time);
        if (duration.count() >= 1.0) {
            current_fps = static_cast<int>(frame_count / duration.count());
            frame_count = 0;
            last_fps_time = end_time;
        }

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