#ifndef MOCK_WINDOWS_H
#define MOCK_WINDOWS_H
#define _WIN32 1
#include <cstddef>
#include <cstring>
typedef unsigned int UINT;
typedef void* HANDLE;
typedef void* HDC;
typedef void* HWND;
typedef void* HBITMAP;
typedef void* HGDIOBJ;
typedef int BOOL;
typedef short SHORT;
#ifndef NULL
#define NULL 0
#endif
#define INVALID_HANDLE_VALUE (HANDLE)-1
#define STD_OUTPUT_HANDLE -11
#define CP_UTF8 65001
#define HGDI_ERROR (HGDIOBJ)-1
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define HALFTONE 4
#define SRCCOPY 0xCC0020
#define BI_RGB 0
#define DIB_RGB_COLORS 0
struct COORD { SHORT X; SHORT Y; };
struct CONSOLE_SCREEN_BUFFER_INFO { COORD dwSize; };
struct BITMAPINFOHEADER { int biSize; int biWidth; int biHeight; int biPlanes; int biBitCount; int biCompression; };
struct BITMAPINFO { BITMAPINFOHEADER bmiHeader; };

// Shared spy variables declared in the test file, we need extern access if we were separating compilation units,
// but since we include main.cpp, we can just define the function here.
// However, to share state with the test runner, we need the test runner to access the spy vars.
// We can declare them extern here and define in the test file.
extern size_t last_wiped_size;
extern void* last_wiped_ptr;

inline void SecureZeroMemory(void* ptr, size_t cnt) {
    last_wiped_ptr = ptr;
    last_wiped_size = cnt;
    std::memset(ptr, 0, cnt);
}

inline UINT GetConsoleOutputCP() { return 0; }
inline BOOL SetConsoleOutputCP(UINT) { return 1; }
inline HDC GetDC(HWND) { return 0; }
inline int ReleaseDC(HWND, HDC) { return 1; }
inline HANDLE GetStdHandle(int) { return 0; }
inline BOOL SetConsoleCursorPosition(HANDLE, COORD) { return 1; }
inline HDC CreateCompatibleDC(HDC) { return 0; }
inline int GetSystemMetrics(int) { return 100; }
inline HBITMAP CreateCompatibleBitmap(HDC, int, int) { return 0; }
inline HGDIOBJ SelectObject(HDC, HGDIOBJ) { return 0; }
inline BOOL DeleteObject(HGDIOBJ) { return 1; }
inline int SetStretchBltMode(HDC, int) { return 1; }
inline BOOL SetBrushOrgEx(HDC, int, int, void*) { return 1; }
inline BOOL StretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, int) { return 1; }
inline int GetDIBits(HDC, HBITMAP, UINT, UINT, void*, BITMAPINFO*, UINT) { return 1; }
inline BOOL GetConsoleScreenBufferInfo(HANDLE, CONSOLE_SCREEN_BUFFER_INFO*) { return 1; }
#endif
