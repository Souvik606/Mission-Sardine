# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4425s | 5.61 MB | 0.05 ms | 0.13 ms | 1434.75 ms | 25.13x slower |
|  | Python | 0.0574s | 10.37 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.0071s | 5.58 MB | 0.10 ms | 0.23 ms | 0.12 ms | 0.06x slower |
|  | Python | 0.1103s | 18.06 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0473s | 5.54 MB | 0.12 ms | 0.22 ms | 40.56 ms | 1.99x slower |
|  | Python | 0.0238s | 10.41 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 12.7958s | 5.61 MB | 0.19 ms | 0.34 ms | 12786.77 ms | 34.25x slower |
|  | Python | 0.3736s | 10.53 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.3763s | 8.31 MB | 0.11 ms | 0.23 ms | 367.94 ms | 11.58x slower |
|  | Python | 0.0325s | 10.96 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2404s | 7.34 MB | 0.23 ms | 0.37 ms | 231.82 ms | 9.24x slower |
|  | Python | 0.0260s | 10.63 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0653s | 5.55 MB | 0.11 ms | 0.22 ms | 57.72 ms | 2.76x slower |
|  | Python | 0.0237s | 10.46 MB | - | - | - | - |
