<div align="center">
  <img src="media/v0.3d-cropped.png" alt="Sardine Logo" style="padding:10px" width="200">
  <br>
</div>

# Sardine: A Custom Language Made with C++

Sardine is a custom programming language built with C++. It combines a powerful **Lexer**,
**Parser**, and **Interpreter** to process and execute code written in our bespoke language. In
addition, Sardine offers an **Interactive Shell** for real-time coding, and will soon be offering a **REST API** for remote
execution and parsing, and a modern **Web Frontend** to enhance your development experience.

[![Sardine Tests](https://github.com/Souvik606/Mission-Sardine/actions/workflows/tests.yml/badge.svg)](https://github.com/Souvik606/Mission-Sardine/actions/workflows/tests.yml)

---

## Features

- **Interactive Shell**: A REPL to write, inspect, and execute Sardine code interactively.
- **File Input Programs**: Run code files by passing them to the compiler executable directly.
- **Full Language Parser/Interpreter**: Includes a tokenizer, parser, AST builder, and interpreter for language elements such as **if/else**, **while**, **for**, **switch-case**, **functions**, **classes**, and **exceptions**.
- **C++ Native Architecture**: Built in C++17 for optimal performance, modularity, and speed.
- **Stable Testable Core**: Modular separation of lexer, parser, AST, and interpreter makes unit testing and feature expansion straightforward.
- **Pluggable Extensions**: Designed for future language features, custom data types, and more complex control flow via clear node and token abstractions.
- **Developer-Friendly**: Includes meaningful error messaging and a concise shell workflow for experimentation.

## Prerequisites

- [CMake](https://cmake.org/) (v3.23 or higher)
- A modern C++ compiler supporting C++17 (e.g., GCC, Clang, or MSVC)
- [Git](https://git-scm.com/)

---

## Setup

1. **Clone the repository:**

   ```bash
   git clone https://github.com/Souvik606/Mission-Sardine.git
   cd Mission-Sardine
   ```

2. **Configure and Build:**

   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

3. **Run a Sardine program:**

   ```bash
   ./build/MissionSardine samples/main.sad
   ```

4. **Run the Interactive Shell (REPL):**

   ```bash
   ./build/MissionSardine
   ```

---

## Language Details

- Dynamically typed variables
- Default data types of Integer, Float, String, List, Dictionary
- Arithmetic, Bitwise and Logical operators
- Nestable, heterogeneous Lists and Dictionaries with List and Dictionary functions
- User-defined functions with recursion
- Object-oriented Programming with custom models, operator overloading, and inheritance
- Error Handling (`risk-trap-clean`)
- For Loops (`cycle`)
- While Loops (`during`)
- Switch-Case (`menu`)
- If-Else-Elif (`when-orwhen-otherwise`)

Please view `docs/grammar_rules.md` for details on all grammar rules. User manual for more friendly explanation of syntax is under construction.

---

## Future Plans

- **REST API (Planned)**: Exposes endpoints to execute code, inspect results, and retrieve AST trees remotely for integration with IDEs or services.
- **Web Frontend (Planned)**: A sleek, interactive UI for writing, visualizing, and debugging Sardine code.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Contributing

Contributions are welcome! Please read
our [Contributing Guidelines](CONTRIBUTING.md) to get started.
