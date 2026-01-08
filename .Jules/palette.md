## 2024-10-24 - Interactive Console Controls
**Learning:** Adding interactive controls (like 'q' for quit) to a continuous CLI loop significantly improves user agency, but requires platform-specific handling (e.g., `<conio.h>` on Windows) that must be guarded to preserve cross-platform buildability.
**Action:** Always wrap `_kbhit()` and `_getch()` in `#ifdef _WIN32` and provide visual cues to the user about available controls.

## 2024-10-25 - Persistent CLI Status Bar
**Learning:** Reserving the last line (`CONSOLE_HEIGHT - 1`) of a full-screen CLI refresh loop for a persistent status bar prevents scrolling artifacts and provides a stable area for critical information (Mode, Controls) that would otherwise be lost after the initial startup.
**Action:** When designing full-screen CLI apps, explicitly subtract 1 from the content loop height and append a padded status string to the frame buffer.

## 2025-02-23 - Skippable CLI Delays
**Learning:** Fixed startup delays (e.g., "Starting in 3...") can frustrate frequent users. Breaking the sleep into small polling intervals (e.g., 100ms) to check for input allows the delay to be skippable, respecting user time and agency.
**Action:** Replace `sleep(n)` with a loop of `sleep(100ms)` checks for `_kbhit()` when implementing startup countdowns.

## 2025-02-23 - Runtime Logic Updates
**Learning:** When allowing runtime configuration changes (like switching ASCII ramps), ensure dependent data structures (like lookup tables) are immediately recomputed within the same event loop cycle to avoid visual artifacts or inconsistent state.
**Action:** Group state update logic (e.g., variable assignment + lookup table regeneration) into a single block triggered by the input event.
