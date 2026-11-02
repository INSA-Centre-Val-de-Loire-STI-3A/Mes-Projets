# TD1 — Types de base, expressions et fonctions

Chapitre 34 du polycopié. Notions : `def` / `return`, docstrings, annotations de type,
`assert` pour la programmation défensive (préconditions) et comme tests unitaires.

| Fichier | Exercice |
|---|---|
| `src/conversion.py` | 34.1 `C_to_F` / `F_to_C`, préconditions sur le zéro absolu · 34.2 `isalmost` : comparer deux flottants à `d` près |
| `src/greatest_root.py` | 34.3.1 `greatest_root(a, b, c)` : plus grande racine réelle de ax² + bx + c, `None` si Δ < 0 |

À faire : 34.3.2 `roots(a, b, c)`, qui renvoie toujours un tuple (vide, une ou deux racines).

## Points à retenir

- `5/9*(F-32)` et `5*(F-32)/9` ne donnent pas toujours le même flottant :
  `conversion.py` affiche les valeurs de `F` où elles diffèrent. D'où `isalmost`.
- Les assertions vérifient les préconditions sans message redondant ; le `try` de
  `greatest_root.py` vérifie que l'appel avec `a = 0` est bien refusé.

```sh
python3 src/conversion.py
python3 src/greatest_root.py
```
