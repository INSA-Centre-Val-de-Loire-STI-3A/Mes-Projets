def P(x, *a):
    print(x, *a, sep=",", end=";")
    return x

l = [1, 2]
m = P(l, P(l.append(3)))
m.append(4)
print(l)