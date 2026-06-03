//----------------------80-char------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define TAILLE 12
#define MAX_CAISSE 20
#define MAX_DEP 1000

typedef char t_Plateau [ TAILLE ]  [ TAILLE ]  ;
typedef struct { int x ,  y ; }   Position ;
typedef Position t_Plateau_creux [MAX_CAISSE];
typedef char t_tabDeplacement [ MAX_DEP ] ;

const char WALL  =  '#' ;
const char BOX  =  '$' ;
const char TARGET  =  '.' ;
const char PLAYER  =  '@' ;
const char BOXTARGET  =  '*' ;
const char PLAYERTARGET  =  '+' ;
const char SPACE  =  ' ' ;
const int ZOOMAX  =  3 ;
const int ZOOMIN  =  1 ;


//controls  =  Arrow zqsd  +  wasd
#define LEFT_A 'D'
#define RIGHT_A 'C'
#define UP_A 'A'
#define DOWN_A 'B'
#define LEFT_Q 'q'
#define RIGHT_D 'd'
#define UP_Z 'z'
#define DOWN_S 's'
#define LEFT 'a'
#define UP_W 'w'
#define EXIT 'x'
#define RESET 'r'
#define UNDO 'u'
#define ZOOM '+'
#define UNZOOM '-'
#define UNDOG 'g'
#define UNDOD 'd'
#define UNDOH 'h'
#define UNDOB 'b'
#define UPCAISSE 'H'
#define DOWNCAISSE 'B'
#define LEFTCAISSE 'G'
#define RIGHTCAISSE 'D'

// obligatoires 
void charger_partie (t_Plateau plateau ,  char fichier [  ] )  ;
void enregistrer_partie (t_Plateau plateau ,  char fichier [  ] )  ;
void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]);
void chargerDeplacements(t_tabDeplacement t, char fichier[], int * nb);
int kbhit ()  ;

//mes fonctions/procédure : 
//affichage
void affichage_jeu (int mouvement ,  char nomFichier []  ,  t_Plateau plateau)  ;
void affichage_victoire (int move)  ;

//jeu
Position trouve_joueur (t_Plateau plateau)  ;
bool deplacer (t_Plateau plateau ,  Position posFrom ,  Position posTo , 
    Position further ,  bool  * caisse)  ;
Position controle (t_Plateau plateau , char input ,  Position pos ,
      int  * aMove,  t_tabDeplacement listeMove )  ;
bool gagne (t_Plateau plateau)  ;
char dep2touche(char touche);



bool platCmp(t_Plateau_creux plato, t_Plateau_creux plato2, int index){
    int i = 0;
    bool identique = true;
    while(i < index && identique){
        if (!(plato[i].x  == plato2[i].x && plato[i].y == plato2[i].y)){
            identique = false;
        }
        i++;
    }
    return identique;
}



void savePlateau( t_Plateau plat , t_Plateau_creux * tabPos , int * move, Position pos ){
    t_Plateau_creux plato;
    plato[0] = pos;
    int index = 1;
    for (int i  =  0 ; i < TAILLE ; i ++ )  { 
        for (int j  =  0 ; j < TAILLE ; j ++ )  {             
            if (plat [ i ] [ j ] == BOXTARGET || plat [ i ] [ j ] == BOX)  { 
                plato [ index ]  = ( Position ) { j , i } ;       
                index++;     
            }  
        }  
    }

    int indice = 0;
    bool cancel = false;
    while(indice < *move && !cancel){
        if (platCmp(plato, tabPos[indice] ,index)){
            *move = indice;
            cancel = true;
        }
        indice++;
    }
        for (int i = 0; i < index; i++) {
        tabPos[*move][i] = plato[i]; // Copy each Position
        }
    }

    





int main ()  { 
    //variables
    char nmFic [ 50 ]  ;
    char nmFicDep [ 50 ]  ;
    t_Plateau plateau ;
    t_Plateau temPlateau;
    Position pos ;
    char touche  =  '\0' ;
    int move  =  0 ;
    int moveprevu;
    t_tabDeplacement listeMove ;
    t_tabDeplacement listeMovePrevu;
    t_Plateau_creux listePosition[MAX_DEP];


    //choix du niveau
    printf ("nom fichier .sok?")  ;
    scanf ("%s" ,  nmFic)  ;
    //chargement des variables de jeu
    charger_partie (plateau , nmFic)  ;
    pos  =  trouve_joueur (plateau)  ;
    printf ("nom fichier .dep?")  ;
    scanf ("%s" ,  nmFicDep);
    chargerDeplacements(listeMovePrevu, nmFicDep, &moveprevu);
    //1er affichage
    savePlateau(plateau, listePosition, &move, pos);
    affichage_jeu (move , nmFic , plateau) ;
    for(int i = 0; i < moveprevu; i++){
        touche = dep2touche(listeMovePrevu[i]);
        Position temPos = pos;
        for (int i = 0; i < TAILLE; i++) {
            for (int j = 0; j < TAILLE; j++) {
                temPlateau[i][j] = plateau[i][j]; //copie
            }
        }
        pos  =  controle (plateau ,  touche ,  pos ,  &move, listeMove);
        if(listeMovePrevu[i] != listeMove[move-1]){ //le mouvement n'est pas correcte

            for (int i = 0; i < TAILLE; i++) {
                for (int j = 0; j < TAILLE; j++) {
                    plateau[i][j] = temPlateau[i][j]; // Copie cellule par cellule
                }
            }
            pos = temPos;
        }
        savePlateau(plateau, listePosition, &move, pos);
        affichage_jeu (move , nmFic , plateau);
        usleep(250000);
    }
    if (gagne(plateau)){
        printf("La suite de déplacements %s ",nmFicDep);
        printf("est bien une solution pour la partie %s.\n",nmFic);
        printf("Elle contient %d deplacement\n",move);
    } 
    else{
        printf("La suite de déplacements %s ",nmFicDep);
        printf("n'est pas une solution pour la partie %s.\n",nmFic);
    }
    return 0 ;
}  

//obligatoire
void charger_partie (t_Plateau plateau ,  char fichier [  ] )  { 
    FILE  *  f ;
    char finDeLigne ;

    f  =  fopen (fichier ,  "r")  ;
    if  (f ==  NULL)  { 
        printf ("ERREUR SUR FICHIER")  ;
        exit (EXIT_FAILURE)  ;
     }   else  { 
        for  (int ligne = 0  ; ligne<TAILLE  ; ligne ++ )  { 
            for  (int colonne = 0  ; colonne<TAILLE  ; colonne ++ )  { 
                fread (&plateau [ ligne ]  [ colonne ]  ,  sizeof (char)  ,  1
                 ,  f)  ;
             }  
            fread (&finDeLigne ,  sizeof (char)  ,  1 ,  f)  ;
         }  
        fclose (f)  ;
     }  
 }  

    

void enregistrer_partie (t_Plateau plateau ,  char fichier [  ] )  { 
    FILE  *  f ;
    char finDeLigne = '\n' ;

    f  =  fopen (fichier ,  "w")  ;
    for  (int ligne = 0  ; ligne<TAILLE  ; ligne ++ )  { 
        for  (int colonne = 0  ; colonne<TAILLE  ; colonne ++ )  { 
            fwrite (&plateau [ ligne ]  [ colonne ]  ,  sizeof (char)  ,  1 ,  
            f)  ;
         }  
        fwrite (&finDeLigne ,  sizeof (char)  ,  1 ,  f)  ;
     }  
    fclose (f)  ;
 }  

int kbhit ()  { 
    // la fonction retourne  : 
    // 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere = 0 ;
	struct termios oldt ,  newt ;
	int ch ;
	int oldf ;

	// mettre le terminal en mode non bloquant
	tcgetattr (STDIN_FILENO ,  &oldt)  ;
	newt  =  oldt ;
	newt . c_lflag &=  ~ (ICANON | ECHO)  ;
	tcsetattr (STDIN_FILENO ,  TCSANOW ,  &newt)  ;
	oldf  =  fcntl (STDIN_FILENO ,  F_GETFL ,  0)  ;
	fcntl (STDIN_FILENO ,  F_SETFL ,  oldf | O_NONBLOCK)  ;
 
	ch  =  getchar ()  ;

	// restaurer le mode du terminal
	tcsetattr (STDIN_FILENO ,  TCSANOW ,  &oldt)  ;
	fcntl (STDIN_FILENO ,  F_SETFL ,  oldf)  ;
 
	if (ch !=  EOF)  { 
		ungetc (ch ,  stdin)  ;
		unCaractere = 1 ;
	 }   
	return unCaractere ;
 }  

void enregistrerDeplacements (t_tabDeplacement t ,  int nb ,  char fic [  ] ) 
{ 
    FILE  *  f ;
    f  =  fopen (fic ,  "w")  ;
   fwrite (t , sizeof (char)  ,  nb ,  f)  ;
   fclose (f)  ;
 }  

void chargerDeplacements(t_tabDeplacement t, char fichier[], int * nb){
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
    } else {
        fread(&dep, sizeof(char), 1, f);
        if (feof(f)){
            printf("FICHIER VIDE\n");
        } else {
            while (!feof(f)){
                t[*nb] = dep;
                (*nb)++;
                fread(&dep, sizeof(char), 1, f);
            }
        }
    }
    fclose(f);
}










//mes fonctions/procédure
//affichage

void afficher_entete (int movements ,  char nmFic [  ])  {
    printf ("\nSOKOBAN created by moi ,  i'm proud of it  : D")  ;
    printf ("\nGame :  %s" , nmFic) ;
    printf ("\n-------------------------------------------------------")  ;
    printf ("-----------------------")  ;
    printf ("\n moved  :  %d times" ,  movements   )  ;
    printf ("\n\n")  ;
}  


void afficher_plateau (t_Plateau plateau )  { 
    for (int i  =  0 ; i < TAILLE ; i ++ )  { 
        for (int j  =  0 ; j < TAILLE ; j ++ )  { 
            char tmp  =  plateau [ i ]  [ j ]  ;
            if  (tmp  ==   BOXTARGET)  { 
                tmp  =  BOX ;
                }  else if (tmp  ==   PLAYERTARGET)  { 
                tmp  =  PLAYER ;
                }  

            printf ("%c" , tmp)  ;
            
        }  
        printf ("\n")  ;
    
     }  
 }  


void affichage_jeu (int mouvement , char nomFichier [] , t_Plateau plateau) { 
    system ("clear")  ;
    afficher_entete (mouvement , nomFichier) ;
    afficher_plateau (plateau) ;
}  

void affichage_victoire (int move)   { 
    printf ("félicitation tu as gagné en %d coups.\n" , move) ;
    //on peut faire de l'ascii art mais flemme  : D 
}  


//jeu

Position trouve_joueur (t_Plateau plateau)  {   
    for (int i  =  0 ; i < TAILLE ; i ++ )  { 
        for (int j  =  0 ; j < TAILLE ; j ++ )  {             
            if (plateau [ i ]  [ j ]   ==   PLAYERTARGET || plateau [ i ]
                  [ j ]   ==   PLAYER)  { 
                return  (Position)  { j ,  i }   ;            
             }  
         }  
     }  
    printf ("ERREUR pas de joueur dans le fichier")  ;
    return  (Position)  { - 1 ,  - 1 }   ;
 }  


bool deplacer (t_Plateau plateau ,  Position posFrom ,  Position posTo , 
    Position further ,  bool  * caisse)  { 
     * caisse  =  false ;
    bool cancel  =  false ;
    if (plateau [ posTo . y ]  [ posTo . x ]   ==   SPACE)  { 
        plateau [ posTo . y ]  [ posTo . x ]   =  PLAYER ;
     }  else
    if (plateau [ posTo . y ]  [ posTo . x ]   ==   TARGET)  { 
        plateau [ posTo . y ]  [ posTo . x ]   =  PLAYERTARGET ;
     }  else 
    if (plateau [ posTo . y ]  [ posTo . x ]   ==   WALL)  { 
        cancel  =  true ;
     }  else {  //soit un joueur ,  soit une boite ,  joueur  impossible 
        if (plateau [ further . y ]  [ further . x ]   ==   SPACE)  { 
            plateau [ further . y ]  [ further . x ]   =  BOX ;
         }  else
        if (plateau [ further . y ]  [ further . x ]   ==   TARGET)  { 
            plateau [ further . y ]  [ further . x ]   =  BOXTARGET ;
         }  else {  //joueur ,  boite ,  mur :  mouvement impossible
            cancel  =  true ;
         }  

        if  (!cancel)  {  //la boite peux etre poussé
             * caisse  =  true ;
            if (plateau [ posTo . y ]  [ posTo . x ]   ==   BOX)  { 
                plateau [ posTo . y ]  [ posTo . x ]   =  PLAYER ;
             }  else
            if (plateau [ posTo . y ]  [ posTo . x ]   ==   BOXTARGET)  { 
                plateau [ posTo . y ]  [ posTo . x ]   =  PLAYERTARGET ;
             }  
         }  
        
     }  

    if  (!cancel)  {  // le joueur ne peux pas avancer
        if (plateau [ posFrom . y ]  [ posFrom . x ]   ==   PLAYER)  { 
            plateau [ posFrom . y ]  [ posFrom . x ]   =  SPACE ;
         }  else
        if (plateau [ posFrom . y ]  [ posFrom . x ]   ==   PLAYERTARGET) { 
            plateau [ posFrom . y ]  [ posFrom . x ]   =  TARGET ;
         }  

     }  
    return cancel ;
 }  

Position undone (t_Plateau plateau ,  Position player ,  char move)  { 
    Position newPlayer  =  player ;
    Position caisse  =  player ;
    bool moveCaisse  =  false ;
    switch (move)  { 
        case UNDOH - 32 :  
            caisse . y  =  player . y - 1 ;
            moveCaisse  =  true ;
        case UNDOH : 
            newPlayer . y  =  player . y + 1 ;
            break ;
        case UNDOG - 32 : 
            caisse . x  =  player . x - 1 ;
            moveCaisse  =  true ;
        case UNDOG : 
            newPlayer . x  =  player . x  + 1 ;
            break ;
        case UNDOB - 32 : 
            caisse . y  =  player . y + 1 ;
            moveCaisse  =  true ;
        case UNDOB : 
            newPlayer . y  =  player . y - 1 ;
            break ;
        case UNDOD - 32 : 
            caisse . x  =  player . x + 1 ;
            moveCaisse  =  true ;
        case UNDOD : 
            newPlayer . x  =  player  .  x - 1 ;
            break ;
     }  
    if  (plateau [ player . y ]  [ player . x ]   ==   PLAYER)  { 
        plateau [ player . y ]  [ player . x ]   =  SPACE ;
     }   else { 
        plateau [ player . y ]  [ player . x ]   =  TARGET ;
     }  
    if  (plateau [ newPlayer . y ]  [ newPlayer . x ]   ==   SPACE)  { 
        plateau [ newPlayer . y ]  [ newPlayer . x ]   =  PLAYER ;
     }   else { 
        plateau [ newPlayer . y ]  [ newPlayer . x ]   =  PLAYERTARGET ;
     }  
    if (moveCaisse)  { 
        if  (plateau [ player . y ]  [ player . x ]   ==   SPACE)  { 
            plateau [ player . y ]  [ player . x ]   =  BOX ;
         }   else { 
            plateau [ player . y ]  [ player . x ]   =  BOXTARGET ;
         }  
        if  (plateau [ caisse . y ]  [ caisse . x ]   ==   BOX)  { 
            plateau [ caisse . y ]  [ caisse . x ]   =  SPACE ;
         }   else { 
        plateau [ caisse . y ]  [ caisse . x ]   =  TARGET ;
         }  
     }  
    return newPlayer ;
 }  

 
Position controle (t_Plateau plateau , char input ,  Position pos ,  
    int  * aMove ,  t_tabDeplacement listeMove)  { 
    Position to  =  pos ;
    Position further  =  pos ;
    bool caisse ;
    switch  (input)  { 
        case UNDO : 
            if  ( * aMove > 0)  { 
                 ( * aMove) -- ;
                return undone (plateau ,  pos ,  listeMove [ ( * aMove)  ] );
             }  
            break ;
        
        case UP_A : 
        case UP_W : 
        case UP_Z : 
            to . y  =  pos . y-1 ;
            further . y  =  pos . y-2 ; //la ou sera poussé la box
            listeMove [  * aMove ]   =  UNDOH ;
            break ;
        case DOWN_A : 
        case DOWN_S : 
            to . y  =  pos . y + 1 ;
            further . y  =  pos . y + 2 ; //la ou sera poussé la box
            listeMove [  * aMove ]   =  UNDOB ;
            break ;
        case LEFT_A : 
        case LEFT_Q : 
        case LEFT : 
            to . x  =  pos . x-1 ;
            further . x  =  pos . x-2 ; //la ou sera poussé la box
            listeMove [  * aMove ]   =  UNDOG ;
            break ;
        case RIGHT_D : 
        case RIGHT_A : 
            to . x  =  pos . x + 1 ;
            further . x  =  pos . x + 2 ; //la ou sera poussé la box
            listeMove [  * aMove ]   =  UNDOD ;
            break ;
        default : 
            return pos ;}  
    if  (!deplacer (plateau ,  pos ,  to ,  further ,  &caisse) )  { 
            if  (caisse)  { 
                listeMove [  * aMove ]  -=  32 ;}  
             ( * aMove)  ++  ;
        return to ;
     }  
    return pos ;
 }

bool gagne (t_Plateau plateau)  { 
    for (int i  =  0 ; i < TAILLE ; i ++ )  { 
        for (int j  =  0 ; j < TAILLE ; j ++ )  { 
            if  (plateau [ i ]  [ j ]   ==   TARGET || plateau [ i ]  [ j ] 
                  ==   PLAYERTARGET )  { 
                return false ;
             }  
         }  
     }  

    return true ;
 }  



char dep2touche(char touche){
    char toucheRetour;
    switch(touche){
        //lettre majuscule
        case UPCAISSE:
            toucheRetour=UP_Z;
            break;
        case LEFTCAISSE:
            toucheRetour=LEFT_Q;
            break;
        case DOWNCAISSE:
            toucheRetour=DOWN_S;
            break;
        case RIGHTCAISSE:
            toucheRetour=RIGHT_D;
            break;
        //lettre majuscule
        case UNDOH:
            toucheRetour=UP_Z;
            break;
        case UNDOG:
            toucheRetour=LEFT_Q;
            break;
        case UNDOB:
            toucheRetour=DOWN_S;
            break;
        case UNDOD:
            toucheRetour=RIGHT_D;
            break;
        default: break;
    }
    return toucheRetour;
}

