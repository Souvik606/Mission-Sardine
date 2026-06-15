# Implementation Plan - Web Playground SPA Upgrade

This plan outlines the architecture and execution steps to restructure the Mission-Sardine Web Playground into a beautiful, multi-page SPA (Single Page Application) using hash-based routing. We will start by deprecating the current index file and building a brand-new Monaco Editor-based regular editor (`#/editor`).

## User Review Required

> [!IMPORTANT]
> - **Monaco Editor Loading**: Monaco Editor will be loaded via standard CDN AMD loader (hosted by cdnjs). This eliminates any build-step requirements.
> - **SPA Worker Lifecycle**: We will run a single, persistent Web Worker instance for the WebAssembly interpreter. Navigating between hash pages (`#/`, `#/docs`, `#/editor`, `#/educational`) will **not** reload or terminate the worker, maintaining runtime responsiveness.
> - **Cross-Origin Isolation**: Since blocking inputs use `SharedArrayBuffer`, we must retain the `coi-serviceworker.js` registration at the top of the new entry point.

---

## Proposed Changes

### Web Playground Rewrite

We will rename the existing playground code to keep it as a reference and create the new structure.

#### [NEW] [oldindex.html](file:///c:/Users/arkop/CLionProjects/Mission-Sardine/web/oldindex.html)
- A direct copy of the original [index.html](file:///c:/Users/arkop/CLionProjects/Mission-Sardine/web/index.html) before the rewrite.

#### [MODIFY] [index.html](file:///c:/Users/arkop/CLionProjects/Mission-Sardine/web/index.html)
- Create a brand-new structure containing:
  - **Global Header**: Styled navbar featuring the Sardine logo, active route indicators, and links to Home (`#/`), Docs (`#/docs`), Editor (`#/editor`), and Educational (`#/educational`).
  - **SPA Hash Router**: A simple, vanilla JS client-side router mapping hash changes (`window.onhashchange`) to rendering target views.
  - **Persistent Web Worker State**: Global worker, `SharedArrayBuffer` for inputs, and file list cache.
  - **#/editor View**: 
    - Split-pane layout containing Monaco Editor.
    - Custom syntax highlighter/lexer token definition for Sardine in Monaco.
    - Interactive Terminal panel for stdin/stdout/stderr console logs.
    - Sidebar listing volatile sandboxed virtual files, with dynamic click-to-view file popups.
    - Toggle controls for Unbounded Mode.
  - **Placeholder Views**: Basic screens for Home (`#/`), Docs (`#/docs`), and Educational (`#/educational`) to be fully expanded in subsequent phases.

---

## Verification Plan

### Automated Tests
- Check console logs to ensure Monaco Editor initialization completes without errors.
* Verify Wasm worker loads correctly via network logs and outputs `// WebAssembly Sandbox interpreter engine successfully loaded!`.

### Manual Verification
- Deploy using the local Python server (`python -m http.server 8000`).
- Test code execution by typing code into Monaco Editor and clicking **Run**.
- Test interactive input by running the "Interactive Input" code example, confirming the input modal opens, takes input, and passes it via `SharedArrayBuffer` to stdout.
- Verify files created in the sandbox (MEMFS) show up in the virtual files list and open a view modal when clicked.
- Test hash routing navigation to confirm transition between page fragments works seamlessly.
