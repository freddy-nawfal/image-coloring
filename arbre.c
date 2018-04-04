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

typedef struct pixel{
  struct pixel *pere;
  int rgb[3];
  int height;
}*Pixel;

typedef struct Matrix{
  int X;
  int Y;
  Pixel **mat;
}Matrix;

int maxHeight=0;
int nbUnion=0;

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


int max(int a,int b){
  return (a>=b)?a:b;
}

void MakeSet(Pixel x){
  if(x->pere==NULL){//pour éviter de recréer l'ensemble
    x->pere=x;
  }
  return;
}

Pixel FindSet(Pixel x){
  Pixel tmp=x;
  while(tmp->pere!=tmp){
    tmp=tmp->pere;
  }
  return tmp;
}

Pixel FindColoredSet(Pixel x){
  if(x->rgb[0]==0){
    return x;
  }
  Pixel tmp=FindSet(x);
  if(tmp->rgb[0]==-1){
    for(int i=0;i<3;i++){
      tmp->rgb[i]=rand() % 255;
    }
  }
  return tmp;
}

void Union(Pixel x,Pixel y){
  nbUnion++;
  Pixel tmp1=FindSet(x);
  Pixel tmp2=FindSet(y);
  if(tmp1->height<tmp2->height){
    tmp1->pere=tmp2;
    maxHeight=max(maxHeight,tmp2->height);
  }
  else if(tmp1->height==tmp2->height){
    tmp1->pere=tmp2;
    tmp2->height++;
    maxHeight=max(maxHeight,tmp2->height);
  }
  else{
    tmp2->pere=tmp1;
    tmp1->height=max(tmp1->height,tmp2->height+1);
    maxHeight=max(maxHeight,tmp1->height);
  }
}

int SameRep(Pixel x,Pixel y){
  return FindSet(x)==FindSet(y);
}

Matrix Read(const char *path){
  int fd = open(path, O_RDONLY);
  unsigned char *f;
  int size;
  struct stat s;
  int status = fstat(fd, &s);
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

  int columns = sizes[0];
  int rows = sizes[1];

  Matrix newMatrice;
  newMatrice.mat = malloc(rows * sizeof(Pixel));
  int j;
  for(j=0; j<rows; j++){
    newMatrice.mat[j] = malloc(columns * sizeof(Pixel));
  }
  int k = 0;
  int ligne = 0;
  printf("1 - LECTURE [%d-%d]\n",rows,columns);

  for(i=debutImg; i<size && ligne<rows && k<columns; i++){
    fflush(stdout);
    if(f[i] != '\n' && f[i] != '\r' && f[i] != ' '){
      Pixel newPixel = malloc(sizeof(struct pixel));
      if(f[i] == '0'){
        newPixel->rgb[0] = -1;
        newPixel->pere = newPixel;
      }
      else{
        newPixel->rgb[0] = 0;
        newPixel->rgb[1] = 0;
        newPixel->rgb[2] = 0;
      }
      newMatrice.mat[ligne][k] = newPixel;
      k++;
      if(k == columns){
        ligne++;
        k = 0;
      }
      if(ligne == rows){
        break;
      }
    }
  }

  newMatrice.X = columns;
  newMatrice.Y = rows;
  return newMatrice;
}



void fprintPix(FILE *f ,Pixel x){
  Pixel tmp = FindColoredSet(x);
  fprintf(f,"%d %d %d ",tmp->rgb[0],tmp->rgb[1],tmp->rgb[2]);
}

int main(int argc, char const *argv[]) {
  Matrix mat2d=Read(argv[1]);
  //printf("\nMatrice de taille : col%d row%d\n",mat2d.X,mat2d.Y);
  int j=0;
  printf("2 - CREATION D'ENSEMBLE\n");
  for(int i=0;i<mat2d.Y;i++){//boucle sur les lignes
    for(int j=0;j<mat2d.X;j++){//boucle sur les colonnes
      if(mat2d.mat[i][j]->rgb[0]==-1){
        /*
              Suivi progression :
        */
        /*
        printf("\rCreation d'ensemble : %d %d",i,j);
        fflush(stdout);
        */
        MakeSet(mat2d.mat[i][j]);

          if(i+1<mat2d.Y && i+1>-1){
            if(mat2d.mat[i+1][j]->rgb[0]==-1){
              MakeSet(mat2d.mat[i+1][j]);

              if(!SameRep(mat2d.mat[i][j],mat2d.mat[i+1][j])){
                Union(mat2d.mat[i][j],mat2d.mat[i+1][j]);
              }
            }
          }
          if(j+1<mat2d.X && j+1>-1){
            if(mat2d.mat[i][j+1]->rgb[0]==-1){
              MakeSet(mat2d.mat[i][j+1]);

              if(!SameRep(mat2d.mat[i][j],mat2d.mat[i][j+1])){
                Union(mat2d.mat[i][j],mat2d.mat[i][j+1]);
              }
            }
          }
      }
    }
  }
  printf("3 - ECRITURE\n");
  char *path=strdup(argv[1]);
  path[strlen(path)-2]='p';
  FILE *f = fopen(path,"a+");
    fprintf(f,"P3\n%d %d\n255\n",mat2d.X,mat2d.Y);
  srand(time(NULL));
  for(int i=0;i<mat2d.Y;i++){//boucle sur les lignes
    for(int j=0;j<mat2d.X;j++){//boucle sur les colonnes
      /*
            Suivi progression :
      */
      /*
      printf("\rposition fprintf : %d %d",i,j);
      fflush(stdout);
      */
        fprintPix(f,mat2d.mat[i][j]);
        fprintf(f,"\n");
    }
  }
  printf("\n\nHauteur maximal des arbres atteinte : %d\n",maxHeight);
  printf("Nombre d'union : %d\n",nbUnion);

  return 0;
}
