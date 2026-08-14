---
name: Native game direction
description: The project is a native C++ 3D game targeting Android first, not a web application.
---

- **Rule:** Keep gameplay code in C++ and target Android first.
  **Why:** The user explicitly chose a native app/game and selected Android as the first phone platform.
  **How to apply:** Prefer C++/raylib-compatible workflows, add touch controls, and avoid replacing the project with a web UI.