#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 256

void envoyer(int *socket, const void *message, size_t taille)
{
    switch (send(*socket, message, taille, 0))
    {
    case -1:
        perror("Erreur d'\u00e9criture\n");
        exit(-5);

    case 0:
        fprintf(stderr, "Le socket a \u00e9t\u00e9 ferm\u00e9 par le client\n");
        close(*socket);
        exit(0);
    }
}

void recevoir(int *socket, void *message, size_t taille)
{
    switch (recv(*socket, message, taille, 0))
    {
    case -1:
        perror("Erreur de r\u00e9ception\n");
        exit(-6);

    case 0:
        fprintf(stderr, "Le socket a \u00e9t\u00e9 ferm\u00e9 par le serveur\n");
        close(*socket);
        exit(0);
    }
}

void afficher_grille(char grille[3][3])
{
    printf("\n+-----------+\n");
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
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
    if (argc > 2)
    {
        strncpy(ip, argv[1], 16);
        sscanf(argv[2], "%d", &port);
    }
    else
    {
        printf("USAGE : %s [ip] [port]\nIl manque un argument\n", argv[0]);
        exit(-1);
    }

    // Création du socket
    int socketJoueur = socket(AF_INET, SOCK_STREAM, 0);
    if (socketJoueur < 0)
    {
        perror("Erreur dans la cr\u00e9ation du socket\n");
        exit(-2);
    }

    printf("Socket cr\u00e9\u00e9\n");

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
    if (connect(socketJoueur, (struct sockaddr *)&sockaddrDistant, longueurAdresse) == -1)
    {
        perror("Erreur de connexion avec le serveur\n");
        close(socketJoueur);
        exit(-3);
    }

    printf("Connexion au serveur r\u00e9ussie\n");

    // Variable pour indiquer à qui est le tour
    char tourActuel;

    // Initialisation de la grille de jeu
    char grille[3][3];

    // Initialisation d'une variable pour receptionner les messages du serveur
    char reception[BUFFER_SIZE];

    // Vérification du role du client (X, O ou S (comme spectateur))
    char role;
    recevoir(&socketJoueur, &role, sizeof(role));
    if (role == 'S')
    {
        // ----- Programme Spectateur -----

        printf("\nVous êtes spéctateur\n\n");

        do
        {
            printf("En attente du prochain tour..");
            recevoir(&socketJoueur, &tourActuel, sizeof(tourActuel));
            printf("TOUR: %c", tourActuel);

            recevoir(&socketJoueur, grille, sizeof(grille));
            afficher_grille(grille);

            recevoir(&socketJoueur, reception, BUFFER_SIZE);
        } while (strcmp(reception, "continue") == 0);
    }

    
    // ----- Programme joueur -----


    // Affichage du premier message indiquant le symbole incarné par le joueur
    printf("Vous incarnez le joueur %c\n", role);
    
    // Message start
    memset(reception, 0, BUFFER_SIZE);
    recevoir(&socketJoueur, reception, BUFFER_SIZE);

    // Le message a bien été recu: affichage de celui-ci
    printf("%s\n", reception);


    // On boucle tant que le message indique que la partie continue
    do
    {
        // Réception de l'état du tour
        recevoir(&socketJoueur, &tourActuel, sizeof(tourActuel));
        printf("TOUR: %c", tourActuel);

        // Réception de la grille
        recevoir(&socketJoueur, grille, sizeof(grille));
        afficher_grille(grille);

        // Jouer le tour si c'est au tour du joueur
        if (tourActuel == role)
        {
            printf("À vous de jouer !\n");

            // Saisie client
            int saisie, estValide;
            do
            {
                printf("Entrez une case vide (1-9) : ");
                scanf("%d", &saisie);

                // Envoi de la saisie au serveur pour vérification
                envoyer(&socketJoueur, &saisie, sizeof(saisie));

                // Récéption de la vérification
                recevoir(&socketJoueur, &estValide, sizeof(estValide));

                // Message d'erreur et boucle si invalide
                if (!estValide)
                {
                    printf("Erreur : case occupée ou invalide.\n");
                }
            } while (!estValide);
        }
        
        // Sinon, attendre le message continue ou end ou win
        else
        {
            printf("En attente de l'adversaire...\n");
        }

        // Attente
        memset(reception, 0, BUFFER_SIZE);
        recevoir(&socketJoueur, reception, BUFFER_SIZE);

    } while (strcmp(reception, "continue") == 0);

    // Affichage du message de fin
    memset(reception, 0, BUFFER_SIZE);
    recevoir(&socketJoueur, reception, BUFFER_SIZE);
    printf("\n%s\n", reception);

    // Récupération de la grille finale
    recevoir(&socketJoueur, grille, sizeof(grille));
    printf("\n\nGrille finale:\n\n");
    afficher_grille(grille);

    // Fermeture du socket
    close(socketJoueur);

    return 0;
}
