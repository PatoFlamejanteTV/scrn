# Sentinel Journal

This journal documents critical security learnings, patterns, and decisions for the AsciiScreen project.

## 2025-02-18 - GDI Resource Leaks on Exception
**Vulnerability:** In `captureScreenGDI`, memory allocation for the frame buffer was performed *after* acquiring GDI resources (Device Contexts). If `std::vector::resize` threw `std::bad_alloc`, the GDI handles would be leaked, potentially leading to resource exhaustion (DoS).
**Learning:** In C++, always perform operations that might throw exceptions (like memory allocation) *before* acquiring raw resources that require manual cleanup, or use RAII wrappers for those resources.
**Prevention:** Acquire resources as late as possible and release them as early as possible. Prefer RAII wrappers (e.g., smart pointers or custom guards) for legacy C-style handles.
