# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4186s | 5.89 MB | 0.05 ms | 0.14 ms | 1411.03 ms | 24.53x slower |
|  | Python | 0.0578s | 10.69 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.0072s | 5.83 MB | 0.11 ms | 0.23 ms | 0.12 ms | 0.07x slower |
|  | Python | 0.1039s | 18.38 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0486s | 5.83 MB | 0.12 ms | 0.23 ms | 41.59 ms | 2.08x slower |
|  | Python | 0.0234s | 10.74 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 9.9162s | 5.88 MB | 0.19 ms | 0.36 ms | 9908.37 ms | 26.35x slower |
|  | Python | 0.3763s | 10.83 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.3686s | 8.61 MB | 0.11 ms | 0.24 ms | 360.74 ms | 11.66x slower |
|  | Python | 0.0316s | 11.30 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2392s | 7.53 MB | 0.20 ms | 0.38 ms | 230.92 ms | 8.77x slower |
|  | Python | 0.0273s | 10.92 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0664s | 5.85 MB | 0.11 ms | 0.25 ms | 58.03 ms | 2.93x slower |
|  | Python | 0.0227s | 10.78 MB | - | - | - | - |
