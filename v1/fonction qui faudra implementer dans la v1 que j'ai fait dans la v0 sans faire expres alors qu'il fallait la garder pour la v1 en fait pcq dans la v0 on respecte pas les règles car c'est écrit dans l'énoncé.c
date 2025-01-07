// Fonction renvoyant 0 si personne n'a gagné, 1 si le joueur X a gagné et 2 si le joueur O à gagné
int estGagnante(char grille[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        // On vérifie si il y a une ligne gagnante
        if (grille[i][0] == grille[i][1] == grille[i][2])
        {
            return grille[i][0] == 'X' ? 1 : 2;
        }

        // On vérifie si il y a une colonne gagnante
        if (grille[0][i] == grille[1][i] == grille[2][i])
        {
            return grille[i][0] == 'X' ? 1 : 2;
        }
    }

    // On vérifie les 2 diagonales à la main
    if (grille[0][0] == grille[1][1] == grille[2][2] || grille[0][2] == grille[1][1] == grille[2][0])
    {
        return grille[1][1] == 'X' ? 1 : 2;
    }

    return 0;
}