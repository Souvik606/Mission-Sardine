# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4253s | 5.93 MB | 0.06 ms | 0.17 ms | 1416.54 ms | 25.57x slower |
|  | Python | 0.0557s | 10.61 MB | - | - | - | - |
| Prime Sieve | Sardine | 3.8365s | 66.33 MB | 0.11 ms | 0.23 ms | 3829.07 ms | 36.89x slower |
|  | Python | 0.1040s | 18.27 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0490s | 5.87 MB | 0.13 ms | 0.24 ms | 41.20 ms | 2.04x slower |
|  | Python | 0.0241s | 10.65 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 9.6418s | 5.90 MB | 0.20 ms | 0.34 ms | 9634.28 ms | 26.74x slower |
|  | Python | 0.3606s | 10.74 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.3626s | 8.63 MB | 0.11 ms | 0.23 ms | 353.81 ms | 11.93x slower |
|  | Python | 0.0304s | 11.18 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2361s | 7.56 MB | 0.19 ms | 0.37 ms | 228.38 ms | 8.94x slower |
|  | Python | 0.0264s | 10.86 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0653s | 5.84 MB | 0.10 ms | 0.22 ms | 57.81 ms | 2.68x slower |
|  | Python | 0.0244s | 10.68 MB | - | - | - | - |
