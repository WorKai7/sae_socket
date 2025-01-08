#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 256

void envoyer(int *socket, const void * message, size_t taille)
{
    switch (send(*socket, message, taille, 0))
    {
        case -1:
            perror("Erreur d'écriture\n");
            exit(-5);

        case 0:
            fprintf(stderr, "Le socket a été fermé par le client\n");
            close(*socket);
            exit(0);
    }
}


void recevoir(int *socket, void * message, size_t taille)
{
    switch (recv(*socket, message, taille, 0))
    {
        case -1:
            perror("Erreur de réception\n");
            exit(-6);

        case 0:
            fprintf(stderr, "Le socket a été fermé par le client\n");
            close(*socket);
            exit(0);
    }
}


void afficher_grille(char grille[3][3])
{
    printf("\n+-----------+\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("| %c ", grille[i][j]);
        }
        printf("|\n");
    }
    printf("+-----------+\n\n");
}

int main(int argc, char *argv[])
{

    char ip[16];
    int port;
    if (argc > 1)
    {
        strncpy(ip, argv[1], 16);
        sscanf(argv[2], "%d", &port);
    }
    else
    {
        printf("USAGE : %s [ip] [port]\n", argv[0]);
        exit(-1);
    }

    // Création du socket
    int socketJoueur = socket(AF_INET, SOCK_STREAM, 0);
    if (socketJoueur < 0)
    {
        perror("Erreur dans la création du socket\n");
        exit(-2);
    }

    printf("Socket créé\n");
    
    // Configuration de la structure sockaddr_in
    struct sockaddr_in sockaddrDistant;

    // On initialise la structure avec des 0
    socklen_t longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0x00, longueurAdresse);

    // Remplissage des champs protocole port et adresse
    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_port = htons(port);
    inet_aton(ip, &sockaddrDistant.sin_addr);

    // Connexion avec le serveur
    if (connect(socketJoueur, (struct sockaddr *) &sockaddrDistant, longueurAdresse) == -1)
    {
        perror("Erreur de connexion avec le serveur\n");
        close(socketJoueur);
        exit(-3);
    }

    printf("Connexion au serveur réussie\n");

    // Réception du premier message start
    char reception[BUFFER_SIZE];
    recevoir(&socketJoueur, reception, BUFFER_SIZE);

    // Le message a bien été reçu: affichage de celui-ci
    printf("%s\n", reception);

    // On boucle tant que le message indique que la partie continue
    do
    {
        char grille[3][3];

        // Réception de la grille
        printf("À vous de jouer\n");
        recevoir(&socketJoueur, grille, sizeof(grille));

        // Variables pour la saisie utilisateur
        int saisie;
        int estValide;

        do
        {
            // La grille a bien été reçue: affichage de celle-ci
            afficher_grille(grille);

            // Saisie utilisateur
            printf("Entrez une case parmi les cases vides (suivant le pattern ci-dessous):\n|1 2 3|\n|4 5 6|\n|7 8 9|\n-> ");
            scanf("%d", &saisie);

            // Envoi de la saisie
            envoyer(&socketJoueur, &saisie, sizeof(saisie));
            recevoir(&socketJoueur, &estValide, sizeof(estValide));

            if (!estValide)
            {
                printf("\nErreur de saisie, la case est déjà occupée !\n");
            }

        } while (!estValide);

        // Réception de la grille intermediaire
        recevoir(&socketJoueur, grille, sizeof(grille));

        // Affichage de la grille intermediaire
        printf("Au tour de votre adversaire\n");
        afficher_grille(grille);

        // Réception du message pour continuer
        memset(reception, 0x00, BUFFER_SIZE);
        recevoir(&socketJoueur, reception, BUFFER_SIZE);

    } while (strcmp(reception, "continue") == 0);

    // Affichage du message de fin
    recevoir(&socketJoueur, reception, BUFFER_SIZE);
    printf("%s\n", reception);

    // Fermeture du socket
    close(socketJoueur);

    return 0;
}