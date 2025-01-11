import socket
import struct

BUFFER_SIZE = 256

def envoyer(sock, message):
    """
    Envoie un message via le socket passé en paramètre
    """
    try:
        sock.sendall(message)
    except socket.error as e:
        print(f"Erreur d'envoi: {e}")
        sock.close()
        exit(-1)

def recevoir(sock, taille):
    """
    Reçoit un message via le socket passé en paramètre, dont la taille doit être spécifiée
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
            print(f"| {grille[i][j]} ", end="")
        print("|\n+-----------+")
    print()

def main():
    ip = input("IP du serveur: ")
    port = int(input("Port du serveur: "))

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((ip, port))
        print("Connexion au serveur réussie.")
    except socket.error as e:
        print(f"Erreur de connexion au serveur: {e}")
        exit(-1)

    role = recevoir(sock, 1).decode()
    if role == 'S':
        print("\nVous êtes spectateur\n")
        while True:
            tour = recevoir(sock, 1).decode()
            print(f"TOUR: {tour}")
            grille_data = recevoir(sock, 9)
            grille = [list(grille_data[i:i+3].decode()) for i in range(0, 9, 3)]
            afficher_grille(grille)
            message = recevoir(sock, BUFFER_SIZE).decode()
            if message != "continue":
                print("\n" + message)
                grille_data = recevoir(sock, 9)
                grille = [list(grille_data[i:i+3].decode()) for i in range(0, 9, 3)]
                print("\n\nGrille finale:\n")
                afficher_grille(grille)
                break
    else:
        print(f"Vous incarnez le joueur {role}.")
        message_debut = recevoir(sock, BUFFER_SIZE).decode()
        print(message_debut)

        while True:
            tour = recevoir(sock, 1).decode()
            print(f"TOUR: {tour}")

            grille_data = recevoir(sock, 9)
            grille = [list(grille_data[i:i+3].decode()) for i in range(0, 9, 3)]
            afficher_grille(grille)

            if tour == role:
                print("À vous de jouer !")
                est_valide = False
                while not est_valide:
                    try:
                        saisie = int(input("Entrez une case vide (1-9): "))
                        if saisie < 1 or saisie > 9:
                            print("Erreur: Veuillez entrer un numéro entre 1 et 9.")
                            continue
                        envoyer(sock, struct.pack("i", saisie))
                        est_valide = struct.unpack("i", recevoir(sock, 4))[0]
                        if not est_valide:
                            print("Erreur: case occupée ou invalide.")
                    except ValueError:
                        print("Erreur: Veuillez entrer un entier valide.")
            else:
                print("En attente de l'adversaire...")

            message = recevoir(sock, BUFFER_SIZE).decode()
            if message != "continue":
                print("\n" + message)
                grille_data = recevoir(sock, 9)
                grille = [list(grille_data[i:i+3].decode()) for i in range(0, 9, 3)]
                print("\n\nGrille finale:\n")
                afficher_grille(grille)
                break

    sock.close()

if __name__ == "__main__":
    main()