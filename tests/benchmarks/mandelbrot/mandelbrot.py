def mandelbrot(size, iterations):
    count = 0
    for y in range(size):
        for x in range(size):
            cr = (x * 3.0 / size) - 1.5
            ci = (y * 3.0 / size) - 1.5
            zr = 0.0
            zi = 0.0
            i = 0
            while i < iterations and (zr * zr + zi * zi < 4.0):
                temp = zr * zr - zi * zi + cr
                zi = 2.0 * zr * zi + ci
                zr = temp
                i += 1
            if i == iterations:
                count += 1
    return count

print("mandelbrot count =", mandelbrot(30, 30))
