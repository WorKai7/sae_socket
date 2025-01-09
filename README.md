# T3N - Morpion sockets

## Présentation
Ce projet est un jeu du morpion codé en C à l'aide de sockets.
Il contient donc pour chaque version:
- Un serveur (qui accepte des connexions et dirige le jeu)
- Un client (qui peut se connecter au serveur et jouer)

## Structure
Ce projet contient plusieurs versions:

#### Version 0 (v0)
- Cette version ne respecte aucune règle du jeu, elle
se contente de gérer les communications réseau entre le client et
le serveur pour remplir la grille.

- Elle est complètement terminée et testée

#### Version 1 (v1)
- Cette version met en place des vérifications et respecte
les règles du jeu, il est possible de jouer contre le serveur
et de gagner ou de perdre.

- Elle est complètement terminée et testée

#### Version 2 (v2)
- Cette version implique maintenant 2 clients au lieu d'un,
les deux clients s'affrontent et le serveur sert uniquement à arbitrer.

- Elle est fonctionnelle

#### Version 3 (v3)
- Cette version implique la connexion de spectateurs pour visionner
la partie en cours.

- Elle n'existe pas

#### Version 4 (v4)
- Cette version permet au serveur de faire joueur plusieurs parties
simultanément.

- Elle n'existe bien evidemment pas non plus

## Dépendances
Pour l'instant, aucune dépendance n'est nécessaire, seulement une machine pouvant exécuter du C, sous Linux de préference

## Compilation et exécution
Ouvrez deux terminal dans le dossier de la version que vous voulez executer,
Tapez les commandes suivantes en remplacant le X par la version correspondante:

#### Compilation
```
gcc T3N_serveur_VX.c -o serveur
gcc T3N_client_VX.c -o client
```

#### Exécution
Exécutez ces deux commandes chacune dans un terminal différent.
Veillez a bien exécuter le serveur AVANT d'exécuter le client
```
./serveur
./client
```

## Membres du projet
- Vandewalle Jérôme (responsable dépôt)
- Arabah Yanis
- Pruvost Scotty
