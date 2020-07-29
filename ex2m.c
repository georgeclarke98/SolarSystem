/*
==========================================================================
File:        ex2.c (skeleton)
Version:     5, 19/12/2017
Author:     Toby Howard
==========================================================================
*/

/* The following ratios are not to scale: */
/* Moon orbit : planet orbit */
/* Orbit radius : body radius */
/* Sun radius : planet radius */

#ifdef MACOS
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BODIES 25
#define TOP_VIEW 1
#define ECLIPTIC_VIEW 2
#define SHIP_VIEW 3
#define EARTH_VIEW 4
#define FLY_VIEW 5
#define PI 3.141593
#define DEG_TO_RAD 0.01745329
#define ORBIT_POLY_SIDES 60
#define TIME_STEP 0.5   /* days per frame */
#define FLY_SPEED  10000000
#define TURN_ANGLE 4.0

typedef struct {
  char    name[25];       /* name */
  GLfloat r,g,b;          /* colour */
  GLfloat orbital_radius; /* distance to parent body (km) */
  GLfloat orbital_tilt;   /* angle of orbit wrt ecliptic (deg) */
  GLfloat orbital_period; /* time taken to orbit (days) */
  GLfloat radius;         /* radius of body (km) */
  GLfloat axis_tilt;      /* tilt of axis wrt body's orbital plane (deg) */
  GLfloat rot_period;     /* body's period of rotation (days) */
  GLint   orbits_body;    /* identifier of parent body */
  GLfloat spin;           /* current spin value (deg) */
  GLfloat orbit;          /* current orbit value (deg) */
 } body;

body  bodies[MAX_BODIES];
int   numBodies, current_view, draw_orbits, draw_labels, draw_starfield;

int   axis = 0, starsDrawn = 0;
//booleans for turning the axis on/off and if the stars are drawn
float  date;

GLint width= 1400, height= 1000;

GLfloat starsX[1000];
GLfloat starsY[1000];
GLfloat starsZ[1000];
//arrays to store the locations of the 1000 stars

GLdouble lat,     lon;              /* View angles (degrees)    */
GLdouble mlat,    mlon;             /* Mouse look offset angles */
GLfloat  eyex,    eyey,    eyez;    /* Eye point                */
GLfloat  centerx, centery, centerz; /* Look point               */
GLfloat  upx,     upy,     upz;
//variables for moving

/*****************************/

float myRand (void)
{
  /* return a random float in the range [0,1] */

  return (float) rand() / RAND_MAX;
}

/*****************************/

void drawStarfield (void)
{
  glBegin (GL_POINTS);
  glColor3f(1.0,1.0,1.0);
  //while colour for the stars

  int index;
  if (starsDrawn == 0) {
    for(index = 0; index < 1000; index++) {
      starsX[index] = (myRand()*600000000)-300000000;
      starsY[index] = (myRand()*600000000)-300000000;
      starsZ[index] = (myRand()*600000000)-300000000;
      //store each star position within the cube
    } //for
  } //if

  for(index = 0; index < 1000; index++) {
    glVertex3f(starsX[index], starsY[index], starsZ[index]);
    //draw each star from positions in arrays
  } //for

  starsDrawn = 1;
  glEnd();
}

/*****************************/

void readSystem(void)
{
  /* reads in the description of the solar system */

  FILE *f;
  int i;

  f= fopen("sys", "r");
  if (f == NULL) {
     printf("ex2.c: Couldn't open 'sys'\n");
     printf("To get this file, use the following command:\n");
     printf("  cp /opt/info/courses/COMP27112/ex2/sys .\n");
     exit(0);
  }
  fscanf(f, "%d", &numBodies);
  for (i= 0; i < numBodies; i++)
  {
    fscanf(f, "%s %f %f %f %f %f %f %f %f %f %d",
      bodies[i].name,
      &bodies[i].r, &bodies[i].g, &bodies[i].b,
      &bodies[i].orbital_radius,
      &bodies[i].orbital_tilt,
      &bodies[i].orbital_period,
      &bodies[i].radius,
      &bodies[i].axis_tilt,
      &bodies[i].rot_period,
      &bodies[i].orbits_body);

    /* Initialise the body's state */
    bodies[i].spin= 0.0;
    bodies[i].orbit= myRand() * 360.0; /* Start each body's orbit at a
                                          random angle */
    bodies[i].radius*= 1000.0; /* Magnify the radii to make them visible */
  }
  fclose(f);
}

/*****************************/

void drawString (void *font, float x, float y, char *str)
{ /* Displays the string "str" at (x,y,0), using font "font" */

  /* This is for you to complete. */

}

/*****************************/

void calculate_lookpoint(void) { /* Given an eyepoint and latitude and longitude angles, will
     compute a look point one unit away */

  GLfloat dirx, diry, dirz;

  if ((lat + mlat) <= 90 && (lat + mlat) >= -90) {
    dirx = cos((lat + mlat) * DEG_TO_RAD) * sin((lon + mlon) * DEG_TO_RAD) * 1000000000;
    diry = sin((lat + mlat) * DEG_TO_RAD) * 1000000000;
    dirz = cos((lat + mlat) * DEG_TO_RAD) * cos((lon + mlon) * DEG_TO_RAD) * 1000000000;
    //calculate directions

    centerx = eyex + dirx;
    centery = eyey + diry;
    centerz = eyez + dirz;
    //calculate centres from eye and direction
  }
} // calculate_lookpoint()

void setView (void) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  switch (current_view) {
  case TOP_VIEW:
    gluLookAt(0.0, 600000000.0, 0.0,  // camera position
            0.0, 0.0, 0.0,    // point to look at
            0.0, 0.0, 1.0 );  // "upright" vector
            //look at the centre from up on the y-axis
    break;
  case ECLIPTIC_VIEW:
  gluLookAt(0.0, 0.0, 600000000.0,  // camera position
            0.0, 0.0, 0.0,    // point to look at
            0.0, 1.0, 0.0 );  // "upright" vector
            //look at the centre from along the z-axis to see orbits
    break;
  case SHIP_VIEW:
    gluLookAt(300000000.0, 150000000.0, 300000000.0,  // camera position
              0.0, 0.0, 0.0,    // point to look at
              0.0, 1.0, 0.0 );  // "upright" vector
              //look at the centre from a spaceship view
    break;
  case EARTH_VIEW:
  gluLookAt(((bodies[3].orbital_radius + 25000000) * cos(bodies[3].orbital_tilt * DEG_TO_RAD))
            * sin(bodies[3].orbit * DEG_TO_RAD),
            bodies[3].radius + 5000000,
            ((bodies[3].orbital_radius + 25000000) * cos(bodies[3].orbital_tilt * DEG_TO_RAD))
            * cos(bodies[3].orbit * DEG_TO_RAD),  // camera position
            0.0, 0.0, 0.0,    // point to look at
            0.0, 1.0, 0.0 );  // "upright" vector
            //look at t  if(moving == 0) {he centre from a spaceship view
    break;
    case FLY_VIEW:
    calculate_lookpoint(); /* Compute the centre of interest   */
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
      break;
  }
}

/*****************************/

void menu (int menuentry) {
  switch (menuentry) {
  case 1: current_view= TOP_VIEW;
          break;
  case 2: current_view= ECLIPTIC_VIEW;
          break;
  case 3: current_view= SHIP_VIEW;
          break;
  case 4: current_view= EARTH_VIEW;
          break;
  case 5: current_view= FLY_VIEW;
          break;
  case 6: draw_labels= !draw_labels;
          break;
  case 7: draw_orbits= !draw_orbits;
          break;
  case 8: draw_starfield= !draw_starfield;
          break;
  case 9: exit(0);
  }
}

/*****************************/

void init(void)
{
  /* Set initial view parameters */
  /* Define background colour */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  eyex = 300000000.0;
  eyey = 150000000.0;
  eyez = 300000000.0;
  upx = 0.0;
  upy = 1.0;
  upz = 0.0;
  lat = -30.0;
  lon = 220.0;
  mlat = 0.0;
  mlon = 0.0;

  glutCreateMenu (menu);
  glutAddMenuEntry ("Top view", 1);
  glutAddMenuEntry ("Ecliptic view", 2);
  glutAddMenuEntry ("Spaceship view", 3);
  glutAddMenuEntry ("Earth view", 4);
  glutAddMenuEntry ("Fly view", 5);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Toggle labels", 6);
  glutAddMenuEntry ("Toggle orbits", 7);
  glutAddMenuEntry ("Toggle starfield", 8);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Quit", 9);
  glutAttachMenu (GLUT_RIGHT_BUTTON);

  current_view= TOP_VIEW;
  draw_labels= 1;
  draw_orbits= 1;
  draw_starfield= 1;
}

/*****************************/

void animate(void)
{
  int i;

    date+= TIME_STEP;

    for (i= 0; i < numBodies; i++)  {
      bodies[i].spin += 360.0 * TIME_STEP / bodies[i].rot_period;
      bodies[i].orbit += 360.0 * TIME_STEP / bodies[i].orbital_period;
      glutPostRedisplay();
    }
}

/*****************************/

void drawOrbit (int n)
{
  glBegin(GL_LINE_LOOP);
  int i;
  for(i = 0; i < ORBIT_POLY_SIDES; i++) {
    glVertex3f(cos(i * (360 / ORBIT_POLY_SIDES) * DEG_TO_RAD) * bodies[n].orbital_radius,
               0,
               sin(i * (360 / ORBIT_POLY_SIDES) * DEG_TO_RAD) * bodies[n].orbital_radius);
    //draw each orbit for the number of points
  }
  glEnd();
}

/*****************************/

void drawLabel(int n)
{ /* Draws the name of body "n" */

    /* This is for you to complete. */
}

/*****************************/

void drawBody(int n)
{
  if(bodies[n].orbits_body == 0) {
    glColor3f(bodies[n].r,bodies[n].g,bodies[n].b);
    //colour of the body

    glRotatef(bodies[n].orbital_tilt, 0.0, 0.0, 1.0);
    //tilt the body by the orbital tilt in the z axis

    if (draw_orbits) drawOrbit(n);
    //draw the orbits if turned on

    glTranslatef(bodies[n].orbital_radius * cos(bodies[n].orbital_tilt * DEG_TO_RAD)
                 * sin(bodies[n].orbit * DEG_TO_RAD),
                 0.0,
                 bodies[n].orbital_radius * cos(bodies[n].orbital_tilt* DEG_TO_RAD)
                 * cos(bodies[n].orbit * DEG_TO_RAD));
    //translate around the orbital path


    glRotatef(bodies[n].axis_tilt, 1.0, 0.0, 0.0);
    //rotate by axis tilt

    glRotatef(bodies[n].spin, 0.0, 1.0, 0.0);
    //spin the planet

    glRotatef(90, 1.0, 0.0, 0.0);
    //rotate by 90 degrees in x axis to get the body orientation correcr

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, bodies[n].radius + (bodies[n].radius/2));
    glVertex3f(0.0f, 0.0f, -(bodies[n].radius + (bodies[n].radius/2)));
    glVertex3f(0.0, 4.0f, 0.0f);
    glVertex3f(-1.0, 3.0f, 0.0f);
    glEnd();
    //draw the axis line

    glutWireSphere(bodies[n].radius, 20, 20);
    //draw the body
  } //if
  else {
    glColor3f(bodies[n].r,bodies[n].g,bodies[n].b);
    //colour of the body

    glRotatef(bodies[bodies[n].orbits_body].orbital_tilt, 0.0, 0.0, 1.0);
    //tilt the body by the orbital tilt in the z axis

    glTranslatef(bodies[bodies[n].orbits_body].orbital_radius *
      cos(bodies[bodies[n].orbits_body].orbital_tilt * DEG_TO_RAD) *
      sin(bodies[bodies[n].orbits_body].orbit * DEG_TO_RAD),
      0.0,
      bodies[bodies[n].orbits_body].orbital_radius *
      cos(bodies[bodies[n].orbits_body].orbital_tilt* DEG_TO_RAD) *
      cos(bodies[bodies[n].orbits_body].orbit * DEG_TO_RAD));
    //translate to the centre of the parent body to draw moons orbits

    if (draw_orbits) drawOrbit(n);
    //draw the orbits if turned on

    glTranslatef(bodies[n].orbital_radius  * sin(bodies[n].orbit * DEG_TO_RAD),
                 0.0,
                 bodies[n].orbital_radius  * cos(bodies[n].orbit * DEG_TO_RAD));
    //translate around the orbital path

    glRotatef(bodies[n].spin, 0.0, 1.0, 0.0);
    //spin the planet  if(moving == 0) {

    glRotatef(90, 1.0, 0.0, 0.0);
    //rotate by 90 degrees in x axis to get the body orientation correcr

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, bodies[n].radius + (bodies[n].radius/2));
    glVertex3f(0.0f, 0.0f, -(bodies[n].radius + (bodies[n].radius/2)));
    glVertex3f(0.0, 4.0f, 0.0f);
    glVertex3f(-1.0, 3.0f, 0.0f);
    glEnd();
    //draw the axis line

    glutWireSphere(bodies[n].radius, 20, 20);
    //draw the body
  }
}

/*****************************/

void display(void)
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT);

  /* set the camera */
  setView();

  if(axis == 1) {
    glColor3f(1.0,0.0,0.0);
    glBegin(GL_LINES);
    glVertex3f(60000000.0, 0.0f, 0.0f);

    glVertex3f(4.0, 0.0f, 0.0f);
    glVertex3f(3.0, -1.0f, 0.0f);
    glEnd();
    glFlush();

    glColor3f(0.0,1.0,0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 60000000.0f, 0.0f);

    glVertex3f(0.0, 4.0f, 0.0f);
    glVertex3f(-1.0, 3.0f, 0.0f);
    glEnd();
    glFlush();

    glColor3f(0.0,0.0,1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0f ,60000000.0f );

    glVertex3f(0.0, 0.0f ,4.0f );
    glVertex3f(0.0, -1.0f ,3.0f );
    glEnd();
    glFlush();
  }

  if (draw_starfield) drawStarfield();

  for (i= 0; i < numBodies; i++)
  {
    glPushMatrix();
      drawBody (i);
    glPopMatrix();
  }

  glutSwapBuffers();
}

/*****************************/

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (48.0, (GLfloat)w/(GLfloat)h,  10000.0, 800000000.0);
}

/*****************************/
void mouse_motion(int x, int y) {

  if (x >= 0 && x <= width && y >= 0 && y <= height) {
    mlon = 50 - (100 * ((double) x / width));
    mlat = 50 - (100 * ((double) y / height));
    //calculate mlon and mlat
    //lon = lon + mlon;
    //if (-90 <= lat + mlat <= 90)
    //  lat = lat + mlat;
  }
} // mouse_motion()

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 27:  /* Escape key */
      exit(0);
      break;
    case 44 : //comma
      eyex = eyex + (sin(((lon + mlon) + 90) *  DEG_TO_RAD) * FLY_SPEED);
      eyez = eyez + (cos(((lon + mlon) + 90) * DEG_TO_RAD) * FLY_SPEED);
      break;
    case 46 : //full stop
      eyex = eyex + (sin(((lon + mlon) - 90) *  DEG_TO_RAD) * FLY_SPEED);
      eyez = eyez + (cos(((lon + mlon) - 90) * DEG_TO_RAD) * FLY_SPEED);
      break;
    case 'a':
      if(axis == 1)
        axis = 0;
      else
        axis = 1;
      break;
   }
} // keyboard()

//////////////////////////////////////////////

void cursor_keys(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT :
      lon = lon + TURN_ANGLE;
      break;
    case GLUT_KEY_RIGHT :
      lon = lon - TURN_ANGLE;
      break;
    case GLUT_KEY_PAGE_UP :
      lat = lat + TURN_ANGLE;
      break;
    case GLUT_KEY_PAGE_DOWN :
      lat = lat - TURN_ANGLE;
      break;
    case GLUT_KEY_HOME :
      lat = 0;
      lon = 0;
      break;
    case GLUT_KEY_UP :
      eyex = eyex + (sin((lon + mlon) *  DEG_TO_RAD) * FLY_SPEED);
      eyey = eyey + (sin((lat + mlat) *  DEG_TO_RAD) * FLY_SPEED);
      eyez = eyez + (cos((lon + mlon) * DEG_TO_RAD) * FLY_SPEED);
      break;
    case GLUT_KEY_DOWN :
      eyex = eyex - (sin((lon + mlon) *  DEG_TO_RAD) * FLY_SPEED);
      eyey = eyey - (sin((lat + mlat) *  DEG_TO_RAD) * FLY_SPEED);
      eyez = eyez - (cos((lon + mlon) * DEG_TO_RAD) * FLY_SPEED);
      break;
  } //switch
  //set cursor keys to change lat and lon respectively and to move
} // cursor_keys()

/*****************************/

int main(int argc, char** argv)
{
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize (width, height);
  glutCreateWindow ("COMP27112 Exercise 2");
  init();
  glutDisplayFunc (display);
  glutIdleFunc (animate);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (keyboard);
  glutSpecialFunc (cursor_keys);
  glutPassiveMotionFunc (mouse_motion);
  readSystem();
  glutMainLoop();
  return 0;
}
/* end of ex2.c */
