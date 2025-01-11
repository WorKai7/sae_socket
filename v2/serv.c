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


// Fonction permettant d'envoyer un message a un client
void envoyer(int *socket, const void *message, size_t taille) 
{
    if (send(*socket, message, taille, 0) <= 0) {
        perror("Erreur d'envoi");
        close(*socket);
        exit(EXIT_FAILURE);
    }
}


// Fonction permettant de recevoir un message d'un client
void recevoir(int *socket, void *message, size_t taille) 
{
    if (recv(*socket, message, taille, 0) <= 0) {
        perror("Erreur de réception");
        close(*socket);
        exit(EXIT_FAILURE);
    }
}


// Fonction renvoyant 1 si la grille est pleine et 0 sinon
int estPleine(char grille[3][3]) 
{
    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++) 
        {
            if (grille[i][j] == ' ') return 0;
        }
    }
    return 1;
}


// Fonction renvoyant 0 si personne n'a gagné, 1 si le joueur X a gagné et 2 si le joueur O a gagné
int estGagnante(char grille[3][3]) 
{
    for (int i = 0; i < 3; i++) {
        // Vérifie la ligne
        if (grille[i][0] == grille[i][1] && grille[i][1] == grille[i][2] && grille[i][0] != ' ')
            return grille[i][0] == 'X' ? 1 : 2;

        // Vérifie la colonne
        if (grille[0][i] == grille[1][i] && grille[1][i] == grille[2][i] && grille[0][i] != ' ')
            return grille[0][i] == 'X' ? 1 : 2;
    }

    // Vérifie les diagonales
    if (grille[0][0] == grille[1][1] && grille[1][1] == grille[2][2] && grille[0][0] != ' ')
        return grille[0][0] == 'X' ? 1 : 2;

    if (grille[0][2] == grille[1][1] && grille[1][1] == grille[2][0] && grille[0][2] != ' ')
        return grille[0][2] == 'X' ? 1 : 2;

    return 0;
}


int main(int argc, char *argv[]) 
{
    // Création du socket
    int socketServeur = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServeur < 0) 
    {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    printf("Socket créé\n");


    // Mise en place de la structure sockaddr_in et configuration de celle-ci
    // (Elle contient les informations telles que l'adresse ip, le port, le protocole utilisé etc..)
    // (Et elle sert à identifier une machine (ici le serveur car c le point de rencontre local))
    struct sockaddr_in serveurAddr = {0};
    serveurAddr.sin_family = AF_INET;
    serveurAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveurAddr.sin_port = htons(PORT);


    // Attachement du socket à l'adresse et au port spécifiés
    if (bind(socketServeur, (struct sockaddr*)&serveurAddr, sizeof(serveurAddr)) < 0) 
    {
        perror("Erreur de liaison du socket");
        close(socketServeur);
        exit(EXIT_FAILURE);
    }

    // Mise en écoute des connexions
    if (listen(socketServeur, 2) < 0) 
    {
        perror("Erreur d'écoute");
        close(socketServeur);
        exit(EXIT_FAILURE);
    }

    printf("En attente de connexions...\n");

    struct sockaddr_in clientAddr1, clientAddr2;
    socklen_t addrLen = sizeof(clientAddr1);

    // Acceptation les connexions client
    int socketClient1 = accept(socketServeur, (struct sockaddr*)&clientAddr1, &addrLen);
    if (socketClient1 < 0) {
        perror("Erreur de connexion du client 1");
        exit(EXIT_FAILURE);
    }
    printf("Client 1 connecté\n");

    int socketClient2 = accept(socketServeur, (struct sockaddr*)&clientAddr2, &addrLen);
    if (socketClient2 < 0) {
        perror("Erreur de connexion du client 2");
        exit(EXIT_FAILURE);
    }
    printf("Client 2 connecté\n");
    fflush(stdout);
    printf("suite");


    // Initialisation de la grille
    printf("LOG: initialisation de la grille\n");
    char grille[3][3] = {
        {' ', ' ', ' '},
        {' ', ' ', ' '},
        {' ', ' ', ' '}
    };
    printf("SUCCES: initialisation de la grille\n");

    // Envoi du message de debut de la partie
    char startMsg[] = "La partie commence. Vous êtes respectivement X et O.\n";
    printf("LOG: Envoi de la grille 1\n");
    envoyer(&socketClient1, startMsg, strlen(startMsg) + 1);
    printf("SUCCES: Grille 1 envoyee\n");

    printf("LOG: Envoi de la grille 2\n");
    envoyer(&socketClient2, startMsg, strlen(startMsg) + 1);
    printf("SUCCES: Grille 2 envoyee\n");


    int joueurActuel = 1; // 1 pour le joueur X, 2 pour le joueur O


    // ----- Boucle du serveur -----


    while (!estGagnante(grille) && !estPleine(grille))
    {
        int tour1 = (joueurActuel == 1) ? 1 : 0;
        int tour2 = (joueurActuel == 2) ? 1 : 0;
        int tour1Network = htonl(tour1);
        int tour2Network = htonl(tour2);
        envoyer(&socketClient1, &tour1Network, sizeof(tour1Network));
        envoyer(&socketClient2, &tour2Network, sizeof(tour2Network));

        envoyer(&socketClient1, grille, sizeof(grille));
        envoyer(&socketClient2, grille, sizeof(grille));

        int socketActuel = (joueurActuel == 1) ? socketClient1 : socketClient2;
        int socketAdverse = (joueurActuel == 1) ? socketClient2 : socketClient1;

        // Envoi du tour actuel
        int joueurActuelNetwork = htonl(joueurActuel);
        envoyer(&socketActuel, &joueurActuelNetwork, sizeof(joueurActuelNetwork));

        // Envoi de la grille au joueur actuel
        envoyer(&socketActuel, grille, sizeof(grille));

        // Réception du coup
        int choixNetwork = 0;
        recevoir(&socketActuel, &choixNetwork, sizeof(choixNetwork));
        int choix = ntohl(choixNetwork) - 1;

        int valide;
        if (choix >= 0 && choix < 9 && grille[choix / 3][choix % 3] == ' ') {
            grille[choix / 3][choix % 3] = (joueurActuel == 1) ? 'X' : 'O';
            valide = 1;
        } else {
            valide = 0;
        }

        // Envoi de la validation
        int valideNetwork = htonl(valide);
        envoyer(&socketActuel, &valideNetwork, sizeof(valideNetwork));

        // Vérification et mise à jour
        if (estGagnante(grille) || estPleine(grille)) break;

        // Envoi de la grille à l'adversaire
        envoyer(&socketAdverse, grille, sizeof(grille));
        joueurActuel = 3 - joueurActuel;
    }

    // Envoi de la grille finale avec un message de fin
    envoyer(&socketClient1, grille, sizeof(grille));
    envoyer(&socketClient2, grille, sizeof(grille));

    char finMsg[BUFFER_SIZE];
    if (estGagnante(grille) != 0) 
    {
        sprintf(finMsg, "Le joueur %c a gagné !\n", (estGagnante(grille) == 1) ? 'X' : 'O');
    } else {
        sprintf(finMsg, "Match nul !\n");
    }
    envoyer(&socketClient1, finMsg, strlen(finMsg));
    envoyer(&socketClient2, finMsg, strlen(finMsg));

    // Fermeture des sockets
    close(socketClient1);
    close(socketClient2);
    close(socketServeur);

    return 0;
}
