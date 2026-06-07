# Mission Sardine Benchmark Results

| Benchmark             | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
| -----------------------| ----------| ----------| -------------| ----------| ------------| ----------------| ----------------------|
| Recursive Fibonacci   | Sardine  | 1.4497s  | 5.82 MB     | 0.05 ms  | 0.16 ms    | 1435.37 ms     | 14.65x slower        |
|                       | Python   | 0.0989s  | 9.38 MB     | -        | -          | -              | -                    |
| Prime Sieve           | Sardine  | 0.0118s  | 5.74 MB     | 0.12 ms  | 0.28 ms    | 0.09 ms        | 0.08x slower         |
|                       | Python   | 0.1464s  | 17.03 MB    | -        | -          | -              | -                    |
| Mandelbrot Fractal    | Sardine  | 0.0510s  | 12.27 MB    | 0.14 ms  | 0.24 ms    | 36.49 ms       | 1.45x slower         |
|                       | Python   | 0.0352s  | 9.43 MB     | -        | -          | -              | -                    |
| Fannkuch Permutations | Sardine  | 11.3815s | 432.96 MB   | 0.26 ms  | 0.40 ms    | 11367.53 ms    | 21.70x slower        |
|                       | Python   | 0.5245s  | 9.49 MB     | -        | -          | -              | -                    |
| Matrix Multiplication | Sardine  | 0.4266s  | 120.17 MB   | 0.11 ms  | 0.25 ms    | 414.01 ms      | 8.97x slower         |
|                       | Python   | 0.0476s  | 9.93 MB     | -        | -          | -              | -                    |
| Binary Search Tree    | Sardine  | 0.3126s  | 8.35 MB     | 0.25 ms  | 0.42 ms    | 299.49 ms      | 7.07x slower         |
|                       | Python   | 0.0442s  | 9.72 MB     | -        | -          | -              | -                    |
| Run-Length Encoding   | Sardine  | 0.1078s  | 25.49 MB    | 0.12 ms  | 0.26 ms    | 95.42 ms       | 3.01x slower         |
|                       | Python   | 0.0358s  | 9.47 MB     | -        | -          | -              | -                    |
