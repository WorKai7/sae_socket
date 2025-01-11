import socket
import struct


BUFFER_SIZE = 256


def envoyer(sock, message):
    """
    Envoie un message via le socket passe en parametre
    """
    try:
        sock.sendall(message)
    except socket.error as e:
        print(f"Erreur d'envoi: {e}")
        sock.close()
        exit(-1)


def recevoir(sock, taille):
    """
    Reçoit un message de via le socket passe en parametre, dont la taille doit etre specifiee
    """
    try:
        data = sock.recv(taille)
        if not data:
            print("Le socket a été fermé par le serveur")
            sock.close()
            exit(0)
        return data
    except socket.error as e:
        print(f"Erreur de réception: {e}")
        sock.close()
        exit(-1)


def afficher_grille(grille):
    """
    Affiche la grille de jeu
    """
    print("\n+-----------+")
    for i in range(3):
        for j in range(3):
            print(f"| {grille[i][j]} ", end="") # On rajoute le end="" pour eviter le passage de ligne automatique
        print("|\n+-----------+")
    print()


def main():
    # On demande les informations du serveur
    ip = input("IP du serveur: ")
    port = int(input("Port du serveur: "))

    # On cree le socket
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((ip, port))
        print("Connexion au serveur réussie.")
    except socket.error as e:
        print(f"Erreur de connexion au serveur: {e}")
        exit(-1)

    # On recoit le symbole affecte au client
    symbol = recevoir(sock, 1).decode()
    print(f"Vous incarnez le joueur {symbol}.")

    # On recoit le message de debut
    message_debut = recevoir(sock, BUFFER_SIZE).decode()
    print(message_debut)

    # ---------- Boucle principale du jeu ----------

    while True:
        # On recoit l'information du tour (si c'est le notre ou non)
        tour = recevoir(sock, 1).decode()
        print(f"TOUR: {tour}")

        # On recoit la grille
        grille_data = recevoir(sock, 9)
        grille = [list(grille_data[i:i+3].decode()) for i in range(0, 9, 3)]
        afficher_grille(grille)

        # Si le tour actuel est le notre
        if tour == symbol:
            print("À vous de jouer !")
            saisie = None
            est_valide = 0

            # On reessaye tant que la saisie utilisateur n'est pas un coup valide
            while not est_valide:
                try:
                    saisie = int(input("Entrez une case vide (1-9): "))
                    if saisie < 1 or saisie > 9:
                        print("Erreur: Veuillez entrer un numéro entre 1 et 9.")
                        continue

                    # On envoie la saisie au serveur
                    envoyer(sock, struct.pack("i", saisie))

                    # On recoit la validation du serveur
                    est_valide = struct.unpack("i", recevoir(sock, 4))[0]
                    if not est_valide:
                        print("Erreur: case occupée ou invalide.")
                except ValueError:
                    print("Erreur: Veuillez entrer un entier valide.")

        else:
            print("En attente de l'adversaire...")

        # On recoit le message de continuation
        message = recevoir(sock, BUFFER_SIZE).decode()
        if message != "continue": # Fin de partie
            print("\n" + message)

            # On recoit la grille finale
            grille_data = recevoir(sock, 9)
            grille = [list(grille_data[i:i+3].decode()) for i in range(0, 9, 3)]
            print("\n\nGrille finale:\n")
            afficher_grille(grille)
            break

    # On ferme le socket
    sock.close()


if __name__ == "__main__":
    main()
