#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5000
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

int main(int argc, char *argv[])
{
    // Création du socket
    int socketServeur = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServeur < 0)
    {
        perror("Erreur dans la création de socket\n");
        exit(-1);
    }

    printf("Socket créé\n");


    // Mise en place de la structure sockaddr_in et configuration de celle-ci
    // (Elle contient les informations telles que l'adresse ip, le port, le protocole utilisé etc..)
    // (Et elle sert à identifier une machine (ici le serveur car c le point de rencontre local))
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse = sizeof(pointDeRencontreLocal);

    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);


    // Attachement du socket à l'interface
    if (bind(socketServeur, (struct sockaddr *) &pointDeRencontreLocal, longueurAdresse) < 0)
    {
        perror("Erreur dans l'attachement\n");
        exit(-2);
    }

    printf("Socket attaché\n");

    // Écoute
    if (listen(socketServeur, 1) < 0)
    {
        perror("Erreur d'écoute\n");
        exit(-3);
    }

    printf("Socket en écoute\n");

    srand(time(NULL));




    // ----- Boucle du serveur -----

    while (1) {
        // Message d'attente
        printf("Attente d'une connexion (Quitter avec Ctrl-C)\n\n");

        // Création de la strcuture sockaddr_in pour identifier la machine du client
        // (celle ci contiendra donc l'adresse ip et le port du client)
        // (elle est automatiquement remplie par la fonction accept à la connexion du client)
        // (c'est pour ca qu'on passe l'adresse des variables)
        struct sockaddr_in pointDeRencontreDistant;
        int socketJoueur = accept(socketServeur, (struct sockaddr *) &pointDeRencontreDistant, &longueurAdresse);
        if (socketJoueur < 0)
        {
            perror("Erreur de connexion du client\n");
            close(socketJoueur);
            close(socketServeur);
            exit(-4);
        }

        printf("Client connecté\n");

        // ----- Lancement d'une partie -----

        // Création de la grille vide
        char grille[3][3] = {
            {' ', ' ', ' '},
            {' ', ' ', ' '},
            {' ', ' ', ' '}
        };

        // Envoi au client du message start
        char startMessage[] = "La partie commence\n";
        envoyer(&socketJoueur, startMessage, strlen(startMessage));

        // Boucle du jeu
        while (1)
        {
            // Envoi au client de la grille
            usleep(5000); // Sécurité pour s'assurer que le serveur n'envoie pas la grille avant que le client l'attende
            envoyer(&socketJoueur, grille, sizeof(grille));

            /*
                Création d'une variable pour la saisie utilisateur
                (Nombre entre 1 et 9 pour la case choisie selon le pattern ci-dessous)
                +-----------+
                | 1 | 2 | 3 |
                | 4 | 5 | 6 |
                | 7 | 8 | 9 |
                +-----------+
            */
            int saisieClient;

            // Réception de la saisie du joueur tant qu'elle n'est pas valide
            recevoir(&socketJoueur, &saisieClient, sizeof(int));

            // Décrémentation de la case choisie pour un traitement plus simple
            saisieClient--;

            // Mise à jour de la grille
            grille[saisieClient / 3][saisieClient % 3] = 'X';

            // Envoi de la grille intermediaire
            envoyer(&socketJoueur, grille, sizeof(grille));

            // On joue une case aléatoire
            int caseRandom = rand() % 9;

            grille[caseRandom / 3][caseRandom % 3] = 'O';
        }

        close(socketJoueur);
    }

    close(socketServeur);
    return 0;
}