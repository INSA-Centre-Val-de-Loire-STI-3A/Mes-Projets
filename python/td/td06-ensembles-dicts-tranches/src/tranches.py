s = [10, 20, 30, 40, 50]
print(s[1:3], s[:2], s[3:], s[-2:], s[::2], s[::-1])
print(s[10:20], len(s[1:4]))
try:
    print(s[10])
except IndexError:
    print("IndexError")
