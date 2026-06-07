def sieve(n):
    limit = n + 1
    primes = [True] * limit
    primes[0] = False
    primes[1] = False
    
    i = 2
    while i * i <= n:
        if primes[i]:
            j = i * i
            while j <= n:
                primes[j] = False
                j = j + i
        i += 1
        
    count = 0
    for k in range(2, n + 1):
        if primes[k]:
            count += 1
    return count

print("primes up to 1000000 =", sieve(1000000))
