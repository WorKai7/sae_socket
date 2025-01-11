#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define PORT 5000
#define BUFFER_SIZE 256
#define MAX_SPECTATORS 10

// Variables globales

int socketsSpectateurs[MAX_SPECTATORS];
int socketServeur;
int nbSpectateurs = 0;


// Fonction permettant d'envoyer un message a un client
void envoyer(int *socket, const void *message, size_t taille) 
{
    usleep(5000); // Pour que le client ai le temps d'afficher le message de fin avant d'être fermé
    if (send(*socket, message, taille, 0) <= 0) {
        perror("Erreur d'envoi");
        close(*socket);
        exit(EXIT_FAILURE);
    }
}

void envoyerAuxSpectateurs(void *message, size_t taille)
{
    for (int i = 0; i < nbSpectateurs; i++)
    {
        envoyer(&socketsSpectateurs[i], message, taille);
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


int partieFinie(int *socket1, int *socket2, char grille[3][3], char joueurAyantJoue)
{
    int gagnante = estGagnante(grille);
    int pleine = estPleine(grille);

    if (gagnante || pleine)
    {
        // Personalisation du message si la partie est gagnée ou juste finie
        char msg[5];
        char messageFin[256];

        if (gagnante)
        {
            sprintf(msg, "%cwin", joueurAyantJoue);
            sprintf(messageFin, "Partie terminée, le joueur %c a gagné !", joueurAyantJoue);
        }
        else
        {
            sprintf(msg, "%cend", joueurAyantJoue);
            strcpy(messageFin, "Partie terminée, personne n'a gagné !");
        }

        // Envoi d'un message pour que le client sache que la partie est terminée
        envoyer(socket1, msg, strlen(msg));
        envoyer(socket2, msg, strlen(msg));
        envoyerAuxSpectateurs(msg, strlen(msg));

        // Affichage du message de fin du côté serveur
        printf("%s\n", messageFin);

        // Envoi du message de fin
        envoyer(socket1, messageFin, strlen(messageFin));
        envoyer(socket2, messageFin, strlen(messageFin));
        envoyerAuxSpectateurs(messageFin, strlen(messageFin));

        // Envoi de la grille finale
        envoyer(socket1, grille, 9);
        envoyer(socket2, grille, 9);
        envoyerAuxSpectateurs(grille, 9);

        close(*socket1);
        close(*socket2);
        nbSpectateurs = 100; // Pour arreter la boucle du thread
        printf("%d", nbSpectateurs);
        return 1;
    }

    return 0;
}


void *acceptSpectators()
{
    while (nbSpectateurs < MAX_SPECTATORS)
    {
        printf("Attente d'un spectateur, nb: %d\n", nbSpectateurs);
        struct sockaddr_in specAddr;
        socklen_t addrLen = sizeof(specAddr);

        int socketSpec = accept(socketServeur, (struct sockaddr*)&specAddr, &addrLen);

        if (socketSpec < 0) {
            perror("Erreur de connexion d'un spectateur, fermeture du thread");
            pthread_exit(NULL);
        }
        
        // Envoi du role à ce spectateur
        char role = 'S';
        envoyer(&socketSpec, &role, sizeof(role));

        // Ajout du nouveau socket pour le spectateur dans la liste
        socketsSpectateurs[nbSpectateurs] = socketSpec;
        nbSpectateurs++;

        printf("Spectateur n°%d connecté\n", nbSpectateurs);
    }

    pthread_exit(NULL);
}


int main(int argc, char *argv[]) 
{
    // Création du socket
    socketServeur = socket(AF_INET, SOCK_STREAM, 0);
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

    struct sockaddr_in clientAddr1, clientAddr2;
    socklen_t addrLen = sizeof(clientAddr1);

    while (1)
    {
        printf("En attente des deux joueurs...\n");

        // Acceptation les connexions client
        int socketClient1 = accept(socketServeur, (struct sockaddr*)&clientAddr1, &addrLen);
        if (socketClient1 < 0) {
            perror("Erreur de connexion du client 1");
            exit(EXIT_FAILURE);
        }
        printf("Client 1 connecté\n");

        char symbol1 = 'X';
        envoyer(&socketClient1, &symbol1, sizeof(char));

        int socketClient2 = accept(socketServeur, (struct sockaddr*)&clientAddr2, &addrLen);
        if (socketClient2 < 0) {
            perror("Erreur de connexion du client 2");
            exit(EXIT_FAILURE);
        }
        printf("Client 2 connecté\n");

        char symbol2 = 'O';
        envoyer(&socketClient2, &symbol2, sizeof(char));

        pthread_t thread;
        if (pthread_create(&thread, NULL, acceptSpectators, NULL) < 0)
        {
            perror("Erreur de création du thread, partie sans spectateurs.");
        }


        // Initialisation de la grille
        char grille[3][3] = {
            {' ', ' ', ' '},
            {' ', ' ', ' '},
            {' ', ' ', ' '}
        };

        char continueMessage[] = "continue";

        // Envoi du message de debut de la partie
        char startMsg[] = "La partie commence.\n";
        envoyer(&socketClient1, startMsg, strlen(startMsg));
        envoyer(&socketClient2, startMsg, strlen(startMsg));

        char tour = 'X';


        // ----- Boucle du serveur -----


        while (1)
        {
            // Envoi du tour aux clients
            envoyer(&socketClient1, &tour, sizeof(tour));
            envoyer(&socketClient2, &tour, sizeof(tour));

            // Envoi de la grille aux clients
            envoyer(&socketClient1, grille, sizeof(grille));
            envoyer(&socketClient2, grille, sizeof(grille));

            // On récupère le socket du joueur qui joue
            int socketActuel = (tour == symbol1) ? socketClient1 : socketClient2;

            int saisieClient;
            int estValide;

            // Réception de la saisie du joueur tant qu'elle n'est pas valide
            do
            {
                recevoir(&socketActuel, &saisieClient, sizeof(saisieClient));

                // Décrémentation de la case choisie pour un traitement plus simple
                saisieClient--;

                // Vérification que la case entrée est valide
                estValide = grille[saisieClient / 3][saisieClient % 3] == ' ';

                // Envoi de la confirmation au client
                envoyer(&socketActuel, &estValide, sizeof(estValide));
            } while (!estValide);

            // Mise à jour de la grille
            grille[saisieClient / 3][saisieClient % 3] = tour;

            // Sortie de boucle si la partie est terminée
            if (partieFinie(&socketClient1, &socketClient2, grille, tour)) break;

            // Changement de joueur
            tour = tour == 'X' ? 'O' : 'X';

            // Envoi du message pour continuer et boucle
            envoyer(&socketClient1, continueMessage, strlen(continueMessage));
            envoyer(&socketClient2, continueMessage, strlen(continueMessage));

            // Envoi du tour, grille et message aux spectateurs
            envoyerAuxSpectateurs(continueMessage, strlen(continueMessage));
            envoyerAuxSpectateurs(&tour, sizeof(tour));
            envoyerAuxSpectateurs(grille, sizeof(grille));
        }
        nbSpectateurs = 0;
        pthread_join(thread, NULL);
    }

    close(socketServeur);
    return 0;
}
