# TD5 — Générateurs et `None`

Chapitres 38 et 39 du polycopié. Notions : expressions génératrices, fonctions variadiques ;
`None` comme valeur des instructions à effet de bord, ordre d'évaluation des arguments.

| Fichier | Contenu |
|---|---|
| `src/mutation_listes.py` | (§39) `print` et `list.append` renvoient `None` ; les arguments sont évalués avant l'appel, et une liste partagée est modifiée en place |

À faire : (§38) `crange(a, b)`, générateur des caractères de `a` à `b` en une ligne ;
(§39) prédire la sortie du script à base de `print(print(2+2), ...)`.

```sh
python3 src/mutation_listes.py   # None;[1, 2, 3],None;[1, 2, 3, 4]
```
