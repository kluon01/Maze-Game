//---------------------------------------------------------
// Program: project5.cpp
// Purpose: To build a playable maze game from a text file
//          utilizing various wall textures.
// Authors: Karshin Luong, John Gauch
// Date:    4/12/2019
//---------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "libim/im_color.h"

// Global variables 
#define SIZE 64
#define ROTATE 1
#define PLAY 2

// Global camera variables
int xangle = 0;
int yangle = 0;
int zangle = 0;

// Global player variables
float playerx = 1.32;
float playery = 1.38;
float playerz = 0.021;

// Global block variables
float blocks[SIZE*SIZE][7];
float grass_blocks[SIZE*SIZE][7];
float block_xmin = -1.40;
float block_xmax = -1.25;
float block_ymin = 1.40;
float block_ymax = 1.25;
float block_zmin = 0;
float block_zmax = 0.5;
int blockCount = 0;
int grassBlockCount = 0;

// Global maze variables
char maze[SIZE][SIZE];
int rows, cols, start_row, start_col;

// Global texture variables
int xdim0, ydim0, xdim1, ydim1, xdim2, ydim2, xdim3, ydim3, xdim4, ydim4;
unsigned char *brick_texture;
unsigned char *rock_texture;
unsigned char *wood_texture;
unsigned char *grass_texture;
unsigned char *player_texture;

int mode = PLAY;

//---------------------------------------
// Initialize texture image
//---------------------------------------
void init_texture(char *name, unsigned char *&texture, int &xdim, int &ydim)
{
   // Read jpg image
   im_color image;
   image.ReadJpg(name);
   //printf("Reading image %s\n", name);
   xdim = 1; while (xdim < image.R.Xdim) xdim*=2; xdim /=2;
   ydim = 1; while (ydim < image.R.Ydim) ydim*=2; ydim /=2;
   image.Interpolate(xdim, ydim);
   //printf("Interpolating to %d by %d\n", xdim, ydim);

   // Copy image into texture array
   texture = (unsigned char *)malloc((unsigned int)(xdim*ydim*3));
   int index = 0;
   for (int y = 0; y < ydim; y++)
      for (int x = 0; x < xdim; x++)
      {
         texture[index++] = (unsigned char)(image.R.Data2D[y][x]);
         texture[index++] = (unsigned char)(image.G.Data2D[y][x]);
         texture[index++] = (unsigned char)(image.B.Data2D[y][x]);
      }
}

//---------------------------------------
// Function to draw 3D block
//---------------------------------------
void block(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
{
   // Define 8 vertices
   float ax = xmin, ay = ymin, az = zmax;
   float bx = xmax, by = ymin, bz = zmax;
   float cx = xmax, cy = ymax, cz = zmax;
   float dx = xmin, dy = ymax, dz = zmax;

   float ex = xmin, ey = ymin, ez = zmin;
   float fx = xmax, fy = ymin, fz = zmin;
   float gx = xmax, gy = ymax, gz = zmin;
   float hx = xmin, hy = ymax, hz = zmin;

   // Draw 6 faces
   glBegin(GL_POLYGON);  // Max texture coord 1.0
   glTexCoord2f(0.0, 0.0);
   glVertex3f(ax, ay, az);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(bx, by, bz);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(cx, cy, cz);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(dx, dy, dz);
   glEnd();

   glBegin(GL_POLYGON);  // Max texture coord 1.0
   glTexCoord2f(0.0, 0.0);
   glVertex3f(ex, ey, ez);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(ax, ay, az);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(dx, dy, dz);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(hx, hy, hz);
   glEnd();

   glBegin(GL_POLYGON);  // Max texture coord 1.0
   glTexCoord2f(0.0, 0.0);
   glVertex3f(fx, fy, fz);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(ex, ey, ez);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(hx, hy, hz);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(gx, gy, gz);
   glEnd();

   glBegin(GL_POLYGON);  // Max texture coord 1.0
   glTexCoord2f(0.0, 0.0);
   glVertex3f(bx, by, bz);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(fx, fy, fz);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(gx, gy, gz);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(cx, cy, cz);
   glEnd();

   glBegin(GL_POLYGON);  // Max texture coord 3.0
   glTexCoord2f(0.0, 0.0);
   glVertex3f(ax, ay, az);
   glTexCoord2f(2.0, 0.0);
   glVertex3f(ex, ey, ez);
   glTexCoord2f(2.0, 2.0);
   glVertex3f(fx, fy, fz);
   glTexCoord2f(0.0, 2.0);
   glVertex3f(bx, by, bz);
   glEnd();

   glBegin(GL_POLYGON);  // Max texture coord 3.0
   glTexCoord2f(0.0, 0.0);
   glVertex3f(gx, gy, gz);
   glTexCoord2f(3.0, 0.0);
   glVertex3f(cx, cy, cz);
   glTexCoord2f(3.0, 3.0);
   glVertex3f(dx, dy, dz);
   glTexCoord2f(0.0, 3.0);
   glVertex3f(hx, hy, hz);
   glEnd();
}

//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
   // Init view
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
   glEnable(GL_DEPTH_TEST);

   // Read maze.txt file
   ifstream infile; 
   infile.open("maze.txt");
   string line;
   infile >> rows >> cols >> start_row >> start_col;
   cout << rows << " " << cols << endl;
   cout << start_row << " " << start_col << endl;

   // Read line by line and store maze layout in 2D array
   for(int i = 0; i < rows + 1; i++)
   {
      getline(infile,line);
      for(int j = 0; j < cols; j++)
      {
         maze[i][j] = line[j];
      }
   }

   // Store the block locations based on the maze array
   for(int i = 1; i < rows + 1; i++)
   {
      for(int j = 0; j < cols; j++)
      {
         if(maze[i][j] == 'b')
         {
            blocks[blockCount][0] = 0.0; // Identifies texture type
            blocks[blockCount][1] = block_xmin;
            blocks[blockCount][2] = block_ymin;
            blocks[blockCount][3] = block_zmin;
            blocks[blockCount][4] = block_xmax;
            blocks[blockCount][5] = block_ymax;
            blocks[blockCount][6] = block_zmax;
            block_xmax += 0.15;
            block_xmin += 0.15;
            blockCount++;
         }
         else if(maze[i][j] == 'r')
         {  
            blocks[blockCount][0] = 1.0;
            blocks[blockCount][1] = block_xmin;
            blocks[blockCount][2] = block_ymin;
            blocks[blockCount][3] = block_zmin;
            blocks[blockCount][4] = block_xmax;
            blocks[blockCount][5] = block_ymax;
            blocks[blockCount][6] = block_zmax;
            block_xmax += 0.15;
            block_xmin += 0.15;
            blockCount++;
         }
         else if(maze[i][j] == 'w')
         {
            blocks[blockCount][0] = 2.0;
            blocks[blockCount][1] = block_xmin;
            blocks[blockCount][2] = block_ymin;
            blocks[blockCount][3] = block_zmin;
            blocks[blockCount][4] = block_xmax;
            blocks[blockCount][5] = block_ymax;
            blocks[blockCount][6] = block_zmax;
            block_xmax += 0.15;
            block_xmin += 0.15;
            blockCount++;
         }
         else if(maze[i][j] == ' ')
         {
            grass_blocks[grassBlockCount][0] = 3.0;
            grass_blocks[grassBlockCount][1] = block_xmin;
            grass_blocks[grassBlockCount][2] = block_ymin;
            grass_blocks[grassBlockCount][3] = block_zmin;
            grass_blocks[grassBlockCount][4] = block_xmax;
            grass_blocks[grassBlockCount][5] = block_ymax;
            grass_blocks[grassBlockCount][6] = 0.02;
            block_xmax += 0.15;
            block_xmin += 0.15;
            grassBlockCount++;
         }
      }
      block_xmin = -1.40;
      block_xmax = -1.25;
      block_ymin -= 0.15;
      block_ymax -= 0.15;
   }

   // Initialize textures
   glEnable(GL_TEXTURE_2D);
   init_texture((char *)"textures/brick.jpg", brick_texture, xdim0, ydim0);
   init_texture((char *)"textures/rock.jpg", rock_texture, xdim1, ydim1);
   init_texture((char *)"textures/wood.jpg", wood_texture, xdim2, ydim2);
   init_texture((char *)"textures/grass.jpg", grass_texture, xdim3, ydim3);
   init_texture((char *)"textures/player.jpg", player_texture, xdim4, ydim4);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

// Functions to change what textures are being used
void initBrick()
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim0, ydim0, 0, GL_RGB, GL_UNSIGNED_BYTE, brick_texture);
}
void initRock()
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim1, ydim1, 0, GL_RGB, GL_UNSIGNED_BYTE, rock_texture);
}
void initWood()
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim2, ydim2, 0, GL_RGB, GL_UNSIGNED_BYTE, wood_texture);
}
void initGrass()
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim3, ydim3, 0, GL_RGB, GL_UNSIGNED_BYTE, grass_texture);
}
void initPlayer()
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim4, ydim4, 0, GL_RGB, GL_UNSIGNED_BYTE, player_texture);
}

// Check for collisions
bool checkCollision(float x, float y)
{
   // Return true if player block is on grass, else return false
   for(int i = 0; i < grassBlockCount; i++)
   {
      if(x > grass_blocks[i][1] && x < grass_blocks[i][4] && y < grass_blocks[i][2] && y > grass_blocks[i][5])
      {
         return true;
      }
   }
   return false;
}

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
   // Incrementally rotate objects
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glRotatef(xangle, 1.0, 0.0, 0.0);
   glRotatef(yangle, 0.0, 1.0, 0.0);
   glRotatef(zangle, 0.0, 0.0, 1.0);

   // Draw player
   initPlayer();
   block(playerx,playery,playerz,playerx+0.11,playery-0.11,playerz+0.20);

   // Draw grass blocks
   initGrass();
   for(int i = 0; i < grassBlockCount; i++)
   {
      block(grass_blocks[i][1], grass_blocks[i][2], grass_blocks[i][3], grass_blocks[i][4], grass_blocks[i][5], grass_blocks[i][6]);
   }

   // Draw walls of maze 
   for(int i = 0; i < blockCount; i++)
   {
      if(blocks[i][0] == 0.0)
      {
         initBrick();
         block(blocks[i][1], blocks[i][2], blocks[i][3], blocks[i][4], blocks[i][5], blocks[i][6]);
      }
      else if(blocks[i][0] == 1.0)
      {
         initRock();
         block(blocks[i][1], blocks[i][2], blocks[i][3], blocks[i][4], blocks[i][5], blocks[i][6]);
      }
      else if(blocks[i][0] == 2.0)
      {
         initWood();
         block(blocks[i][1], blocks[i][2], blocks[i][3], blocks[i][4], blocks[i][5], blocks[i][6]);
      }
      else
      {
         initBrick();
         block(blocks[i][1], blocks[i][2], blocks[i][3], blocks[i][4], blocks[i][5], blocks[i][6]);
      }
   }
   glFlush();
}

//---------------------------------------
// Keyboard callback for OpenGL
//---------------------------------------
void keyboard(unsigned char key, int x, int y)
{
   if ((key == 'r') || (key == 'R'))
   {
      printf("Type x y z to decrease or X Y Z to increase ROTATION angles.\n");
      mode = ROTATE;
   }
   else if ((key == 't') || (key == 'T'))
   {
      printf("Use w, a, s, d to move fire block around and find the exit.\n");
      mode = PLAY;
   }
   else if ((key == 'f') || (key == 'F'))
   {
      xangle = 0;
      yangle = 0;
      zangle = 0;
   }

   // Handle player movement and check for collision
   if(mode == PLAY)
   {
      if (key == 'w' && checkCollision(playerx,playery+0.15))
      playery += 0.15;
      else if (key == 'a' && checkCollision(playerx-0.15,playery))
      playerx -= 0.15;
      else if (key == 's' && checkCollision(playerx,playery-0.15))
      playery -= 0.15;
      else if (key == 'd' && checkCollision(playerx+0.15,playery))
      playerx += 0.15;
      glutPostRedisplay();
   }

   // Handle rotation
   if (mode == ROTATE)
   {
      if (key == 'x')
	   xangle -= 5;
      else if (key == 'y')
	   yangle -= 5;
      else if (key == 'z')
	   zangle -= 5;
      else if (key == 'X')
	   xangle += 5;
      else if (key == 'Y')
	   yangle += 5;
      else if (key == 'Z')
	   zangle += 5;
      glutPostRedisplay();
   }
}

//---------------------------------------
// Main program
//---------------------------------------
int main(int argc, char *argv[])
{
   // Create OpenGL window
   glutInit(&argc, argv);
   glutInitWindowSize(800, 800);
   glutInitWindowPosition(250, 250);
   glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
   glutCreateWindow("Texture");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   init();
   // Print commands   
   printf("Keyboard commands:\n");
   printf("   't' or 'T' - go to play mode\n");
   printf("   'r' or 'R' - go to rotate mode\n");
   printf("   'f' or 'F' - go to reset camera to top down view\n");
   glutMainLoop();
   return 0;
}
