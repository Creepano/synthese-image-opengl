#include <cstdio>
#include <cstdlib>
/****************************************************************/
/* works on mac using freeglut and xquartz                      */
/* you can install freeglut using homebrew                      */
/* xquartz can be downloaded at http://xquartz.macosforge.org   */
/****************************************************************/
#include <GL/freeglut.h>
#include <jpeglib.h>
#include <jerror.h>
#include <cmath>
#ifdef __WIN32
#pragma comment (lib, "jpeg.lib")
#endif

unsigned char image[256*256*3];
char presse;
int anglex=30,angley=20,x,y,xold,yold;
// Textures JPEG de l'abdomen textures[0] et textures[1] celle du damier
GLuint textures[2];
GLubyte imageDamier[256][256][3];
float zoom= 6;
float anglePatte = 0;
int sens= 1;
float angleCrochet = 0.0f;

void affichage();
void clavier(unsigned char touche,int x,int y);
void souris(int boutton, int etat,int x,int y);
void sourismouv(int x,int y);
void redim(int l,int h);
void loadJpegImage(char *fichier);
void genererTextureDamier();
void touchesSpeciales(int touche, int x, int y);
void dessinerAbdomenParametrique(float rx, float ry, float rz, int N, int M);
void dessinerAraignee();
void dessinerPatte(float angH,float cote);
void initLumiere();
void dessinerPedipalpe(float cote);

//Animation automatique de marche
void idle()
{
    anglePatte += sens*0.4;
    if (anglePatte > 15)
    { sens = -1;}
    else if (anglePatte<-15)
    sens =1;

    glutPostRedisplay();
}

int main(int argc,char **argv)
{
  /* Chargement de la texture */
  loadJpegImage("./texture.jpg");

  // Assombrit le JPEG pour obtenir un pelage sombre d'araignée
  for (int i = 0; i < 256 * 256 * 3; i+=3)
  {
    unsigned char gris=image[i]/4;
    image[i] = gris;
    image[i+1] = gris;
    image[i+2] = gris +5;
  }
  /* Creation de la fenetre OpenGL */
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(600,600);
  glutCreateWindow("Texture JPEG");

  /* Initialisation de l'etat d'OpenGL */
  glClearColor(0.0,0.0,0.0,0.0);
  //active le lissage de Gouraud pour que la lumière calcule des dégradés continus sur les surfaces courbes plutôt que des facettes plates
  glShadeModel(GL_SMOOTH);
  initLumiere();
  glEnable(GL_DEPTH_TEST);

  /* Mise en place de la projection perspective */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0,1,1.0,25);
  glMatrixMode(GL_MODELVIEW);
  // Réserve pour les 2 textures
  glGenTextures(2, textures);

  /* Texture de base du TP */
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,256,256,0,
	       GL_RGB,GL_UNSIGNED_BYTE,image);

  /* Texture du damier */
  genererTextureDamier();
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,256,256,0,
	       GL_RGB,GL_UNSIGNED_BYTE,imageDamier);
  glEnable(GL_TEXTURE_2D);

  /* Mise en place des fonctions de rappel */
  glutSpecialFunc(touchesSpeciales);
  glutDisplayFunc(affichage);
  glutKeyboardFunc(clavier);
  glutMouseFunc(souris);
  glutMotionFunc(sourismouv);
  glutReshapeFunc(redim);
  glutIdleFunc(idle);

  /* Entrée dans la boucle principale glut */
  glutMainLoop();
  return 0;
}

void affichage()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  // On place l'oeil de la caméra en 0,0,zoom
  gluLookAt(0.0,0.0,zoom,0.0,0.0,0.0,0.0,1.0,0.0);

  glRotatef(angley,1.0,0.0,0.0);
  glRotatef(anglex,0.0,1.0,0.0);

  dessinerAraignee();
  // On active la texture du damier et on la plaque à un carré
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glColor3f(1,1,1);

  glNormal3f(0.0f, 1.0f, 0.0f);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0);   glVertex3f(-2.5, -0.34, 2.5);
  glTexCoord2f(0.0,1.0);   glVertex3f(-2.5,-0.34, -2.5);
  glTexCoord2f(1.0,1.0);   glVertex3f( 2.5,-0.34, -2.5);
  glTexCoord2f(1.0,0.0);   glVertex3f( 2.5, -0.34, 2.5);
  glEnd();

  glutSwapBuffers();
}

void clavier(unsigned char touche,int x,int y)
{
  switch(touche) {
  case 'l':
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glutPostRedisplay();
    break;
  case 'n':
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glutPostRedisplay();
  break;
  //zoom en avant
  case 'z':
    if(zoom>1.2)
    {
      zoom-=0.2;
    }
    glutPostRedisplay();
  break;
  //zoom en arrière
  case 'Z':
    if (zoom<10)
    {
     zoom+=0.2;
    }
    glutPostRedisplay();
  break;

  case 'c': // Refermer les crochets
    if (angleCrochet < 25.0f) angleCrochet += 3.0f;
    glutPostRedisplay();
    break;
  case 'v': // Écarter les crochets
    if (angleCrochet > -15.0f) angleCrochet -= 3.0f;
    glutPostRedisplay();
    break;

  case 27: /* touche ESC */
    exit(0);
  default:
	break;
  }
}
// Bouge la scène via les flèches
void touchesSpeciales(int touche, int x, int y)
{
  switch(touche)
  {
    case GLUT_KEY_LEFT:
      anglex-=5;
    break;

    case GLUT_KEY_RIGHT:
      anglex+=5;
    break;

    case GLUT_KEY_UP:
      angley -= 5;
    break;

    case GLUT_KEY_DOWN:
      angley += 5;
    break;
  }
  glutPostRedisplay();
}

void souris(int bouton, int etat,int x,int y)
{
  if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_DOWN)
  {
    presse = 1;
    xold = x;
    yold=y;
  }
  if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_UP)
    presse=0;
}

void sourismouv(int x,int y)
{
  if (presse)
  {
    anglex=anglex+(x-xold);
    angley=angley+(y-yold);
    glutPostRedisplay();
  }

  xold=x;
  yold=y;
}

void redim(int l,int h)
{
  if (l<h)
    glViewport(0,(h-l)/2,l,l);
  else
    glViewport((l-h)/2,0,h,h);
}

void genererTextureDamier()
{
  for(int i=0;i<=255;i++)
  {
    for(int j=0;j<=255;j++)
    {
      int premierepartie= i/32;
      int deuxiemepartie = j/32;

      if ((premierepartie+deuxiemepartie)%2==0) //alterne blanc et noir divisié par 32 permet de faire un damier 8x8
      {
        imageDamier[i][j][0]=255;
        imageDamier[i][j][1]=255;
        imageDamier[i][j][2]=255;
       }
       else
       {
        imageDamier[i][j][0]=0;
        imageDamier[i][j][1]=0;
        imageDamier[i][j][2]=0;
       }
    }
  }
}

// Tracer de l'ellipsoîde pour l'abdomen
void dessinerAbdomenParametrique(float rx, float ry, float rz, int N, int M )
{
  for (int j=0; j< M; j++)
  {
    float phi1 = -M_PI/2 + (float)j*M_PI/M; //-pi/2 à pi/2
    float phi2= -M_PI/2 +(float)(j+1)*M_PI/M;

    float t1= (float)j/(float)M;
    float t2=(float)(j+1)/(float)M;

    glBegin(GL_QUAD_STRIP);
    for (int i=0; i<=N; i++)
    {
        float theta=(float)i*2*M_PI/N; //0 à 2pi
        float s= (float)i/(float)N;
        //calcul le vecteur normal n
        float nx1= cosf(phi1) * cosf(theta);
        float ny1= sinf(phi1);
        float nz1= cosf(phi1) * sinf(theta);
        glNormal3f(nx1, ny1, nz1); // Normale unitaire pour la lumière
        glTexCoord2f(s, t1); // Coordonnée de texture
        glVertex3f(rx * nx1, ry * ny1, rz * nz1); // Position 3D étirée selon rx, ry, rz

        float nx2= cosf(phi2) * cosf(theta);
        float ny2= sinf(phi2);
        float nz2= cosf(phi2) * sinf(theta);
        glNormal3f(nx2, ny2, nz2);
        glTexCoord2f(s, t2);
        glVertex3f(rx * nx2, ry * ny2, rz * nz2);
    }
    glEnd();
  }
}

void dessinerPatte(float angH, float cote)
{
  glPushMatrix();
    // Orientation horizontale et élévation à 42 degrés
    glRotatef(cote * angH, 0, 1, 0);
    glRotatef(cote * 42, 0, 0, 1);

    // Fémur de longueur 0.45
    glPushMatrix();
      glTranslatef(cote * 0.225, 0, 0);// Décalage de la moitié pour pivoter sur l'articulation et non au centre du cube
      glScalef(0.45, 0.055, 0.055);
      glutSolidCube(1);
    glPopMatrix();

    // Rotule 1 au bout du fémur
    glTranslatef(cote * 0.45, 0, 0);
    glutSolidSphere(0.035,10,10);

    //Pliage vers le bas du tibia
    glRotatef(cote * (-65),0,0,1);

    // Tibia longieur 0.44
    glPushMatrix();
      glTranslatef(cote * 0.22, 0, 0);
      glScalef(0.44, 0.04, 0.04);
      glutSolidCube(1);
    glPopMatrix();

    // Rotule 2 au bout du tibia
    glTranslatef(cote * 0.44, 0,0);
    glutSolidSphere(0.025,10,10);

    //Pliage vers le bas de la pointe
    glRotatef(cote * (-50), 0,0,1);

    // Pointe de longueur 0.64
    glPushMatrix();
      glTranslatef(cote * 0.32,0,0);
      glScalef(0.64, 0.022,0.022);
      glutSolidCube(1);
    glPopMatrix();

  glPopMatrix();
}

void dessinerPedipalpe(float cote)
{
  glPushMatrix();
    //Orientation vers l'avant
    glRotatef(cote * 20,0,1,0);
    glRotatef(12,1,0,0);

    // Première partie
    glPushMatrix();
      glTranslatef(0,0,0.09);
      glScalef(0.035,0.035,0.18);
      glutSolidCube(1);
    glPopMatrix();

    // Rotule
    glTranslatef(0,0,0.18);
    glutSolidSphere(0.024,10,10);

    // Deuxième partie
    glRotatef(cote *(-10),0,1,0);
    glRotatef(35,1,0,0);

    glPushMatrix();
      glTranslatef(0,0,0.08);
      glScalef(0.028,0.028,0.16);
      glutSolidCube(1);
    glPopMatrix();

    glTranslatef(0,0,0.16);
    glutSolidSphere(0.018,10,10);
  glPopMatrix();
}

void dessinerAraignee()
{
  // ABDOMEN : texture avec textures[0]
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  glColor3f(1,1,1);

  glPushMatrix();
    glTranslatef(0.0, 0.3, -0.65);
    dessinerAbdomenParametrique(0.5,0.4,0.65,24,24);
  glPopMatrix();

  // TETE : sans texture
  glDisable(GL_TEXTURE_2D);
  glColor3f(0.15,0.15,0.15);

  glPushMatrix();
    glTranslatef(0,0.2,0.32);
    glScalef(0.35,0.22,0.35);
    glutSolidSphere(1,20,20);
  glPopMatrix();

  // HUIT YEUX
  glColor3f(0.95, 0.05, 0.05);

  float xYeuxbas[4] = {-0.14,-0.05, 0.05, 0.14};
  float zYeuxbas[4] = {0.64, 0.67, 0.67, 0.64};
  float rYeuxbas[4] = {0.025,0.038, 0.038, 0.025};

  for (int i=0; i<4; i++)
  {
    glPushMatrix();
      glTranslatef(xYeuxbas[i],0.25, zYeuxbas[i]);
      glutSolidSphere(rYeuxbas[i],10,10);
    glPopMatrix();
  }

  float xYeuxhaut[4] = {-0.18,-0.07, 0.07, 0.18};
  float zYeuxhaut[4] = {0.56, 0.62, 0.62, 0.56};

  for (int i=0; i<4; i++)
  {
    glPushMatrix();
      glTranslatef(xYeuxhaut[i],0.32, zYeuxhaut[i]);
      glutSolidSphere(0.022,10,10);
    glPopMatrix();
  }

  // Tracé des crochets
  glColor3f(0.12,0.12,0.12);
  // Crochet gauche
  glPushMatrix();
    glTranslatef(-0.09,0.09,0.63);
    glRotatef(30,1,0,0);
    glRotatef(angleCrochet,0,1,0);
    glScalef(0.05,0.16,0.05);
    glutSolidCube(1);
  glPopMatrix();

  // Crochet droit
  glPushMatrix();
    glTranslatef(0.09,0.09,0.63);
    glRotatef(30,1,0,0);
    glRotatef(-angleCrochet,0,1,0);
    glScalef(0.05,0.16,0.05);
    glutSolidCube(1);
  glPopMatrix();

  // Tracé des pédipalpes
  glColor3f(0.15, 0.15, 0.15);

  // Pédipalpe gauche
  glPushMatrix();
    glTranslatef(0.14,0.12,0.58);
    dessinerPedipalpe(1);
  glPopMatrix();

  // Pédipale droit
  glPushMatrix();
    glTranslatef(-0.14,0.12,0.58);
    dessinerPedipalpe(-1);
  glPopMatrix();

  // Tracé des 8 pattes
  float zPattes[4] = {0.47, 0.37, 0.27, 0.17};
  float xPattes[4] = {0.31, 0.35, 0.35, 0.31};
  float anglesH[4] = {-45.0, -15.0, 15.0, 45.0};

  for (int i = 0; i < 4; i++)
  {
    //les pattes paires avancent et les impaires reculent
    float deltaG= (i%2==0) ? anglePatte: -anglePatte;
    float deltaD= (i%2==0) ? -anglePatte : anglePatte;
    // Patte gauche (cote = 1)
    glPushMatrix();
      glTranslatef(xPattes[i], 0.16f, zPattes[i]);
      dessinerPatte(anglesH[i]+deltaG, 1.0);
    glPopMatrix();

    // Patte droite (cote = -1)
    glPushMatrix();
      glTranslatef(-xPattes[i], 0.16, zPattes[i]);
      dessinerPatte(anglesH[i]+deltaD,-1.0);
    glPopMatrix();
  }
}

void initLumiere()
{
    //LUMIÈRE DIRECTIONNELLE (GL_LIGHT0 : w = 0.0)
    GLfloat light0_ambient[]  = {0.2, 0.2, 0.2, 1.0};
    GLfloat light0_diffuse[]  = {0.7, 0.7, 0.7, 1.0};
    GLfloat light0_specular[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat light0_dir[]      = {1.0, 1.0, 1.0, 0.0}; // Rayons parallèles

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_dir);
    glEnable(GL_LIGHT0);

    // LUMIÈRE PONCTUELLE LOCALE (GL_LIGHT1 : w = 1.0)
    GLfloat light1_diffuse[]  = {0.5, 0.5, 0.5, 1.0};
    GLfloat light1_specular[] = {0.6, 0.6, 0.6, 1.0};
    GLfloat light1_pos[]      = {0.0, 3.0, 0.0, 1.0}; // Source située à 3 unités au-dessus

    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glEnable(GL_LIGHT1);

    // Propriétés de brillance du matériau (carapace d'araignée)
    GLfloat mat_specular[] = {0.6, 0.6, 0.6, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 40.0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);// Recalcule la norme des normales déformées par les glScalef
    glEnable(GL_LIGHTING);
}

void loadJpegImage(char *fichier)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *file;
  unsigned char *ligne;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
#ifdef __WIN32
  if (fopen_s(&file,fichier,"rb") != 0)
    {
      fprintf(stderr,"Erreur : impossible d'ouvrir le fichier texture.jpg\n");
      exit(1);
    }
#elif __GNUC__
  if ((file = fopen(fichier,"rb")) == 0)
    {
      fprintf(stderr,"Erreur : impossible d'ouvrir le fichier texture.jpg\n");
      exit(1);
    }
#endif
  jpeg_stdio_src(&cinfo, file);
  jpeg_read_header(&cinfo, TRUE);

  if ((cinfo.image_width!=256)||(cinfo.image_height!=256)) {
    fprintf(stdout,"Erreur : l'image doit etre de taille 256x256\n");
    exit(1);
  }
  if (cinfo.jpeg_color_space==JCS_GRAYSCALE) {
    fprintf(stdout,"Erreur : l'image doit etre de type RGB\n");
    exit(1);
  }

  jpeg_start_decompress(&cinfo);
  ligne=image;
  while (cinfo.output_scanline<cinfo.output_height)
    {
      ligne=image+3*256*cinfo.output_scanline;
      jpeg_read_scanlines(&cinfo,&ligne,1);
    }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
}
