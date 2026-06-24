<div align="center">
  <img src="media/v0.3d-cropped.png" alt="Sardine Logo" style="padding:10px" width="220">
  <h1>Sardine Programming Language</h1>
  <p><strong>A modern, object-oriented, dynamically-typed scripting language with a high-performance C++23 native execution backend and WebAssembly runtime.</strong></p>

  [![Sardine Tests](https://github.com/Souvik606/Mission-Sardine/actions/workflows/tests.yml/badge.svg)](https://github.com/Souvik606/Mission-Sardine/actions/workflows/tests.yml)
  [![C++ Standard](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
  [![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
</div>

---

## 📖 Overview

Sardine fuses the **syntactic elegance of Python** with the **deterministic performance and safety of a native C++ engine**.

Sardine is designed from the ground up as a robust system that decouples compilation and execution phases into modular, highly-testable components:
*   **DFA-based Lexical Analyzer**: Scans source code character-by-character to tokenize inputs and filter out syntax-neutral content.
*   **Pratt-based Syntax Analyzer**: Parses tokens against a custom context-free grammar using top-down operator precedence (Pratt parsing) to build a multi-dimensional **Abstract Syntax Tree (AST)**.
*   **Semantic Evaluator (Interpreter)**: Executes logic by recursively traversing the AST using hierarchical **Context Maps** and **Symbol Tables** for dynamic scope resolution.
*   **WebAssembly Core**: Runs the compilation pipeline directly inside a web worker, exposing educational visualizations such as tree inspectors and scope trackers.

---

## ✨ Features

### 1. Unified Dynamic Type System
*   **Primitive Types**: Signed 64-bit Integers (`long long` range) and IEEE 754 double-precision Floats.
*   **Logical Types**: Booleans (`True`/`False`) that map internally to numeric states but resolve dynamically.
*   **Immutable Strings**: Access characters by index, perform concatenation (`+`), repeat strings (`*`), and embed expression values using F-string templates (`$"Value of x is {x}"`).
*   **Heterogeneous Collections**: Nested `Lists` (arrays) and unordered `Dictionaries` (hash maps with string/number keys) featuring dedicated operator overloading (e.g., `+` to merge lists/dicts, `-` or `/` to drop keys/values).

### 2. Modern Control Flow & Pattern Matching
*   **Conditionals**: `when` (if), `orwhen` (else-if), and `otherwise` (else) blocks.
*   **Loops & Traversals**: 
    *   `cycle`: Step-based loops (e.g., `cycle i = 0:10` iterates $0$ to $10$).
    *   `during`: Expression-driven loops (while-construct).
    *   `trace`: Collection-driven foreach iteration (e.g., `trace item <- list`).
*   **Switch-Case**: `menu` constructs matching specific expressions via `choice` blocks (featuring fall-through execution) and a default `fallback` path.
*   **Jump Controls**: `yield` (return/emit), `proceed` (continue), and `escape` (break).

### 3. Native Object-Oriented Programming (OOP)
*   **Blueprint Models**: Declared using the `model` keyword.
*   **Chained Constructors**: Named `init` method with support for parameter default values and C++ style initializer lists (e.g., `init(name, age) { name: name, age: age }`).
*   **Granular Member Visibility**: 
    *   `open`: Universally public access.
    *   `guarded`: Accessible within subclasses (protected).
    *   `secret`: Strictly private to the defining model.
*   **Polymorphic Inheritance**: Single and multi-level class hierarchy resolution with runtime late-binding, parent method proxies via `super()`, and class inheritance check utilities (`is_a`).
*   **Dunder Operator Overloading**: Implement custom behaviors for arithmetic and comparisons using standard magic methods (e.g., `__add__`, `__sub__`, `__eq__`, `__neg__`, `__not__`).

### 4. Robust Sandboxing & Execution Safeguards
To prevent memory exhaustion and infinite runtimes, the execution engine enforces strict sandboxing rules:
*   **Iteration Ceilings**: Loops, traversals, and comprehensions are capped at **100,000 iterations**.
*   **Call Stack Boundaries**: Recursion is restricted to a depth of **100 active stack frames**.
*   **Parsing Complexity**: The maximum AST tree depth is limited to **60 levels**.
*   **Container Scaling**: Lists and Strings are capped at **1,000,000 elements/chars**; Dictionaries at **100,000 keys**.
*   **File Stream Safeguard**: Direct file reads are restricted to a maximum of **10 MB**.
*   *Note: All limits can be overridden for production workloads using the `--unbounded` CLI flag.*

### 5. Self-Hosted Standard Library (`stdlib`)
Sardine's core standard library is written entirely in Sardine itself, making the framework extensible:
*   `math.sad`: Exposes mathematical operations, factorials, exponents, and constant definitions.
*   `linalg.sad`: Dynamic linear algebra functions for matrix manipulations.
*   `stat.sad`: Basic statistical functions (mean, median, standard deviation).
*   `random.sad`: Pseudo-random number and selection generators.
*   `csv.sad`: File-based comma-separated value parsers (`CSVReader`, `CSVWriter`).

---

## ⚡ Performance Optimizations

To deliver native-level execution speed while maintaining a dynamic scripting layer, Sardine incorporates key runtime optimizations within its C++23 engine:

*   **Copy-on-Write (COW) Containers**: Lists and Dictionaries utilize COW semantics to prevent eager memory allocations during reads or when passed/returned by value, deferring copies until modifications occur.
*   **Static AST Dispatch (RTTI Elimination)**: Evaluates nodes using custom numeric type enums for static visitor lookup, bypassing expensive C++ run-time type information (`dynamic_cast` and RTTI) in the recursive evaluator.
*   **Thread-Local Object Pool**: Features a thread-local memory pool specifically tuned for allocating `Number` objects, mitigating heap fragmentation and speed bottlenecks during heavy math and loop iterations.
*   **Pointer-Keyed String Interning**: Interns all variable names and dictionary keys to enable $O(1)$ symbol table lookups and comparisons via quick pointer equality checks rather than character scanning.
*   **Resolved Member Caching**: Caches constructor entry points and polymorphic method offsets inside models to achieve constant-time OOP member access.
*   **In-Place Scoping & Loop Return Bypassing**: Reuses frame environments and bypasses dynamic result accumulations during loop cycles, keeping memory usage completely flat even over millions of iterations.

---

## 🛠️ Getting Started

### 📋 Prerequisites
*   [CMake](https://cmake.org/) (version 3.23 or higher)
*   A C++23 compliant compiler (e.g., GCC 13+, Clang 16+, or MSVC 2022+)
*   [Python 3](https://www.python.org/) (for running test and benchmark runner scripts)

---

### 🔨 Compilation

#### 🖥️ Windows (MinGW / GCC)
To compile using MinGW compiler paths explicitly:
```powershell
# Configure build directory
"C:\Program Files\CMake\bin\cmake.exe" -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="C:\mingw64\bin\g++.exe" -DCMAKE_BUILD_TYPE=Release

# Compile using 12 cores for rapid execution
"C:\Program Files\CMake\bin\cmake.exe" --build build -j 12
```

#### 🐧 macOS & Linux
```bash
# Configure build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compile
cmake --build build -j 12
```

---

### 🚀 Running Sardine

1.  **Launch Interactive Shell (REPL)**:
    ```bash
    ./build/MissionSardine
    ```
2.  **Run a Script File**:
    ```bash
    ./build/MissionSardine samples/main.sad
    ```

---

## 📝 Syntax Examples

### 1. Variables & Formatted Strings
```sardine
name = "Sardine"
version = 0.3
info = $"Welcome to {name} v{version}!"
show(info) # Welcome to Sardine v0.3!
```

### 2. Classes, Access Control & Initializers
```sardine
model Animal {
    open attr <name>
    guarded attr <age>
    secret attr <sound>

    init(name, age, sound) {
        name: name,
        age: age,
        sound: sound
    }

    open method speak() {
        show(name, "says", sound)
    }
}

model Dog: Animal {
    open attr <breed>

    init(name, age, sound, breed) {
        name: name,
        age: age,
        sound: sound,
        breed: breed
    }

    open method describe() {
        show($"This is {name}, a {breed}. Age: {age}")
    }
}

buddy = Dog("Buddy", 5, "Woof", "Golden Retriever")
buddy.speak()       # Buddy says Woof
buddy.describe()    # This is Buddy, a Golden Retriever. Age: 5
```

### 3. Comprehensions & Collection Operators
```sardine
# Generate squares of even numbers from 1 to 5
evens = [x ** 2 cycle x = 1:5 when x % 2 == 0]
show(evens) # [4, 16]

# Merge dictionaries
defaults = {"theme": "light", "fontSize": 12}
user_settings = {"theme": "dark"}
final_settings = defaults + user_settings
show(final_settings) # {"theme": "dark", "fontSize": 12}
```

### 4. Exception Handling
```sardine
risk {
    x = 10 / 0
} trap RunTimeError err {
    show("Interception successful!")
    show("Details: " + err.message)
    show("Traceback:\n" + err.traceback)
} clean {
    show("Cleanup completed.")
}
```

---

## 🔬 Telemetry & Benchmarking Suite

Sardine provides comprehensive command-line flags to capture and serialize pipeline diagnostics directly to `stderr`:

| CLI Flag | Scope | Description |
| :--- | :--- | :--- |
| `--unbounded` | Execution Sandbox | Deactivates all safety thresholds for iteration, stacks, and containers. |
| `--edu` | AST / Tokenizer | Outputs human-readable token lists and AST hierarchies for debugging. |
| `--json` | AST / Tokenizer | Encodes educational compiler diagnostics as structured JSON for IDE integrations. |
| `--timeprofile` | Performance | Benchmarks compiler phases (Lexer, Parser, Interpreter) in milliseconds. |
| `--memoryprofile`| Performance | Captures net heap memory delta per compiler stage in Kilobytes. |
| `--profile` | Performance | Aggregates both temporal and spatial profiles into a single diagnostics JSON. |

### Running the Test Suite
Sardine maintains a strict collection of feature and regression tests:
```bash
python tests/run_tests.py
```

### Running the Benchmark Suite
Compare execution speeds and heap memory signatures between Sardine and Python:
```bash
python tests/benchmarks/run_benchmarks.py
```

*Benchmark results are dynamically compiled and saved into [tests/benchmarks/benchmark_results.md](tests/benchmarks/benchmark_results.md).*

---

## 🌐 WebAssembly Editor & Developer Tools
Sardine comes integrated with a browser-based development workbench situated in the `/web` directory:
*   **WebAssembly Compilation Core**: Operates within thread-isolated Web Workers to process compilation inputs without impacting browser page interactivity.
*   **Educational Mode Features**:
    *   **Token Explorer**: Visualizes the input character streams mapped to lexical identifiers.
    *   **Interactive AST Visualizer**: Draws the parsed hierarchical tree structures, complete with expansion, auto-centering, and node collapsing.
    *   **Execution Flow Visualizer**: Tracks step-by-step instructions through active scopes, highlighting local scopes and active context maps.

To run the web editor locally:
1.  Navigate to `/web` in a terminal or web server utility.
2.  Launch a simple web-server (e.g. `python -m http.server 8000`) and navigate to `http://localhost:8000`.

---

## 📜 License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
