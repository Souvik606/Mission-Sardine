# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4591s | 5.89 MB | 0.16 ms | 0.29 ms | 1450.22 ms | 23.79x slower |
|  | Python | 0.0613s | 10.55 MB | - | - | - | - |
| Prime Sieve | Sardine | 3.9019s | 66.31 MB | 0.21 ms | 0.32 ms | 3893.97 ms | 34.96x slower |
|  | Python | 0.1116s | 18.22 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0504s | 5.83 MB | 0.24 ms | 0.33 ms | 41.73 ms | 1.74x slower |
|  | Python | 0.0289s | 10.58 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 9.8272s | 5.90 MB | 0.34 ms | 0.47 ms | 9817.99 ms | 25.92x slower |
|  | Python | 0.3792s | 10.71 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.3733s | 8.62 MB | 0.25 ms | 0.36 ms | 363.95 ms | 10.69x slower |
|  | Python | 0.0349s | 11.14 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2540s | 7.77 MB | 0.33 ms | 0.49 ms | 244.87 ms | 8.02x slower |
|  | Python | 0.0317s | 10.80 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0682s | 5.80 MB | 0.22 ms | 0.31 ms | 60.22 ms | 2.44x slower |
|  | Python | 0.0280s | 10.66 MB | - | - | - | - |
