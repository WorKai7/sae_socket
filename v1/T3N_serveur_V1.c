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


// Fonction renvoyant 1 si la grille est pleine et 0 sinon
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


// Fonction renvoyant 0 si personne n'a gagné, 1 si le joueur X a gagné et 2 si le joueur O à gagné
int estGagnante(char grille[3][3])
{
    for (int i = 0; i < 3; i++)
    {
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


int partieFinie(int *socket, char grille[3][3], char joueurAyantJoue)
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
        envoyer(socket, msg, strlen(msg));

        // Affichage du message de fin du côté serveur
        printf("%s\n", messageFin);

        // Envoi du message de fin
        usleep(5000); // Pour que le client ai le temps d'afficher le message de fin avant d'être fermé
        envoyer(socket, messageFin, strlen(messageFin));

        // Envoi de la grille finale
        usleep(5000);
        envoyer(socket, grille, 9);

        close(*socket);
        return 1;
    }

    return 0;
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
        char continueMessage[] = "continue";
        envoyer(&socketJoueur, startMessage, strlen(startMessage));

        // Boucle du jeu
        while (1)
        {
            // Envoi au client de la grille
            usleep(5000); // Sécurité pour s'assurer que le serveur n'envoie pas la grille avant que le client l'attende
            envoyer(&socketJoueur, grille, sizeof(grille));

            // Envoi d'un message pour que le client sache que la partie continue
            envoyer(&socketJoueur, continueMessage, strlen(continueMessage));

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
            int estValide;

            // Réception de la saisie du joueur tant qu'elle n'est pas valide
            do
            {
                recevoir(&socketJoueur, &saisieClient, sizeof(saisieClient));

                // Décrémentation de la case choisie pour un traitement plus simple
                saisieClient--;

                // Vérification que la case entrée est valide
                estValide = grille[saisieClient / 3][saisieClient % 3] == ' ';

                // Envoi de la confirmation au client
                envoyer(&socketJoueur, &estValide, sizeof(estValide));

                // Si la confirmation n'est pas bonne, on envoie un message pour lui dire de continuer
                if (!estValide)
                {
                    envoyer(&socketJoueur, continueMessage, strlen(continueMessage));
                }
            } while (!estValide);

            // Mise à jour de la grille
            grille[saisieClient / 3][saisieClient % 3] = 'X';

            // Envoi de la grille intermediaire
            envoyer(&socketJoueur, grille, sizeof(grille));

            // Si la partie est terminée, la fonction gère tout et on sort de la boucle
            if (partieFinie(&socketJoueur, grille, 'X'))
            {
                break;
            }

            // La partie n'est pas terminée, on envoie le message pour continuer
            envoyer(&socketJoueur, continueMessage, strlen(continueMessage));

            // On joue une case aléatoire
            int caseRandom;

            do
            {
                caseRandom = rand() % 9;
            } while (grille[caseRandom / 3][caseRandom % 3] != ' ');

            // Mise à jour de la grille
            grille[caseRandom / 3][caseRandom % 3] = 'O';

            // Si la partie est terminée, la fonction gère tout et on sort de la boucle
            if (partieFinie(&socketJoueur, grille, 'O'))
            {
                break;
            }

            // La partie n'est pas terminée, on boucle
        }
    }

    close(socketServeur);
    return 0;
}