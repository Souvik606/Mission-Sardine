# Mission Sardine Benchmark Results

| Benchmark | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
|---|---|---|---|---|---|---|---|
| Recursive Fibonacci | Sardine | 17.9357s | 6.68 MB | 0.35 ms | 0.79 ms | 17922.53 ms | 295.77x slower |
|  | Python | 0.0606s | 10.63 MB | - | - | - | - |
| Prime Sieve | Sardine | 0.6209s | 113.99 MB | 0.30 ms | 0.51 ms | 611.96 ms | 22.98x slower |
|  | Python | 0.0270s | 10.66 MB | - | - | - | - |
| Mandelbrot Fractal | Sardine | 0.3974s | 14.20 MB | 1.03 ms | 0.58 ms | 388.29 ms | 15.45x slower |
|  | Python | 0.0257s | 10.66 MB | - | - | - | - |
| Fannkuch Permutations | Sardine | 1.8065s | 67.33 MB | 0.68 ms | 1.10 ms | 1795.70 ms | 53.45x slower |
|  | Python | 0.0338s | 10.75 MB | - | - | - | - |
