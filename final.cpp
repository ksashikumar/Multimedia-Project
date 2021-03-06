#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"

#define POINTS_PER_VERTEX        3 
#define TOTAL_FLOATS_IN_TRIANGLE 9
#define MAX                      100

using namespace std;

float           xy_aspect;
float           rotationX = 0.0;
float           rotationY = 0.0;
float           g_rotation;

int             main_window;
int             flag      = 0;
int             str_check = 0;

string          text      = "";
char            name[100] = "./res/";

GLUI_Checkbox  *checkbox;
GLUI_EditText  *edittext;
Mix_Music      *music     = NULL;


void            myGlutDisplay (void);
void            control_cb    (int);
void            myGlutIdle    (void);
void            display       (void); 
void            initialize    (void);

class Model_OBJ
{
  public: 
  	Model_OBJ();			
    float* calculateNormal(float* coord1,float* coord2,float* coord3 );
    int Load(char *filename);	
	  void Draw();					
	  void Release();				
  
	  float* normals;				
    float* Faces_Triangles;
	  float* vertexBuffer;		
	  long TotalConnectedPoints;				
	  long TotalConnectedTriangles;			 
};

 
Model_OBJ::Model_OBJ()
{
	this->TotalConnectedTriangles = 0; 
	this->TotalConnectedPoints = 0;
}
 
float* Model_OBJ::calculateNormal(float *coord1, float *coord2, float *coord3)
{
  float va[3], vb[3], vr[3], val;
  va[0] = coord1[0] - coord2[0];
  va[1] = coord1[1] - coord2[1];
  va[2] = coord1[2] - coord2[2];

  vb[0] = coord1[0] - coord3[0];
  vb[1] = coord1[1] - coord3[1];
  vb[2] = coord1[2] - coord3[2];

  vr[0] = va[1] * vb[2] - vb[1] * va[2];
  vr[1] = vb[0] * va[2] - va[0] * vb[2];
  vr[2] = va[0] * vb[1] - vb[0] * va[1];

  val = sqrt(vr[0]*vr[0] + vr[1]*vr[1] + vr[2]*vr[2]);
 
	float *norm;  
  norm = (float*) malloc(sizeof(float)*3);
	norm[0] = vr[0]/val;
	norm[1] = vr[1]/val;
	norm[2] = vr[2]/val;
 
 
	return norm;
}
 
 
int Model_OBJ::Load(char* filename)
{
  string line;
  ifstream objFile (filename);	

  if (objFile.is_open())													
	{
	  objFile.seekg (0, ios::end);									 
		long fileSize = objFile.tellg();							
		objFile.seekg (0, ios::beg);									
 
		vertexBuffer = (float*) malloc (fileSize);		
		Faces_Triangles = (float*) malloc(fileSize*sizeof(float));			
		normals  = (float*) malloc(fileSize*sizeof(float));				
 
		int triangle_index = 0;												
		int normal_index = 0;												
 
		while (! objFile.eof() )										
		{		
			getline (objFile,line);											
 
			if (line.c_str()[0] == 'v')									
			{
				line[0] = ' ';												
 
				sscanf(line.c_str(),"%f %f %f ",							
					&vertexBuffer[TotalConnectedPoints],
					&vertexBuffer[TotalConnectedPoints+1], 
					&vertexBuffer[TotalConnectedPoints+2]);
 
				TotalConnectedPoints += POINTS_PER_VERTEX;					
			}
			if (line.c_str()[0] == 'f')										
			{
		    	line[0] = ' ';												
 
				int vertexNumber[4] = { 0, 0, 0 };
                sscanf(line.c_str(),"%i%i%i",								
					&vertexNumber[0],										
					&vertexNumber[1],										
					&vertexNumber[2] );										
 
				vertexNumber[0] -= 1;										
				vertexNumber[1] -= 1;										
				vertexNumber[2] -= 1;										
				int tCounter = 0;
				for(int i = 0; i < POINTS_PER_VERTEX; i++)					
				{
					Faces_Triangles[triangle_index + tCounter   ] = vertexBuffer[3*vertexNumber[i] ];
					Faces_Triangles[triangle_index + tCounter +1 ] = vertexBuffer[3*vertexNumber[i]+1 ];
					Faces_Triangles[triangle_index + tCounter +2 ] = vertexBuffer[3*vertexNumber[i]+2 ];
					tCounter += POINTS_PER_VERTEX;
				}
  			float coord1[3] = { Faces_Triangles[triangle_index], Faces_Triangles[triangle_index+1],Faces_Triangles[triangle_index+2]};
				float coord2[3] = {Faces_Triangles[triangle_index+3],Faces_Triangles[triangle_index+4],Faces_Triangles[triangle_index+5]};
				float coord3[3] = {Faces_Triangles[triangle_index+6],Faces_Triangles[triangle_index+7],Faces_Triangles[triangle_index+8]};
				float *norm = this->calculateNormal( coord1, coord2, coord3 );
 
				tCounter = 0;
				for(int i = 0; i < POINTS_PER_VERTEX; i++)
				{
					normals[normal_index + tCounter ] = norm[0];
					normals[normal_index + tCounter +1] = norm[1];
					normals[normal_index + tCounter +2] = norm[2];
					tCounter += POINTS_PER_VERTEX;
				}
 
				triangle_index += TOTAL_FLOATS_IN_TRIANGLE;
				normal_index += TOTAL_FLOATS_IN_TRIANGLE;
				TotalConnectedTriangles += TOTAL_FLOATS_IN_TRIANGLE;			
			}	
		}
		objFile.close();														
	}
	else 
	{
		cout << "Unable to open file";								
	}
	return 0;
}
 
void Model_OBJ::Release()
{
	free(this->Faces_Triangles);
	free(this->normals);
	free(this->vertexBuffer);
}
 
void Model_OBJ::Draw()
{
 	glEnableClientState(GL_VERTEX_ARRAY);						
 	glEnableClientState(GL_NORMAL_ARRAY);						
	glVertexPointer(3,GL_FLOAT,	0,Faces_Triangles);				
	glNormalPointer(GL_FLOAT, 0, normals);						
	glDrawArrays(GL_TRIANGLES, 0, TotalConnectedTriangles);		
	glDisableClientState(GL_VERTEX_ARRAY);						
	glDisableClientState(GL_NORMAL_ARRAY);						
}


Model_OBJ obj[MAX];

void control_cb(int control)
{

  char str[200];
  int t;
  printf("text: %s\n", edittext->get_text());
  Mix_PlayMusic( music, -1 );

  for(int i = 0; i < text.length(); i++)
  {
    for(int j = 0; j <= strlen(name); j++)
      str[j] = name[j];

    t = strlen(str);
    str[t++] = text[i];
    str[t++] = '.';
    str[t++] = 'o';
    str[t++] = 'b';
    str[t++] = 'j';
    str[t++] = '\0';  
   	obj[i].Load(str);
  }
  str_check = 1;
  
}


void myGlutIdle(void)
{
  if (glutGetWindow() != main_window) 
    glutSetWindow(main_window);  

  glutPostRedisplay();
}

void display(void) 
{
  static int t = 0, count = 0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glLoadIdentity();
 	gluLookAt( 0,1,4, 0,0,0, 0,1,0);
 	glPushMatrix();

  if(count <= text.length())
  {
   	glRotatef(g_rotation,0,1,0);
    glRotatef(90,0,1,0);

    if(str_check != 0)
      flag++;

    g_rotation++;

    if(flag < 150)
	    obj[t].Draw();
    else
    {
      obj[t].Release();
      t++;
      flag = 0;
      count++;
    }
  }
  else
    myGlutDisplay();

  glFlush();  
  glPopMatrix();
	glutSwapBuffers();
}
 
 
void initialize(void) 
{
  SDL_Init( SDL_INIT_EVERYTHING );
  Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 );
  music = Mix_LoadMUS( "./res/beat.wav" );

  glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, 640, 480);
	GLfloat aspect = (GLfloat) 640 / 480;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
	gluPerspective(45, aspect, 1.0f, 500.0f);
  glMatrixMode(GL_MODELVIEW);
  glShadeModel( GL_SMOOTH );
  glClearColor( 0.0f, 0.1f, 0.0f, 0.5f );
  glClearDepth( 1.0f );
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LEQUAL );
  glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
 
  GLfloat amb_light[] = { 0.1, 0.1, 0.1, 1.0 };
  GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
  GLfloat specular[] = { 0.7, 0.7, 0.3, 1 };
  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
  glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
  glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
  glEnable( GL_LIGHT0 );
  glEnable( GL_COLOR_MATERIAL );
  glShadeModel( GL_SMOOTH );
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0); 
}


void myGlutDisplay(void)
{
  static int check = 0;
  glClearColor( .9f, .9f, .9f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum( -xy_aspect*.08, xy_aspect*.08, -.08, .08, .1, 15.0 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( 0.0f, 0.0f, -1.6f );
  glRotatef( rotationY, 0.0, 1.0, 0.0 );
  glRotatef( rotationX, 1.0, 0.0, 0.0 );


  glDisable( GL_LIGHTING ); 
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0.0, 100.0, 0.0, 100.0  );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glColor3ub( 0, 0, 0 );
  glRasterPos2i( 10, 50 );

  check++;
  char temp[15] = "Entered Text: ";  
  if(check < 80)
  {
    for(int i = 0; i < strlen(temp); i++)
      glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, temp[i] );

    for(int i = 0; i < text.length(); i++)
      glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, text[i] );
  }
  else
  {
    char temp2[150] = "Multimedia Systems Project by: Alamelu, Chandraa, Sashi, Selva";

    for(int i = 0; i < strlen(temp2); i++)
      glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, temp2[i] );
    Mix_HaltMusic();
  }
  
  glEnable( GL_LIGHTING );
  glutSwapBuffers(); 
}

int main(int argc, char* argv[])
{

  glutInit(&argc, argv);
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition(50, 50);
  glutInitWindowSize(750, 480);
 
  main_window = glutCreateWindow( "Display Window" );
  glutDisplayFunc(display);
	glutIdleFunc(display);							

  GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
  GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
  GLfloat light0_position[] = {1.0f, 1.0f, 1.0f, 0.0f};

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

  glEnable(GL_DEPTH_TEST);

  GLUI *glui = GLUI_Master.create_glui("Enter Text", 0, 820, 50); 
  new GLUI_Separator(glui);
  edittext = new GLUI_EditText(glui, "Text:", text, 3, control_cb);
  new GLUI_Button(glui, "Quit", 0,(GLUI_Update_CB)exit);
 
  glui->set_main_gfx_window(main_window);
  initialize();
  GLUI_Master.set_glutIdleFunc(myGlutIdle);


  glutMainLoop();
 
  return EXIT_SUCCESS;
}
