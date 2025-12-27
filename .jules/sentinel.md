## 2024-05-24 - [GDI Resource Management]
**Vulnerability:** Windows GDI handles (like `HDC`) are raw resources that must be manually released. If an exception occurs (e.g., `std::bad_alloc` during buffer resize), the release call is skipped, leading to a handle leak.
**Learning:** Even in simple loops, exception safety requires RAII to guarantee resource cleanup. This is critical for long-running processes like screen capture where leaks cause eventual failure.
**Prevention:** Always wrap Windows handles in RAII classes (e.g., `ScopedHDC`) to ensure `ReleaseDC` is called in the destructor.
