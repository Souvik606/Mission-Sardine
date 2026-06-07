# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.5239s | 5.85 MB | 0.06 ms | 0.13 ms | 1517.00 ms | 27.30x slower |
|  | Python | 0.0558s | 10.68 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.0064s | 5.78 MB | 0.13 ms | 0.21 ms | 0.07 ms | 0.06x slower |
|  | Python | 0.1102s | 18.34 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0494s | 12.38 MB | 0.12 ms | 0.22 ms | 41.73 ms | 2.12x slower |
|  | Python | 0.0233s | 10.71 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 11.6610s | 432.99 MB | 0.19 ms | 0.35 ms | 11652.53 ms | 32.25x slower |
|  | Python | 0.3616s | 10.82 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.4131s | 120.21 MB | 0.11 ms | 0.23 ms | 405.04 ms | 13.34x slower |
|  | Python | 0.0310s | 11.26 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2467s | 8.44 MB | 0.22 ms | 0.38 ms | 238.93 ms | 9.43x slower |
|  | Python | 0.0262s | 10.92 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0746s | 25.48 MB | 0.11 ms | 0.21 ms | 67.67 ms | 3.20x slower |
|  | Python | 0.0233s | 10.76 MB | - | - | - | - |
