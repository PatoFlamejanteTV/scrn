#ifdef _WIN32
#include <windows.h>
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
bool captureScreenGDI(std::vector<unsigned char>& buffer, int& width, int& height) {
    // Optimization: Cache GDI objects (Memory DC and Bitmap) to avoid re-allocation overhead every frame.
    // We do NOT cache hScreenDC as it is a Common DC and should be released after use.
    static HDC hMemoryDC = NULL;
    static HBITMAP hBitmap = NULL;
    static int cachedWidth = 0;
    static int cachedHeight = 0;

    // Get the device context for the entire screen.
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) return false;

    // Initialize Memory DC once
    if (!hMemoryDC) {
        hMemoryDC = CreateCompatibleDC(hScreenDC);
        if (!hMemoryDC) {
            ReleaseDC(NULL, hScreenDC);
            return false;
        }
        // Set up scaling mode for high quality downsampling
        SetBrushOrgEx(hMemoryDC, 0, 0, NULL);
        SetStretchBltMode(hMemoryDC, HALFTONE);
    }

    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    // Performance optimization:
    // Instead of capturing the full screen and downsampling in CPU (slow),
    // we use GDI to stretch-blit the screen to the target console size (fast).
    // This reduces data transfer from ~8MB (1080p) to ~76KB, significantly improving FPS.
    int target_width = CONSOLE_WIDTH;
    int target_height = CONSOLE_HEIGHT;

    if (screen_width <= 0 || screen_height <= 0) {
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Recreate bitmap if target size changes
    if (target_width != cachedWidth || target_height != cachedHeight) {
        HBITMAP hNewBitmap = CreateCompatibleBitmap(hScreenDC, target_width, target_height);
        if (!hNewBitmap) {
            ReleaseDC(NULL, hScreenDC);
            return false;
        }

        SelectObject(hMemoryDC, hNewBitmap);
        if (hBitmap) DeleteObject(hBitmap);
        hBitmap = hNewBitmap;

        cachedWidth = target_width;
        cachedHeight = target_height;
    }

    // Perform the stretch blit transfer from the screen to the memory DC.
    // GDI handles the downsampling here.
    if (!StretchBlt(hMemoryDC, 0, 0, target_width, target_height, hScreenDC, 0, 0, screen_width, screen_height, SRCCOPY)) {
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Setup the bitmap info structure to get the pixel data.
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = target_width;
    bi.biHeight = -target_height; // A negative height indicates a top-down DIB.
    bi.biPlanes = 1;
    bi.biBitCount = 32; // We want 32-bit BGRA format.
    bi.biCompression = BI_RGB;

    // Use size_t for calculation to prevent integer overflow
    size_t required_size = static_cast<size_t>(target_width) * target_height * 4;
    if (buffer.size() != required_size) {
        buffer.resize(required_size);
    }

    // Extract the pixel data from the bitmap.
    GetDIBits(hScreenDC, hBitmap, 0, (UINT)target_height, buffer.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // Release the screen DC as we are done with it for this frame
    ReleaseDC(NULL, hScreenDC);

    // Update output params
    width = target_width;
    height = target_height;

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

    std::cout << "Starting screen capture using GDI... Press Ctrl+C to exit.\n";
    std::cout << "Current mode: " << ASCII_RAMP << std::endl;
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
                        // Use size_t to prevent overflow if image dimensions are large
                        const auto pixel_offset = (static_cast<size_t>(sy) * src_width + sx) * 4;
                        const unsigned char b = src_data[pixel_offset];
                        const unsigned char g = src_data[pixel_offset + 1];
                        const unsigned char r = src_data[pixel_offset + 2];

                        // Integer approximation of 0.2126*r + 0.7152*g + 0.0722*b using 16-bit fixed point
                        // 0.2126 * 65536 ~= 13933
                        // 0.7152 * 65536 ~= 46871
                        // 0.0722 * 65536 ~= 4732
                        total_gray += (static_cast<unsigned int>(r) * 13933 +
                                       static_cast<unsigned int>(g) * 46871 +
                                       static_cast<unsigned int>(b) * 4732) >> 16;
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