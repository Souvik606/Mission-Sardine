def rle(s):
    n = len(s)
    if n == 0:
        return ""
        
    result = ""
    current_char = s[0]
    count = 1
    
    for i in range(1, n):
        char = s[i]
        if char == current_char:
            count += 1
        else:
            result = result + str(count) + current_char
            current_char = char
            count = 1
            
    result = result + str(count) + current_char
    return result

test_str = ("A" * 5000) + ("B" * 5000) + ("C" * 5000) + ("D" * 5000) + ("E" * 5000) + ("F" * 5000)
print("rle len =", len(rle(test_str)))
