def matmul(n):
    A = [[float(i + j) for j in range(n)] for i in range(n)]
    B = [[float(i * j) for j in range(n)] for i in range(n)]
    C = [[0.0 for _ in range(n)] for _ in range(n)]
    
    for i in range(n):
        for j in range(n):
            s = 0.0
            for k in range(n):
                s += A[i][k] * B[k][j]
            C[i][j] = s
    return C[n - 1][n - 1]

print("matmul val =", matmul(60))
