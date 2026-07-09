# Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 1.4406s | 5.96 MB | 0.25 ms | 0.23 ms | 1431.88 ms | 25.42x slower |
|  | Python | 0.0567s | 10.62 MB | - | - | - | - |
| Prime Sieve | Sardine | 4.0537s | 66.38 MB | 0.30 ms | 0.34 ms | 4045.36 ms | 39.52x slower |
|  | Python | 0.1026s | 18.28 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0483s | 5.90 MB | 0.22 ms | 0.31 ms | 40.87 ms | 2.08x slower |
|  | Python | 0.0232s | 10.63 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 9.5499s | 5.95 MB | 0.35 ms | 0.44 ms | 9541.04 ms | 25.66x slower |
|  | Python | 0.3722s | 10.77 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.3580s | 8.68 MB | 0.23 ms | 0.35 ms | 348.86 ms | 11.20x slower |
|  | Python | 0.0320s | 11.19 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.2545s | 7.83 MB | 0.35 ms | 0.50 ms | 245.18 ms | 8.42x slower |
|  | Python | 0.0302s | 10.86 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.0740s | 5.89 MB | 0.22 ms | 0.30 ms | 66.32 ms | 3.06x slower |
|  | Python | 0.0242s | 10.68 MB | - | - | - | - |
