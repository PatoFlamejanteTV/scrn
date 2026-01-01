## 2024-10-24 - Interactive Console Controls
**Learning:** Adding interactive controls (like 'q' for quit) to a continuous CLI loop significantly improves user agency, but requires platform-specific handling (e.g., `<conio.h>` on Windows) that must be guarded to preserve cross-platform buildability.
**Action:** Always wrap `_kbhit()` and `_getch()` in `#ifdef _WIN32` and provide visual cues to the user about available controls.

## 2024-10-25 - Persistent CLI Status Bar
**Learning:** Reserving the last line (`CONSOLE_HEIGHT - 1`) of a full-screen CLI refresh loop for a persistent status bar prevents scrolling artifacts and provides a stable area for critical information (Mode, Controls) that would otherwise be lost after the initial startup.
**Action:** When designing full-screen CLI apps, explicitly subtract 1 from the content loop height and append a padded status string to the frame buffer.
