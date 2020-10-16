# INF_5101A

Rendu de TP de Jules Neghnagh--Chenavas et Alexandre Durmeyer

---

## MPI
### ⛔️ Limitations ⛔️
N'utilisez JAMAIS laplace avec une seule node, on va pas coder un cas pour ca sachant que dans 😠 TOUS LES AUTRES CAS ON DOIT JOUER AVEC LA MEMOIRE POUR LES LIGNES DE RECOUVREMENT😠 .

### Build 🏗️:
- Faites immediatement `make utils` pour avoir les petits scripts pour convertir les donnees

### Parallel IO
- Nous avons testes les parallel IO, donc nous lisons/ecrivons les matrices sous forme de fichier binaire (.dat) parsque c'est la seule methode possible.

- Pour transformer ces .dat en fichier lisible, utilisez les scripts de conversions de matrice ci-dessous, ou utilisez le fabuleux executable pour lire un bloc de la matrice

#### Conversions matrices:
    - text->.dat : `mpirun -np [NPROCS] ./convert [size_line] [filename]`
    - .dat->text : `./convert_dat_to_char.sh [size_line] [filename]`

#### Lecture d'un bloc:
    - `mpirun -np [NPROCS] ./read [size_line] [filename] [rank]`
    ou rank<NPROCS


---

# VOILA BISOUS 😘😘😘