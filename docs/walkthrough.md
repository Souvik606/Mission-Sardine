# Walkthrough - Web Playground SPA Upgrade

We have completed the restructuring of the Mission-Sardine Web Playground into a modern Single Page Application (SPA) with a custom syntax-highlighted Monaco Editor.

## Changes Made

### 1. File Deprecation & Reorganization
- Copied the legacy playground implementation to [oldindex.html](file:///c:/Users/arkop/CLionProjects/Mission-Sardine/web/oldindex.html).
- Created a brand-new, clean [index.html](file:///c:/Users/arkop/CLionProjects/Mission-Sardine/web/index.html) as the primary entry point.

### 2. SPA Hash Routing
- Configured a lightweight client-side router supporting `#/` (Home), `#/docs` (Documentation), `#/editor` (Playground), and `#/educational` (Educational Mode).
- Built persistent view states toggled via CSS classes, which avoids reloading the Web Worker thread and maintains Monaco Editor memory context during navigation.

### 3. Regular Editor Workspace (`#/editor`)
- **Monaco Editor Integration**: Loaded via standard AMD loader CDN. Registered `sardine` as a custom language with Monarch token rules.
- **Custom Theme**: Created the `sardine-dark` theme. Color tokens (such as `keyword`, `typeKeywords`, `operator`, `number`, `string`, `comment`, and `editor.background`) are defined dynamically, enabling straightforward custom styling.
- **Inline Writable Terminal Input**: Removed the modal prompt. User inputs (`listen()`) are now typed directly in a terminal command row (`#terminal-input-row`) at the bottom of the black terminal box.
- **Sardine Logo**: Copied the repository's logo file `media/v0.3d-cropped.png` directly into the web folder, resolving asset rendering in the page header.
- **Clear Terminal Switch**: Added a checkbox `Clear Terminal upon execution` that clears the console before running the code.
- **MEMFS Virtual Sidebar**: Lists files dynamically created by the running C++ runtime within the sandboxed environment.
- **Click-to-View Detail Popups**: Clicking a virtual file opens a modal overlay housing a read-only instance of Monaco Editor displaying the file contents.
- **Terminal Simulator & Controls**: Implemented stdout/stderr logging, Run/Stop action toggles, and Unbounded Mode configurations.

### 4. Interactive Virtual File Manager (`#/editor`)
- **Main Editor Integration**: Clicking any file in the sidebar switches the active view and loads its contents into the main editor (no longer using the read-only popup modal).
- **Default File**: Opening the editor automatically creates and selects `main.sad` populated with the default Sardine control flow example code.
- **Dynamic Syntax Highlighting**: Configured `selectFile()` to set the Monaco model language to `sardine` only if the filename ends in `.sad`. Non-Sardine files (like `.txt` or `.csv`) load in `plaintext` mode.
- **Interactive File Controls**: Added an inline **Add File** prompt button in the sidebar, along with individual **Download** and **Delete** actions for each file card (including `main.sad` which can now be fully deleted).
- **Run Button State Controls**: The **Run Code** button dynamically disables when viewing a non-`.sad` file (like `.txt` or `.csv`) or when no files are open, preventing runtime type mismatches.
- **File Icons**: Files ending in `.sad` are marked with a `file-code` icon, while data files use a `file-text` icon.
- **Sync & Save Persistence**: 
  - As the user types in Monaco, changes are auto-saved back to the active file state in memory.
  - When code is executed, the entire virtual files registry is serialized and sent to the Web Worker, which deletes stale files and writes the current registry into the WASM sandboxed MEMFS filesystem before launching.
  - After run completion, any files created or modified by the C++ engine in MEMFS are serialized back, merged into the UI sidebar list, and loaded in Monaco if active.
- **Empty State Overlay**: Added an overlay panel displaying `"Select a file to get started!"` when all files are deleted or no file is currently selected.
- **Active Filename Indicator**: Synchronized the header tab span (`#editor-active-filename`) to show the active filename instead of hardcoded labels.

### 5. Website Favicon & Inline Sidebar File Creation
- **Favicon Integration**: Linked the user's custom website icons from `web/media/icons` (Apple Touch icon, 32x32, 16x16, site manifest, and standard ICO) directly into the page's `<head>`.
- **Inline Sidebar Input**: Replaced the native blocking browser `prompt()` dialog with a Tailwind-styled inline text input card matching the active sidebar file cards.
- **Micro-interactions & UX**:
  - **Input Sanitization**: Dynamically filters out forbidden filename characters (`<`, `>`, `:` , `"`, `/`, `\`, `|`, `?`, `*`) instantly as the user types.
  - **Dynamic Icon Preview**: Automatically switches the preview icon between a purple `file-code` (for `.sad` files) and a gray `file-text` (for non-code files) dynamically as the user types.
  - **Inline Validation & Keyframes**: Detects duplicate filenames in real-time, changing the border color to red with an `alert-circle` error icon. If the user presses `Enter` on a duplicate, the input triggers a custom CSS shake animation (`@keyframes shake`) and retains focus.
  - **Focus & Blur Handling**: Scrolls the sidebar viewport automatically to ensure the input is visible. Pressing `Escape` or clicking away (blur) cancels creation and cleanly resets the file list layout.

### 6. Custom In-Browser Delete Confirmation Modal
- **Custom Modal Dialog**: Replaced the native browser blocking `confirm()` popup with a sleek, Tailwind-styled, in-browser warning modal (`#delete-confirm-modal`).
- **Interactive Handlers**:
  - **Cancel & Dismiss**: Clicking "Cancel" or clicking the dark backdrop overlay closes the modal.
  - **Confirmation Action**: Clicking the red "Delete" button executes the deletion, shifts the active editor file context if needed, and updates the virtual file explorer list.
  - **Keyboard Bindings**: Registered a global listener for the `Escape` key, which closes both the delete confirmation modal and the file viewer detail modal when active.
  - **Rich Styling**: Features an amber `alert-triangle` icon, red theme accents, matching slate glassmorphic colors, and dynamically prints the specific filename to be deleted in a styled monospace element.

### 7. Full-Screen Page Loading Overlay
- **Loading Overlay**: Implemented a modern full-screen gradient overlay (`#page-loading-overlay`) displaying a rotating indigo loading spinner, a pulsing Sardine logo, and an "Initializing Environment..." caption.
- **Asynchronous Fade Out**: Binds into Monaco Editor's ready state to smoothly fade out (`opacity-0` and CSS transition) and remove the overlay once Monaco initialization completes.
- **Fail-safe Timeout**: Configured a 3-second backup timeout to dismiss the loader in case of CDN latency or execution blocks, keeping the playground fully accessible.

### 8. Collapsible Sidebar & Drag-to-Resize Terminal Panel
- **Collapsible Sidebar**:
  - Implemented collapsible structures `#sidebar-expanded-content` and `#sidebar-collapsed-content` within the sidebar container.
  - Added toggle buttons (chevron-left for collapse and folder icon for expand).
  - Toggles width smoothly (`w-72` to `w-14`) via a clean CSS transition (`transition-all duration-300`).
  - Triggers Monaco Editor layout refreshes automatically after transitions to ensure the code canvas perfectly matches the screen constraints.
- **Drag-to-Resize Terminal Height**:
  - Added a horizontal drag handle (`#terminal-resize-handle`) along the top of the terminal sandbox panel.
  - Bound mouse dragging events (`mousedown`, `mousemove`, and `mouseup`) globally to adjust terminal panel height dynamically.
  - Constrained resizing between `60px` and `75%` of screen height.
  - Force-updates Monaco Editor's layout geometry in real-time as the user drags, keeping the code editor aligned with the resizing boundary.

### 9. IO Window Layout Tweak & Initialization Log Removal
- **Terminal Log Cleanups**: Removed the automated runtime status messages (`// Engine initialization...` and `// Sandbox WebAssembly execution engine loaded and ready.`) that previously populated the terminal console upon load.
- **Checkbox Relocation & Renaming**: Moved the "Clear terminal upon execution" checkbox out of the Action Bar and directly into the IO Window header next to the "Clear" button. Renamed it to "Clear window upon execution" to align with the panel label.

### 10. Educational Mode Layout & Features Integration
- **Educational Mode Layout (`#view-educational`)**: Replicated files sidebar, active filename headers, console output windows, and run controls matching the main playground editor.
- **Multi-Tab Right Panel**: Added a custom glassmorphic tab selector toggling between **Tokens** and **AST Tree** panels:
  - **Tokens View**: Structured data table showing compiler token types and stringified/mapped literal values.
  - **AST View**: Rendered using a high-performance D3.js vertical top-down tree layout displaying interactive parent-child connections. It features dynamic scale transitions on hover, custom node identification classes, and viewport auto-centering.
- **Execution Constraints**: Hardcoded unbounded execution mode to false (`unbounded = false`), and forces standard terminal consoles, tokens tables, and AST visualization layers to clear upon execution.
- **Split Resizing Splitters**: Implemented vertical drag splitter adjusting the terminal viewport, and horizontal drag handle resizing the right Info Sidebar dynamically.
- **Dual Monaco Editor Instances**: Configured a separate instance `eduEditorInstance` inside Monaco Configuration AMD module to prevent document canvas conflicts.

### 11. Interactive D3 AST Visualizer & Traversal Mode
- **D3.js Graph Canvas**: Replaced the basic static layout with a zoomable and pannable SVG canvas (`#edu-ast-tree-container`). Implemented smooth momentum zoom and drag controls.
- **Obsidian-Style Micro-Animations**:
  - Node cards scale up smoothly by 8% and glow indigo upon hover.
  - Clicking any node highlights it, lowers the opacity of all non-related nodes/links to 15% (background muting), and centers the viewport on the selected node.
  - Clicking empty canvas space resets the visualization focus and centers the root tree layout to top-center.
- **BFS Traversal Mode**:
  - Extracts the AST hierarchy into a flat BFS queue array (`buildBfsQueue`).
  - Added a floating controller toolbar containing BFS queue stepping (Previous / Next), a nodes count indicator (`Node X of Y`), and a reset view control.
  - Interactive keyboard listener captures `ArrowLeft` (step backward), `ArrowRight` (step forward), and `Escape` (reset exploration focus) when the AST tab is active, while automatically bypassing keyboard intercept rules if the user is actively typing in a Monaco editor or terminal stdin box.
- **AMD Loader Namespace Collision Resolution**: Swapped script execution order to load `d3.js` *before* Monaco's AMD loader script (`vs/loader.js`). This ensures `d3` initializes itself in the global namespace before Monaco defines `window.define`, resolving the collision where `window.d3` remained undefined.
- **AST Node Scaling & Overlap Bug Resolution**: Fixed a CSS override issue where `.ast-node.muted { transform: scale(0.9); }` was applied to the SVG group element (`.ast-node`). This overrode D3's inline `transform="translate(d.y, d.x)"` attribute, causing all muted cards in the graph to jump to `(0, 0)` and overlap on top of the root base node. The scale transform is now target-restricted to the child `.ast-node-card` element, preserving position coordinates.
- **Dynamic Node Card Expansion for Large Values**: Expanded the SVG `foreignObject` bounds container to `280x180` pixels and configured CSS transitions for card dimensions (`width: 160px` to `260px`, `height: 60px` to `150px`). When focused/highlighted, card values wrap automatically with scrollable vertical space to support viewing up to 300 characters cleanly without clipping or blurriness.
- **Rectangular Node Card Styling & Uniform Highlights**: Reconfigured AST cards to be completely rectangular (`rounded-none`) and removed the left-only thick border. Border glow highlights and shadow effects now wrap around all four edges uniformly for hovered and highlighted states.
- **High-Contrast Light Node Theme**: Styled card containers with a distinct light-grey background (`bg-slate-200/95` and border `border-slate-300`) and mapped dark-colored typography (`text-slate-500` for labels, `text-indigo-900` for node type tokens, and `text-emerald-800` for compiler values). This guarantees maximum contrast and readability on dark canvas backgrounds when cards are in non-highlighted or muted states.
