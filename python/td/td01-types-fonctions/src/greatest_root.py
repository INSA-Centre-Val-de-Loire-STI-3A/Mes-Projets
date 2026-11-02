def greatest_root(a:float, b:float , c:float) -> float:
    """Return the greatest root of the polynomial ax^2 + bx + c = 0"""
    assert a != 0
    delta = b**2 - 4*a*c
    if delta < 0:
        return None
    
    return max ((-b + delta**0.5) / (2*a) , (-b - delta**0.5) / (2*a))

assert greatest_root(1, 1, 1) == None
assert greatest_root(1, -2, 1) == 1
assert greatest_root(4, -4, -24) == 3

try:
    greatest_root(0,3,5)
except AssertionError:
    pass
else:
    raise SystemExit("La precondtion n' a pas ete verifier")
