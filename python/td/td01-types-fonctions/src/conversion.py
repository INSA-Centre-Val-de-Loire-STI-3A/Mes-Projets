def C_to_F(celsius: float) -> float:
    """Convert Celsius to Fahrenheit"""
    assert celsius >= -273.15, celsius
    return (celsius * 9/5) + 32

def F_to_C(fahrenheit: float) -> float:
    assert fahrenheit >= -459.67
    return (5/9 *(fahrenheit-32))

def isalmost(n, m, d=1e-13):
    """Teste si n et m sont à une distance au plus d l'un de l'autre."""
    return abs(n - m) <= d

print([ (a,b)
for F in range(20)
for a,b in [[5/9*(F-32), 5*(F-32)/9]]
if a!=b ])


assert all( isalmost(*(err := a)) for a in
    [(1, 1), (1, 1.1, .11)] +
    [(n, n - 1e-14) for n in range(1, 100)]
), err

assert not any( isalmost(*(err := a)) for a in
    [(1, 2), (1, 1.1), (5, 5.00008)] +
    [(n, n - 1e-12) for n in range(1, 100)]
), err

assert isalmost(F_to_C(-459.67), -273.15)
assert isalmost(C_to_F(-273.15), -459.67)
assert all( isalmost(efc := F_to_C(C_to_F(c)), ec := c)
            and isalmost(efc := C_to_F(F_to_C(c)), c)
            for c in range(-273, 200)
), (ec, efc)