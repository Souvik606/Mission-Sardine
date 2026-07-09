# Sardine Benchmark Results

| Benchmark             | Language | Time (s) | Memory (MB) | Lex (ms) | Parse (ms) | Interpret (ms) | Sardine/Python Ratio |
| -----------------------| ----------| ----------| -------------| ----------| ------------| ----------------| ----------------------|
| Recursive Fibonacci   | Sardine  | 2.4136s  | 6.10 MB     | 0.57 ms  | 1.02 ms    | 2355.59 ms     | 13.67x slower        |
|                       | Python   | 0.1766s  | 9.40 MB     | -        | -          | -              | -                    |
| Prime Sieve           | Sardine  | 3.9253s  | 66.37 MB    | 0.61 ms  | 0.64 ms    | 3902.04 ms     | 21.23x slower        |
|                       | Python   | 0.1849s  | 17.06 MB    | -        | -          | -              | -                    |
| Mandelbrot Fractal    | Sardine  | 0.0658s  | 5.91 MB     | 0.45 ms  | 0.53 ms    | 49.06 ms       | 1.44x slower         |
|                       | Python   | 0.0458s  | 9.47 MB     | -        | -          | -              | -                    |
| Fannkuch Permutations | Sardine  | 10.5751s | 5.95 MB     | 0.50 ms  | 0.67 ms    | 10557.63 ms    | 15.51x slower        |
|                       | Python   | 0.6817s  | 9.53 MB     | -        | -          | -              | -                    |
| Matrix Multiplication | Sardine  | 0.4047s  | 8.68 MB     | 0.50 ms  | 0.67 ms    | 384.33 ms      | 5.74x slower         |
|                       | Python   | 0.0705s  | 9.95 MB     | -        | -          | -              | -                    |
| Binary Search Tree    | Sardine  | 0.3248s  | 7.85 MB     | 0.74 ms  | 1.00 ms    | 302.44 ms      | 5.20x slower         |
|                       | Python   | 0.0625s  | 9.74 MB     | -        | -          | -              | -                    |
| Run-Length Encoding   | Sardine  | 0.0905s  | 5.87 MB     | 0.64 ms  | 0.68 ms    | 69.14 ms       | 1.73x slower         |
|                       | Python   | 0.0523s  | 9.50 MB     | -        | -          | -              | -                    |
