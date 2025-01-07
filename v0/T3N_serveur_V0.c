#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 256

int estPleine(char grille[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (grille[i][j] == ' ')
            {
                return 0;
            }
        }
    }

    return 1;
}

int main(int argc, char *argv[])
{
    // Création du socket
    int socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0)
    {
        perror("Erreur dans la création de socket");
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
    if (bind(socketEcoute, (struct sockaddr *) &pointDeRencontreLocal, longueurAdresse) < 0)
    {
        perror("Erreur dans l'attachement");
        exit(-2);
    }

    printf("Socket attaché\n");

    // Écoute
    if (listen(socketEcoute, 1) < 0)
    {
        perror("Erreur d'écoute");
        exit(-3);
    }

    printf("Socket en écoute\n");

    while (1) {
        // Création d'une variable pour la saisie utilisateur
        char saisieClient[BUFFER_SIZE];
        memset(saisieClient, 'a', BUFFER_SIZE * sizeof(char));
        printf("Attente d'une connexion (Quitter avec Ctrl-C)\n\n");

        // Création de la strcuture sockaddr_in pour identifier la machine du client
        // (celle ci contiendra donc l'adresse ip et le port du client)
        // (elle est automatiquement remplie par la fonction accept à la connexion du client)
        // (c'est pour ca qu'on passe l'adresse des variables)
        struct sockaddr_in pointDeRencontreDistant;
        int socketDialogue = accept(socketEcoute, (struct sockaddr *) &pointDeRencontreDistant, &longueurAdresse);
        if (socketDialogue < 0)
        {
            perror("Erreur de connexion du client");
            close(socketDialogue);
            close(socketEcoute);
            exit(-4);
        }

        printf("Client connecté");

        // Lancement d'une partie

        // Création de la grille vide
        char grille[3][3] = {
            {' ', ' ', ' '},
            {' ', ' ', ' '},
            {' ', ' ', ' '}
        };

        // Envoi au client de la grille et du message start
        char startMessage[] = "La partie commence\n";
        if (send(socketDialogue, startMessage, strlen(startMessage), 0) < 0)
        {
            perror("Erreur de l'envoi du message start");
            exit(-5);
        }

        // Tant que la grille n'est pas pleine et que personne n'a gagné (à rajouter)
        while (!estPleine(grille))
        {
            // On envoie la grille au client
            // On recoit son info
            // On met a jour
            // On check si la game est finie
            // Si oui on close les trucs
            // Si non on joue une case random et ca boucle
            printf("TODO");
        }
    }
}