from typing import Callable

def sign(x: float) -> int:
    return 1 if x >= 0 else -1


def di(f: Callable[[float], float],
       a: float,
       b: float,
       epsilon: float = 1e-10) -> float:

    if sign(f(a)) == sign(f(b)):
        raise ValueError("L'intervalle ne contient pas nécessairement de racine.")

    m = (a + b) / 2

    if abs(f(m)) < epsilon or abs(b - a) < epsilon:
        return m

    if sign(f(a)) == sign(f(m)):
        return di(f, m, b, epsilon)

    else:
        return di(f, a, m, epsilon)