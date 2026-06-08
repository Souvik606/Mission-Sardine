# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4783s | 5.90 MB | 0.07 ms | 0.19 ms | 1468.72 ms | 25.22x slower |
|  | Python | 0.0586s | 10.58 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.0074s | 5.88 MB | 0.10 ms | 0.23 ms | 0.11 ms | 0.07x slower |
|  | Python | 0.1065s | 18.26 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0489s | 5.85 MB | 0.12 ms | 0.23 ms | 41.76 ms | 1.92x slower |
|  | Python | 0.0255s | 10.62 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 11.1885s | 5.91 MB | 0.20 ms | 0.36 ms | 11181.03 ms | 30.09x slower |
|  | Python | 0.3719s | 10.73 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.3699s | 8.61 MB | 0.15 ms | 0.27 ms | 361.58 ms | 11.35x slower |
|  | Python | 0.0326s | 11.19 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2757s | 7.53 MB | 0.24 ms | 0.42 ms | 267.70 ms | 9.62x slower |
|  | Python | 0.0287s | 10.84 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0713s | 5.84 MB | 0.12 ms | 0.27 ms | 63.50 ms | 2.68x slower |
|  | Python | 0.0266s | 10.70 MB | - | - | - | - |
