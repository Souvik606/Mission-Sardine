def fannkuch(n):
    p = list(range(n))
    q = list(range(n))
    s = list(range(n))
    sign = 1
    max_flips = 0
    sum_flips = 0
    
    while True:
        for i in range(n):
            q[i] = p[i]
            
        flips = 0
        f = q[0]
        while f != 0:
            i = 0
            j = f
            while i < j:
                temp = q[i]
                q[i] = q[j]
                q[j] = temp
                i += 1
                j -= 1
            flips += 1
            f = q[0]
            
        if flips > max_flips:
            max_flips = flips
        sum_flips += sign * flips
        
        if sign == 1:
            temp = p[0]
            p[0] = p[1]
            p[1] = temp
            sign = -1
        else:
            temp = p[1]
            p[1] = p[2]
            p[2] = temp
            sign = 1
            
            i = 2
            while i < n:
                sx = s[i]
                if sx != 0:
                    s[i] = sx - 1
                    break
                if i == n - 1:
                    return sum_flips
                s[i] = i
                t = p[0]
                j = 0
                while j < i:
                    p[j] = p[j + 1]
                    j += 1
                p[i] = t
                i += 1

print("fannkuch sum(9) =", fannkuch(9))
