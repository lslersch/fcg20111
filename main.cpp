#include <windows.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "lib/stdafx.h"
#include "lib/glm.h"
#include <math.h>

//CONSTANTES
#define PERSPECTIVE 1
#define ORTOGRAPHIC 2
#define THIRDPERSON 3
#define ANOTHERPLANE 4
#define ROTATEINCOBJ 3.0
#define X 0
#define Y 1
#define Z 2
#define PI 3.14159265
#define FPS 17 //Esse valor e definido por 1000/60
#define MAXSPEED 0.5 //Velocidade maxima do aviao
#define MINSPEED 0.3 //Velocidade para alcar voo
#define SPEEDINC 0.001 //Aceleracao

//area de visualizacao da camera ORTHO
#define XMIN -100
#define XMAX 100
#define YMIN -100
#define YMAX 100

//altura maxima do cenario
#define MAXHEIGHT 1000

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

//Header:
void renderScene(void);
void reshape(int w, int h);
void keyEvent(unsigned char key, int x, int y);
void specialEvent(int key, int x, int y);
void mouseEvent(int button, int state, int x, int y);
void motionEvent(int x, int y);
void setup(void);
void drawScene(void);
void drawAirplane(void);
void drawHangar(void);
void drawBuilding(void);
GLuint glmLoadTexture(char *filename, GLboolean alpha, GLboolean repeat, GLboolean filtering, GLboolean mipmaps, GLfloat *texcoordwidth, GLfloat *texcoordheight);
void setPerspectiveView(void);
void setOrtographicView(void);
void movePlane(void);
void idleFunc(void);
void setThirdPersonView(void);
void setAnotherPlaneView(void);
void refreshCamera(void);


//Variaveis globais:
GLMmodel* pmodel1 = NULL;
GLMmodel* pmodel2 = NULL;
GLMmodel* pmodel3 = NULL;
int cameraType = PERSPECTIVE; //inicializa a camera como perspectiva
int viewPortHeight;
int viewPortWidth;
int firstTime=1;
GLfloat luzAmbiente[4]={0.5,0.5,0.5,0.5};	//luz ambiente 
GLfloat luzDifusa[4]={1.0,1.0,1.0,1.0};		 // "cor" 
GLfloat luzEspecular[4]={1.0, 1.0, 1.0, 1.0};// "brilho" 
GLfloat posicaoLuz[4]={50.0, 99.0, 0.0, 0.0};   // inicial
double planePosition[3]={0,1,697};
double pointVector[3]={0,0,1};
double planeSpeed=0.0;
float rotateAngle[3]={0,180,0};
//float upVector[3]={0,1,0};
//Texturas:
GLuint textureSky;
GLuint textureGrass;
GLuint textureLane;
GLuint textureBuilding1;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition (15, 15);
	glutCreateWindow("Simulador");
	glutDisplayFunc(renderScene);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyEvent);
	glutSpecialFunc(specialEvent);
	//glutMouseFunc(mouseEvent);
	//glutMotionFunc(motionEvent);
	glutIdleFunc( idleFunc );	

	setup();

	glutMainLoop();

	return 0;
}



void renderScene(void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(1.0f, 1.0f, 1.0f);

	//Desenha o Aviao
	glLoadIdentity();
	glTranslated(planePosition[X], planePosition[Y], planePosition[Z]);
	glRotatef(rotateAngle[Y],0,1,0);
	glRotatef(-rotateAngle[X],1,0,0);
	glRotatef(rotateAngle[Z],0,0,1);
	drawAirplane();
	
	//Desenha a Cena
	glPushMatrix();	
	glLoadIdentity();
	drawScene();
	glPopMatrix();


	//Comandos para acompanhar o comportamento do v�o
	//system("cls");
	printf("Velocidade: %f/%f\n",planeSpeed,MAXSPEED);
	printf("Direcao: %f,%f,%f\n",pointVector[X],pointVector[Y],pointVector[Z]);
	printf("Posicao: %f,%f,%f\n\n",planePosition[X],planePosition[Y],planePosition[Z]);


	glutSwapBuffers();
}

void reshape(int w, int h)
{
	viewPortHeight = h;
	viewPortWidth = w;
    refreshCamera();
	glMatrixMode (GL_MODELVIEW);
}

void movePlane(void)
{
	if (planeSpeed<MINSPEED && planePosition[Y]>1) //Simulacao de gravidade
		{
			planePosition[Y]-=0.1;
			if( rotateAngle[X]>-90 && rotateAngle[X]<=90)
				rotateAngle[X]-=ROTATEINCOBJ*10/100;
			if( rotateAngle[X]<=-90 && rotateAngle[X]>90)
				rotateAngle[X]+=ROTATEINCOBJ*10/100;
		}


	//Set new camera Up Vector //Tentativa de rotacionar a camera
//	upVector[X]=sin(rotateAngle[Z]*PI/180)*cos(rotateAngle[Y]*PI/180);
//	upVector[Y]=cos(rotateAngle[Z]*PI/180)*cos(rotateAngle[X]*PI/180);
//	upVector[Z]=sin(rotateAngle[X]*PI/180)*sin(rotateAngle[Y]*PI/180);

	
	//Set new plane Point Vector
	//pointVector[X]=cos(rotateAngle[Z]*PI/180)*sin(rotateAngle[Y]*PI/180);
	//pointVector[Y]=sin(rotateAngle[X]*PI/180)*cos(rotateAngle[Z]*PI/180);
	pointVector[X]=sin(rotateAngle[Y]*PI/180)*cos(rotateAngle[X]*PI/180);
	pointVector[Y]=sin(rotateAngle[X]*PI/180);
	pointVector[Z]=cos(rotateAngle[Y]*PI/180)*cos(rotateAngle[X]*PI/180);

	//Setup new Plane Position
	planePosition[X]+= pointVector[X]*planeSpeed;
	planePosition[Y]+= pointVector[Y]*planeSpeed;
	planePosition[Z]+= pointVector[Z]*planeSpeed;

	if (planePosition[Y]<=1) //Simulacao de atrito e ajuste do aviao no solo
	{
		planeSpeed-=SPEEDINC*10/100;
		if(planeSpeed<0)
			planeSpeed=0;
		if( (rotateAngle[X]>-90 && rotateAngle[X]<0) || (rotateAngle[X]>90 && rotateAngle[X]<180))
			rotateAngle[X]+=ROTATEINCOBJ*10/100;
		if( (rotateAngle[X]<=-90 && rotateAngle[X]>-180) || (rotateAngle[X]<=90 && rotateAngle[X]>0))
			rotateAngle[X]-=ROTATEINCOBJ*10/100;

	}


	if (planePosition[Y]<1) //Colisao com o solo
		planePosition[Y]=1;
}
void idleFunc(void)
{
		movePlane();
		refreshCamera();
		glutPostRedisplay();
		Sleep(FPS);
}
void keyEvent(unsigned char key, int x, int y)
{
	switch (key) 
		{
		//ESC - encerra o programa
		case 27:
			exit(0);
			break;
	

		//W - Gira em torno do eixo x
		case 'w':
			if(planePosition[Y]>1 && planeSpeed>MINSPEED)
			{
			glRotatef(-ROTATEINCOBJ,1,0,0);
			rotateAngle[X]-=ROTATEINCOBJ;
			if(rotateAngle[X]<=-360)
				rotateAngle[X]+=360.0;
			movePlane();
			}
			break;

		//S - Gira em torno do eixo x
		case 's':

				if(planeSpeed>MINSPEED)
				{
				glRotatef(ROTATEINCOBJ,1,0,0);
				rotateAngle[X]+=ROTATEINCOBJ;
				if(rotateAngle[X]>=360)
					rotateAngle[X]-=360.0;
				movePlane();
				}

			break;
	  
		//A - Gira em torno do eixo Y
		case 'a':
			if(planeSpeed>0)
			{
			rotateAngle[Y]+=ROTATEINCOBJ;
			if(rotateAngle[Y]>=360)
				rotateAngle[Y]-=360.0;

			movePlane();
			glRotatef(ROTATEINCOBJ,0,1,0);
			}
			break;

		//D - Gira em torno do eixo Y
		case 'd':
			if(planeSpeed>0)
			{
			rotateAngle[Y]-=ROTATEINCOBJ;
			if(rotateAngle[Y]<=-360)
				rotateAngle[Y]+=360.0;
			movePlane();
			glRotatef(-ROTATEINCOBJ,0,1,0);
			}
			break;

		//E - Gira em torno do eixo Z
		case 'e':
			if(planeSpeed>0 && planePosition[Y]>1)
			{
			rotateAngle[Z]+=ROTATEINCOBJ;
			if(rotateAngle[Z]>=360)
				rotateAngle[Z]-=360.0;
			movePlane();
			glRotatef(ROTATEINCOBJ,0,0,1);
			}
			break;	
		
		//Q - Gira em torno do eixo Z
		case 'q':
			if(planeSpeed>0 && planePosition[Y]>1)
			{
			rotateAngle[Z]-=ROTATEINCOBJ;
			if(rotateAngle[Z]<=-360)
				rotateAngle[Z]+=360.0;
			movePlane();
			glRotatef(-ROTATEINCOBJ,0,0,1);
			}
			break;	

		case '+':
			planeSpeed+=SPEEDINC;
			if (planeSpeed>MAXSPEED)
				planeSpeed=MAXSPEED;
			break;
		case '-':
			planeSpeed-=SPEEDINC;
			if(planeSpeed<0)
				planeSpeed=0;
			break;

  		}

	refreshCamera();	
	glutPostRedisplay();
}

void specialEvent(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F1:
			cameraType=PERSPECTIVE;
			break;

	case GLUT_KEY_F2:
			cameraType=ORTOGRAPHIC;
			break;

	case GLUT_KEY_F3:         
			cameraType=THIRDPERSON;
			break;

	case GLUT_KEY_F4:         
			cameraType=ANOTHERPLANE;
			break;
	}
	refreshCamera();	
	glutPostRedisplay();
}

void mouseEvent(int button, int state, int x, int y)
{
}

void motionEvent(int x, int y)
{
}

void setup(void)
{
	glClearColor (1.0f, 1.0f, 1.0f, 1.0f);   
	glShadeModel (GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
   
	//ILUMINACAO
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz );
	glLightfv(GL_LIGHT0, GL_AMBIENT,luzAmbiente);
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular );
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa );

	//Carrega arquivos de textura
	float w, h;
	textureSky = glmLoadTexture("sky.tga", GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE, &w, &h);
	textureGrass = glmLoadTexture("grass.tga", GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE, &w, &h);
	textureLane = glmLoadTexture("lane.tga", GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE, &w, &h);
	textureBuilding1 = glmLoadTexture("building1.tga", GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE, &w, &h);

}

void drawScene(void)
{
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//======================================== ESFERA ========================================
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureSky);

	GLUquadric *qobj = gluNewQuadric();
	gluQuadricTexture(qobj, GL_TRUE);
	gluSphere(qobj, 1000.0f, 50, 50);//se aumentar o raio da esfera, DEVE-SE aumentar as coordenadas do plano abaixo assim como a constante "MAXHEIGHT"
	gluDeleteQuadric(qobj);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);

	//======================================== PLANO ========================================
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureGrass);

	glBegin(GL_QUADS);
	glTexCoord2d(0,0); glVertex3f(-1000.0f, 0.0f, 1000.0f);
	glTexCoord2d(50,0); glVertex3f(-1000.0f, 0.0f, -1000.0f);
	glTexCoord2d(50,50); glVertex3f(1000.0f, 0.0f, -1000.0f);
	glTexCoord2d(0,50); glVertex3f(1000.0f, 0.0f, 1000.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	//==============================+========== PISTA ==============================+==========
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureLane);

	glBegin(GL_QUADS);
	glTexCoord2d(0,3); glVertex3f(5.0f,0.1f,700.f);
	glTexCoord2d(0,0); glVertex3f(5.0f,0.1f,550.0f);
	glTexCoord2d(1,0); glVertex3f(-5.0f,0.1f,550.0f);
	glTexCoord2d(1,3); glVertex3f(-5.0f,0.1f,700.0f);

	glEnd();

	glDisable(GL_TEXTURE_2D);

	//======================================== PREDIOS ========================================
	glLoadIdentity();
	glColor3f(1.0, 0.0, 0.0);
	glTranslatef(-20.0, 0.0, -20.0);
	glutSolidCube(15.0);

	glLoadIdentity();
	glColor3f(0.0, 1.0, 0.0);
	glTranslatef(-20.0, 0.0, 20.0);
	glutSolidCube(15.0);

	glLoadIdentity();
	glColor3f(0.0, 0.0, 1.0);
	glTranslatef(20.0, 0.0, -20.0);
	glutSolidCube(15.0);

	glLoadIdentity();
	glColor3f(1.0, 1.0, 0.0);
	glTranslatef(20.0, 0.0, 20.0);
	glutSolidCube(15.0);

	//glLoadIdentity();
	//drawBuilding();

	glLoadIdentity();
	drawHangar();
}

void drawAirplane(void)
{
	if (!pmodel1) 
	{
		// this is the call that actualy reads the OBJ and creates the model object
        pmodel1 = glmReadOBJ("airplane.obj");	
        if (!pmodel1) exit(0);
        glmUnitize(pmodel1);
        glmFacetNormals(pmodel1);        
		glmVertexNormals(pmodel1, 90.0);
		glTranslatef(0.0f, 0.0f, 697.0f);
		glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

    }
		glmDraw(pmodel1, GLM_SMOOTH | GLM_TEXTURE);
		glEnable(GL_COLOR_MATERIAL);
}

void drawHangar(void)
{
	//HANGAR
	if (!pmodel2) 
	{
		// this is the call that actualy reads the OBJ and creates the model object
        pmodel2 = glmReadOBJ("hangar.obj");	
        if (!pmodel2) exit(0);
        glmUnitize(pmodel2);
        glmFacetNormals(pmodel2);        
		glmVertexNormals(pmodel2, 90.0);
    }
	glTranslatef(0.0f, 1.0f, 697.0f);
	glScalef(3.0f, 3.0f, 3.0f);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glmDraw(pmodel2, GLM_SMOOTH | GLM_TEXTURE | GLM_MATERIAL);
	glEnable(GL_COLOR_MATERIAL);
}

void drawBuilding(void)
{
	if (!pmodel3) 
	{
		// this is the call that actualy reads the OBJ and creates the model object
        pmodel3 = glmReadOBJ("predio.obj");	
        if (!pmodel3) exit(0);
        glmUnitize(pmodel3);
        glmFacetNormals(pmodel3);        
		glmVertexNormals(pmodel3, 90.0);
    }
	glTranslatef(0.0f, 1.0f, 0.0f);
	glScalef(100.0f, 100.0f, 100.0f);
    glmDraw(pmodel3, GLM_SMOOTH | GLM_TEXTURE | GLM_MATERIAL);
	glEnable(GL_COLOR_MATERIAL);
}

void refreshCamera(void)
{
	switch(cameraType)
	{
		case PERSPECTIVE:
			setPerspectiveView();
			break;
		
		case ORTOGRAPHIC:
			setOrtographicView();
			break;

		case THIRDPERSON:
			setThirdPersonView();
			break;

		case ANOTHERPLANE:
			setAnotherPlaneView();
			break;
	}

}

void setThirdPersonView(void)
{
	float upAxis;
	if((rotateAngle[X]>90 && rotateAngle[X]<=270) || (rotateAngle[X]<-90 && rotateAngle[X]>=-270))
		upAxis=-1;
	else
		upAxis=1;

	glViewport (0, 0, (GLsizei) viewPortWidth, (GLsizei) viewPortHeight); 
	glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(60.0, (GLfloat) viewPortWidth/(GLfloat) viewPortHeight, 0.1, 2000.0);
	glTranslatef(0,0,-2);
	gluLookAt(planePosition[X],planePosition[Y],planePosition[Z],planePosition[X]+pointVector[X],planePosition[Y]+pointVector[Y], planePosition[Z]+pointVector[Z],0,upAxis,0);
	glMatrixMode (GL_MODELVIEW);
}

void setAnotherPlaneView(void)
{
	glViewport (0, 0, (GLsizei) viewPortWidth, (GLsizei) viewPortHeight); 
	glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(60.0, (GLfloat) viewPortWidth/(GLfloat) viewPortHeight, 0.1, 2000.0);
	glTranslatef(0,0,-10);
	gluLookAt(planePosition[X],planePosition[Y],planePosition[Z],planePosition[X],planePosition[Y]-1, planePosition[Z],0,0,1); //Camera acompanha o aviao
	//gluLookAt(planePosition[X],planePosition[Y],planePosition[Z],planePosition[X],planePosition[Y]-1, planePosition[Z],pointVector[X],0,pointVector[Z]); //Camera acompanha o aviao e gira com ele
	glMatrixMode (GL_MODELVIEW);
}

void setPerspectiveView(void)
{
	float upAxis;
	if((rotateAngle[X]>90 && rotateAngle[X]<=270) || (rotateAngle[X]<-90 && rotateAngle[X]>=-270))
		upAxis=-1;
	else
		upAxis=1;

	glViewport (0, 0, (GLsizei) viewPortWidth, (GLsizei) viewPortHeight); 
	glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(60.0, (GLfloat) viewPortWidth/(GLfloat) viewPortHeight, 0.1, 2000.0);
	glTranslatef(0,0,1);
	//glRotatef(rotateAngle[Z],pointVector[X],pointVector[Y],pointVector[Z]); //Tentativa de fazer a camera girar
	//gluLookAt(planePosition[X],planePosition[Y],planePosition[Z],planePosition[X]+pointVector[X],planePosition[Y]+pointVector[Y], planePosition[Z]+pointVector[Z],upVector[X],upVector[Y],upVector[Z]); //Tentativa de fazer a camera girar
	gluLookAt(planePosition[X],planePosition[Y],planePosition[Z],planePosition[X]+pointVector[X],planePosition[Y]+pointVector[Y], planePosition[Z]+pointVector[Z],0,upAxis,0);
	glMatrixMode (GL_MODELVIEW);
}

void setOrtographicView(void)

{
	float orthoPoints;
	glViewport (0, 0, (GLsizei) viewPortWidth, (GLsizei) viewPortHeight); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	orthoPoints = MAXHEIGHT/2;
	glOrtho(-orthoPoints,orthoPoints,-orthoPoints,orthoPoints,MAXHEIGHT*30/100,MAXHEIGHT+1);
	gluLookAt (0,MAXHEIGHT,0,0,0,0,1,0,1);
	glMatrixMode (GL_MODELVIEW);
}