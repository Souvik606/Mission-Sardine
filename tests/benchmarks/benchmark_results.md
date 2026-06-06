# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 2.0297s | 5.76 MB | 0.09 ms | 0.17 ms | 2016.75 ms | 20.96x slower |
|  | Python | 0.0968s | 9.37 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.0358s | 13.52 MB | 0.11 ms | 0.24 ms | 23.99 ms | 0.99x slower |
|  | Python | 0.0361s | 9.39 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.0699s | 12.28 MB | 0.12 ms | 0.23 ms | 57.80 ms | 1.39x slower |
|  | Python | 0.0503s | 9.43 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 17.0185s | 433.01 MB | 0.19 ms | 0.34 ms | 17000.97 ms | 30.87x slower |
|  | Python | 0.5513s | 9.45 MB | - | - | - | - |
