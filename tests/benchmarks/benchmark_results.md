# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 2.0713s | 6.13 MB | 0.05 ms | 0.16 ms | 2062.82 ms | 32.67x slower |
|  | Python | 0.0634s | 10.97 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.0084s | 5.88 MB | 0.11 ms | 0.23 ms | 0.08 ms | 0.33x slower |
|  | Python | 0.0252s | 10.98 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0661s | 12.56 MB | 0.14 ms | 0.23 ms | 57.55 ms | 2.35x slower |
|  | Python | 0.0281s | 10.96 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 17.4465s | 433.24 MB | 0.23 ms | 0.37 ms | 17436.87 ms | 43.82x slower |
|  | Python | 0.3981s | 11.10 MB | - | - | - | - |
| Matrix Multiplication | Sardine | 0.5937s | 120.45 MB | 0.15 ms | 0.30 ms | 577.01 ms | 16.78x slower |
|  | Python | 0.0354s | 11.55 MB | - | - | - | - |
| Binary Search Tree | Sardine | 0.3695s | 8.68 MB | 0.22 ms | 0.43 ms | 360.53 ms | 12.27x slower |
|  | Python | 0.0301s | 11.19 MB | - | - | - | - |
| Run-Length Encoding | Sardine | 0.1217s | 25.72 MB | 0.13 ms | 0.26 ms | 113.62 ms | 4.85x slower |
|  | Python | 0.0251s | 11.01 MB | - | - | - | - |
