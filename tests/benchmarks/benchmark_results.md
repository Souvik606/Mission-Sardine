# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4365s | 5.93 MB | 0.16 ms | 0.28 ms | 1428.38 ms | 25.09x slower |
|  | Python | 0.0572s | 10.60 MB | - | - | - | - |
| Prime Sieve | Sardine | 4.0623s | 66.31 MB | 0.23 ms | 0.31 ms | 4054.25 ms | 38.76x slower |
|  | Python | 0.1048s | 18.29 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0499s | 5.84 MB | 0.22 ms | 0.28 ms | 42.68 ms | 2.09x slower |
|  | Python | 0.0239s | 10.66 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 9.5783s | 5.93 MB | 0.32 ms | 0.49 ms | 9569.63 ms | 25.74x slower |
|  | Python | 0.3721s | 10.73 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.3745s | 8.47 MB | 0.25 ms | 0.34 ms | 363.21 ms | 11.41x slower |
|  | Python | 0.0328s | 11.21 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2434s | 7.56 MB | 0.37 ms | 0.54 ms | 233.73 ms | 8.14x slower |
|  | Python | 0.0299s | 10.84 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0712s | 5.83 MB | 0.24 ms | 0.34 ms | 61.15 ms | 2.86x slower |
|  | Python | 0.0249s | 10.70 MB | - | - | - | - |
