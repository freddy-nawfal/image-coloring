#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

int debug = 0;

// structure de pixel
typedef struct pixel{
  struct pixel *rep; //tous les pixels pointent vers le représentant
  int rgb[3];
  struct pixel *next;
}*Pixel;


// structure d'ensemble
typedef struct S{
  struct pixel *head; // tete de l'ensemble
  struct pixel *tail; // queue de l'ensemble
}S;

// structure de matrice
typedef struct Matrix{
  int X;
  int Y;
  Pixel **mat;
}Matrix;



/////////////// GESTION DE LISTE ////////////////////

typedef struct ListeEns{
  struct S *ens;
  struct ListeEns *next;
}ListeEns;

ListeEns *ListeDEnsembles = NULL;

ListeEns *stockEns(S *ens){
  if(debug)printf("Stocking ens\n");
  if(ListeDEnsembles == NULL){
    ListeEns *newListe = NULL;
    newListe = malloc(sizeof(ListeEns));
    newListe->ens = ens;
    newListe->next = NULL;
    ListeDEnsembles = newListe;
    if(debug)printf("ListeDEnsembles est vide, creation\n");
    return newListe;
  }
  else{
    /*ListeEns *tmp = ListeDEnsembles;
    while(tmp->next != NULL){
      tmp = tmp->next;
    }*/
    ListeEns *newListe = NULL;
    newListe = malloc(sizeof(ListeEns));
    newListe->ens = ens;

    //newListe->next = NULL;
    //tmp->next = newListe;

    newListe->next = ListeDEnsembles;
    ListeDEnsembles = newListe;

    if(debug)printf("ListeDEnsembles n'est pas vide, ajout d'une nouvelle\n");
    return newListe;
  }
}

// precondition : x est dans un ensemble et ListeDEnsembles non vide
S *FindEns(Pixel x){
  ListeEns *tmp = ListeDEnsembles;
  if((x == NULL)||(tmp == NULL)) return NULL;
  while(tmp != NULL){
    if(tmp->ens->head == x){
      if(debug)printf("FINDENS: Found ens\n");
      return tmp->ens;
    }
    tmp = tmp->next;
  }
  return NULL;
  if(debug)printf("FINDENS: Didn't find ens\n");
}

int countNumberOfEns(){
  int i=0;
  ListeEns *tmp = ListeDEnsembles;
  if(tmp == NULL) return 0;
  while(tmp->next != NULL){
    i++;
    tmp = tmp->next;
  }
  return i+1;
}


void generateRandomColorForEachEns(){
  ListeEns *tmp = ListeDEnsembles;
  if(tmp == NULL) return;
  while(tmp != NULL){
    int randomnumber;
    tmp->ens->head->rgb[0] = rand() % 255;
    tmp->ens->head->rgb[1] = rand() % 255;
    tmp->ens->head->rgb[2] = rand() % 255;
    tmp = tmp->next;
  }
}

int removeEns(S *ens){
  ListeEns *tmp = ListeDEnsembles;
  if(tmp == NULL){
    printf("ListeDEnsembles ne contient rien\n");
    return 0;
  }

  while(tmp != NULL){
    if(tmp->next == NULL){
      if(tmp->ens == ens){
        free(tmp->ens);
        free(tmp);
        ListeDEnsembles = NULL;
        return 1;
      }
      else{
        return 0;
      }
    }
    if(tmp->next->ens == ens){
      if(tmp->next->next == NULL){
        free(tmp->next->ens);
        free(tmp->next);
        tmp->next = NULL;
        return 1;
      }
      else{
        ListeEns *tmpNext = tmp->next;
        tmp->next = tmp->next->next;
        free(tmpNext->ens);
        free(tmpNext);
        return 1;
      }
    }
    else{
      tmp = tmp->next;
    }
  }
  return 0;
}

//////////////// FONCTIONS DE PRINCIPALES ///////////////////


char *getLineByNB(int lineNB, const char *path){
  FILE *f = fopen(path, "r");
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int lineID = 0;
  while ((read = getline(&line, &len, f)) != -1) {
    if(lineID == lineNB){
      line[strcspn(line,"\n")] = 0;
      fclose(f);
      return line;
    }
    lineID++;
  }
  if(line)free(line);
  return "";
}



Matrix Read(const char *path){
  int fd = open(path, O_RDONLY);
  unsigned char *f;
  int size;
  struct stat s;
  int status = fstat(fd, & s);
  size = s.st_size;
  f = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);

  char *magicNB = getLineByNB(0, path);
  char *sizeXY = getLineByNB(1, path);
  char *found;
  int i = 0;
  int sizes[2];
  int debutImg = strlen(magicNB)+strlen(sizeXY)+2;

  found = strtok(sizeXY, " ");
  while(found != NULL){
    sizes[i] = atoi(found);
    i++;
    found = strtok(NULL, " ");
  }

  int sizeX = sizes[0];
  int sizeY = sizes[1];

  Matrix newMatrice;
  newMatrice.mat = malloc(sizeX * sizeof(Pixel));
  int j;
  for(j=0; j<sizeX; j++){
    newMatrice.mat[j] = malloc(sizeY * sizeof(Pixel));
  }
  int k = 0;
  int ligne = 0;
  printf("1 - LECTURE\n");

  for(i=debutImg; i<size; i++){
    if(f[i] != '\n' && f[i] != '\r'){
      Pixel newPixel = malloc(sizeof(struct pixel));
      newPixel->rep = newPixel;
      newPixel->next = NULL;
      if(f[i] == '0'){
        newPixel->rgb[0] = 255;
        newPixel->rgb[1] = 255;
        newPixel->rgb[2] = 255;
      }
      else{
        newPixel->rgb[0] = 0;
        newPixel->rgb[1] = 0;
        newPixel->rgb[2] = 0;
      }
      newMatrice.mat[ligne][k] = newPixel;
      k++;
      if(k == sizeX){
        ligne++;
        k = 0;
      }
      if(ligne == sizeY){
        break;
      }
    }

  }

  newMatrice.X = sizeY;
  newMatrice.Y = sizeX;
  return newMatrice;
}


// fonction de création d'un ensemble à partir d'un pixel
S *MakeSet(Pixel x){
  S *newEns = malloc(sizeof(S));
  newEns->head = x;
  newEns->tail = x;
  x->rep = x;
  x->next = NULL;

  return newEns;
}

// fonction permettant de trouver le représentant d'un ensemble
Pixel FindSet(Pixel x){
  return x->rep;
}

// fonction d'union de deux ensembles
S *Union(Pixel x, Pixel y){
  Pixel repx = FindSet(x);
  Pixel repy = FindSet(y);
  if(debug)printf("UNION: found sets\n");

  S *ensOfX = FindEns(repx);
  S *ensOfY = FindEns(repy);
  if((ensOfX != NULL) && (ensOfY != NULL)){
    if(debug)printf("UNION: found ensembles\n");
  }
  else{
      printf("UNION: Did not find ensembles\n");
      exit(1);
  }
  if(ensOfX == ensOfY){
    if(debug)printf("UNION: X and Y already in same ensemble\n");
    return ensOfX;
  }


  if(debug)printf("There is %d Ensembles\n", countNumberOfEns());

  S *newEns = malloc(sizeof(S));
  //repy->rep = repx;
  Pixel tmp = repx;
  while(tmp != NULL){
    tmp = tmp->next;
  }
  tmp = repy;

  while(tmp->next != NULL){
    tmp->rep = repx;
    tmp = tmp->next;
  }
  tmp->rep = repx;

  newEns->head = repx;
  newEns->tail = tmp;
  stockEns(newEns);

  if(debug)printf("UNION: linked ensembles\n");

  if(!removeEns(ensOfX)){
    printf("REMOVEENS: ens X non trouvé\n");
    exit(1);
  }
  if(!removeEns(ensOfY)){
    printf("REMOVEENS: ens Y non trouvé\n");
    exit(1);
  }


  if(debug)printf("UNION: removed both ensembles\n");
  if(debug)printf("There is %d Ensembles\n", countNumberOfEns());

  return newEns;
}


///////////////// FONCTIONS UTILITAIRES ////////////////////

int isWhite(Pixel x){
  if((x->rgb[0] == 255) && (x->rgb[1] == 255) && (x->rgb[2] == 255)){
    return 1;
  }
  else{
    return 0;
  }
}

int isBlack(Pixel x){
  if((x->rgb[0] == 0) && (x->rgb[1] == 0) && (x->rgb[2] == 0)){
    return 1;
  }
  else{
    return 0;
  }
}


void printMatrix(Matrix m){
  int i,j;
  for(i=0; i<m.X; i++){
    for(j=0; j<m.Y; j++){
      printf("(%d %d %d) ", m.mat[i][j]->rgb[0], m.mat[i][j]->rgb[1], m.mat[i][j]->rgb[2]);
    }
    if(j == m.Y)printf("\n");
  }
}

void printMatrixWithRightsColorsKThanks(Matrix m){
  int i,j;
  for(i=0; i<m.X; i++){
    for(j=0; j<m.Y; j++){
      Pixel x = FindSet(m.mat[i][j]);
      printf("(%d %d %d) ", x->rgb[0], x->rgb[1], x->rgb[2]);
    }
    if(j == m.Y)printf("\n");
  }
}


int Write(Matrix m){
  FILE *f = fopen("fichierFinal.ppm", "w+");
  fprintf(f, "P3\n");
  fprintf(f, "%d %d\n", m.Y, m.X);
  fprintf(f, "255\n");

  int i,j;
  for(i=0; i<m.X; i++){
    printf("\r3 - ECRITURE: %d/%d", i, m.X-1);
    fflush(stdout);
    for(j=0; j<m.Y; j++){
      Pixel x = FindSet(m.mat[i][j]);
      fprintf(f, "%d %d %d ", x->rgb[0], x->rgb[1], x->rgb[2]);
      if(j%4==0){
        fprintf(f,"\n");
      }
    }
  }

  printf("\n");
  fclose(f);
}

float Get2DPerlinNoiseValue(float x, float y, float res){
    float tempX,tempY;
    int x0,y0,ii,jj,gi0,gi1,gi2,gi3;
    float unit = 1.0f/sqrt(2);
    float tmp,s,t,u,v,Cx,Cy,Li1,Li2;
    float gradient2[][2] = {{unit,unit},{-unit,unit},{unit,-unit},{-unit,-unit},{1,0},{-1,0},{0,1},{0,-1}};

    unsigned int perm[] =
       {151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,
        142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,
        203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,
        105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,
        187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,
        64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,
        47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,
        153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,
        112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,
        235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,
        127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,
        156,180};

    //Adapter pour la résolution
    x /= res;
    y /= res;

    //On récupère les positions de la grille associée à (x,y)
    x0 = (int)(x);
    y0 = (int)(y);

    //Masquage
    ii = x0 & 255;
    jj = y0 & 255;

    //Pour récupérer les vecteurs
    gi0 = perm[ii + perm[jj]] % 8;
    gi1 = perm[ii + 1 + perm[jj]] % 8;
    gi2 = perm[ii + perm[jj + 1]] % 8;
    gi3 = perm[ii + 1 + perm[jj + 1]] % 8;

    //on récupère les vecteurs et on pondère
    tempX = x-x0;
    tempY = y-y0;
    s = gradient2[gi0][0]*tempX + gradient2[gi0][1]*tempY;

    tempX = x-(x0+1);
    tempY = y-y0;
    t = gradient2[gi1][0]*tempX + gradient2[gi1][1]*tempY;

    tempX = x-x0;
    tempY = y-(y0+1);
    u = gradient2[gi2][0]*tempX + gradient2[gi2][1]*tempY;

    tempX = x-(x0+1);
    tempY = y-(y0+1);
    v = gradient2[gi3][0]*tempX + gradient2[gi3][1]*tempY;


    //Lissage
    tmp = x-x0;
    Cx = 3 * tmp * tmp - 2 * tmp * tmp * tmp;

    Li1 = s + Cx*(t-s);
    Li2 = u + Cx*(v-u);

    tmp = y - y0;
    Cy = 3 * tmp * tmp - 2 * tmp * tmp * tmp;

    return Li1 + Cy*(Li2-Li1);
}

Matrix generateRandomMatrix(int lignes, int colonnes, int resolution){
  Matrix newMatrice;
  newMatrice.mat = malloc(colonnes * sizeof(Pixel));
  int j;
  for(j=0; j<colonnes; j++){
    newMatrice.mat[j] = malloc(lignes * sizeof(Pixel));
  }

  newMatrice.X = colonnes;
  newMatrice.Y = lignes;

  int randomDisplacementI = rand() % 100000;
  int randomDisplacementJ = rand() % 100000;

  char number[100];
  sprintf(number, "%d-%d", randomDisplacementI,randomDisplacementJ);
  char fileName[500];
  strcpy(fileName, "fichierRANDOM");
  strcat(fileName, number);
  strcat(fileName, ".ppm");

  FILE *f = fopen(fileName, "w+");
  fprintf(f, "P3\n");
  fprintf(f, "%d %d\n", newMatrice.Y, newMatrice.X);
  fprintf(f, "255\n");

  int i;
  for(i=0; i<newMatrice.X; i++){
    for(j=0; j<newMatrice.Y; j++){
      float valeur = (Get2DPerlinNoiseValue(i+randomDisplacementI, j+randomDisplacementJ, resolution)+1)*0.5*255;
      if(valeur >= 150 && valeur <= 156) valeur = 0;
      else valeur = 255;
      fprintf(f, "%f %f %f ", valeur, valeur, valeur);


      /*float altitude = Get2DPerlinNoiseValue(i+randomDisplacementI, j+randomDisplacementJ, resolution)*100;
      if(altitude <= -50){
        fprintf(f, "%d %d %d ", 1, 0, 51);
      }
      else if(altitude <= 0){
        fprintf(f, "%d %d %d ", 1, 0, 110);
      }
      else if(altitude <= 10){
        fprintf(f, "%d %d %d ", 48, 23, 0);
      }
      else if(altitude <= 20){
        fprintf(f, "%d %d %d ", 3, 39, 0);
      }
      else if(altitude <= 50){
        fprintf(f, "%d %d %d ", 3, 71, 0);
      }
      else if(altitude <= 70){
        fprintf(f, "%d %d %d ", 27, 28, 11);
      }
      else if(altitude <= 80){
        fprintf(f, "%d %d %d ", 19, 20, 21);
      }
      else if(altitude <= 90){
        fprintf(f, "%d %d %d ", 12, 11, 12);
      }
      else if(altitude <= 95){
        fprintf(f, "%d %d %d ", 35, 35, 35);
      }
      else{
        fprintf(f, "%d %d %d ", 224, 233, 237);
      }*/

      if(j%4==0){
        fprintf(f,"\n");
      }
    }
  }
  fclose(f);
}


///////////////// ALGORITHME PRINCIPAL //////////////////////

int main(int argc, char const *argv[]) {
  srand(time(NULL));

  if(argc < 2){
    int i;
    for(i=0; i<10; i++){
      generateRandomMatrix(1000, 1000, 200);
    }
    return 0;
  }


  Matrix m = Read(argv[1]);

  int i,j;
  for(i=0; i<m.X; i++){//parcours des lignes
    if(!debug){
      printf("\r2 - TRAITEMENT: %d/%d -- ENSEMBLES: %d", i, m.X-1, countNumberOfEns());
      fflush(stdout);
    }
    for(j=0; j<m.Y; j++){//parcours des colonnes
      if(debug)printf("CHECKING %d:%d\n", i,j);
      if(isWhite(m.mat[i][j])){
        if(FindEns(FindSet(m.mat[i][j])) == NULL){
          if(debug)printf("%d:%d is not black\n", i,j);
          S *newEns1 = MakeSet(m.mat[i][j]);
          if(stockEns(newEns1) == NULL){
            printf("Erreur de stockage\n");
            exit(1);
          }
        }

        // on test en bas

        if(i<m.X-1){
          if(isWhite(m.mat[i+1][j])){
            if(FindEns(FindSet(m.mat[i+1][j])) == NULL){
              if(debug)printf("found match with %d:%d \n", i+1,j);
              S *newEns2 = MakeSet(m.mat[i+1][j]);
              if(stockEns(newEns2) == NULL){
                printf("Erreur de stockage\n");
                exit(1);
              }
            }
            else{
              if(debug)printf("%d:%d is already in an ensemble\n", i+1,j);
            }
            if(debug)printf("uniting %d:%d with %d:%d\n", i,j,i+1,j);
            Union(m.mat[i][j], m.mat[i+1][j]);
          }
        }


        // on test a droite
        if(j<m.Y-1){
          if(isWhite(m.mat[i][j+1])){
            if(FindEns(FindSet(m.mat[i][j+1])) == NULL){
              if(debug)printf("found match with %d:%d \n", i,j+1);
              S *newEns3 = MakeSet(m.mat[i][j+1]);
              if(stockEns(newEns3) == NULL){
                printf("Erreur de stockage\n");
                exit(1);
              }
            }
            else{
              if(debug)printf("%d:%d is already in an ensemble\n", i,j+1);
            }
            if(debug)printf("uniting %d:%d with %d:%d\n", i,j,i,j+1);
            Union(m.mat[i][j], m.mat[i][j+1]);
          }
        }

        // on test en haut
        if(i>0){
          if(isWhite(m.mat[i-1][j])){
            if(FindEns(FindSet(m.mat[i-1][j])) == NULL){
              if(debug)printf("found match with %d:%d \n", i-1,j);
              S *newEns4 = MakeSet(m.mat[i-1][j]);
              if(stockEns(newEns4) == NULL){
                printf("Erreur de stockage\n");
                exit(1);
              }
            }
            else{
              if(debug)printf("%d:%d is already in an ensemble\n", i-1,j);
            }
            if(debug)printf("uniting %d:%d with %d:%d\n", i,j,i-1,j);
            Union(m.mat[i][j], m.mat[i-1][j]);
          }
        }

        // on test a gauche
        if(j>0){
          if(isWhite(m.mat[i][j-1])){
            if(FindEns(FindSet(m.mat[i][j-1])) == NULL){
              if(debug)printf("found match with %d:%d \n", i,j-1);
              S *newEns5 = MakeSet(m.mat[i][j-1]);
              if(stockEns(newEns5) == NULL){
                printf("Erreur de stockage\n");
                exit(1);
              }
            }
            else{
              if(debug)printf("%d:%d is already in an ensemble\n", i,j-1);
            }
            if(debug)printf("uniting %d:%d with %d:%d\n", i,j,i,j-1);
            Union(m.mat[i][j], m.mat[i][j-1]);
          }
        }


      }
      else{
        if(debug)printf("%d:%d is black\n", i,j);
      }

    }
  }


  generateRandomColorForEachEns();
  printf("\n");

  Write(m);

  return 0;
}
