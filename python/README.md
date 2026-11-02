# Programmation Python

Python 3 (3A STI) : types de base, fonctions et assertions, conteneurs, compréhensions,
générateurs, programmation orientée objet et méthodes spéciales.

Les TD suivent la partie « Python Exercises » du polycopié du cours (un TD par chapitre,
numéro de section entre parenthèses). Le polycopié n'est pas versionné ici.

| Dossier | Contenu |
|---|---|
| `td/td01-types-fonctions/` | (§34) Conversion Celsius ↔ Fahrenheit, comparaison de flottants, racines d'un trinôme |
| `td/td02-methodes-magiques/` | (§35) Lancers de dés D&D : classes et méthodes spéciales |
| `td/td03-flottants/` | (§36) Dichotomie, dérivée numérique, méthode de Newton–Raphson |
| `td/td04-comprehensions/` | (§37) Compréhensions, palindromes et autres *one-liners* |
| `td/td05-generateurs-none/` | (§38–39) Générateurs ; `None`, effets de bord et ordre d'évaluation |
| `td/td06-ensembles-dicts-tranches/` | (§40) Ensembles, dictionnaires et tranches |
| `td/td07-what-the-what/` | (§41) Lire, corriger et simplifier du code étudiant |
| `td/td08-matrices-creuses/` | (§42) Encapsulation d'une matrice creuse dans une classe |
| `td/td09-ensembles/` | (§43) Ensemble des parties d'un ensemble |
| `td/td10-primitives/` | (§44) Approximation numérique de primitives |
| `td/td11-calcul-formel/` | (§45) Calcul formel : `match`/`case`, puis version objet |
| `td/td12-suite-conway/` | (§46) Suite de Conway avec des générateurs |

## Exécution

Prérequis : Python ≥ 3.10 (le cours utilise `match`/`case`), aucune dépendance.

```sh
python3 td/td01-types-fonctions/src/conversion.py
```

Les exercices se vérifient par des `assert` placés à la fin de chaque fichier : un script
qui se termine sans erreur passe ses tests. La CI exécute tous les fichiers `src/*.py`.
