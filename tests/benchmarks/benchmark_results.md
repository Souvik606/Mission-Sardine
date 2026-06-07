# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4875s | 5.82 MB | 0.05 ms | 0.14 ms | 1479.44 ms | 26.04x slower |
|  | Python | 0.0571s | 10.61 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.0081s | 5.76 MB | 0.11 ms | 0.22 ms | 0.11 ms | 0.08x slower |
|  | Python | 0.1012s | 18.28 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0544s | 12.59 MB | 0.12 ms | 0.24 ms | 45.16 ms | 2.33x slower |
|  | Python | 0.0234s | 10.64 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 13.3305s | 433.03 MB | 0.19 ms | 0.34 ms | 13297.30 ms | 36.62x slower |
|  | Python | 0.3640s | 10.78 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.4606s | 126.66 MB | 0.11 ms | 0.23 ms | 430.64 ms | 14.39x slower |
|  | Python | 0.0320s | 11.21 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2522s | 8.50 MB | 0.23 ms | 0.41 ms | 244.22 ms | 9.53x slower |
|  | Python | 0.0265s | 10.87 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0778s | 25.47 MB | 0.11 ms | 0.25 ms | 69.67 ms | 3.00x slower |
|  | Python | 0.0259s | 10.69 MB | - | - | - | - |
