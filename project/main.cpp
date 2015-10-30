/*
 * Robert Werthman
 * CSCI 5229
 *
 *
 * Sources:
 * https://stackoverflow.com/questions/26536570/how-do-i-texture-a-cylinder-in-opengl-created-with-triangle-strip
 * - How to add textures to the cylinder
 *
 * http://cs.lmu.edu/~ray/notes/flightsimulator/
 *
 * http://learnopengl.com/#!Getting-started/Camera
 *
 * https://code.google.com/p/gords-flight-sim/source/browse/trunk/camera.cpp?spec=svn4&r=4
 *
 * Tasks To Complete
 * -------------------
 * 1. Have helicopter automatically return to 0 for pitching, yawing, rolling, strafing, flying.
 * 2. Set limits for the angles of the helicopter and the min height.
 *
 * Known Bugs
 * -------------------
 * 1. Rotors don't rotate now.
 *
 * Resolved Bugs
 * ----------------------
 * 1. I have to use glrotate on the helicopter in order to use glmultmatrix otherwise it faces in the wrong direction.
 * 	+++++The way I had the matrix used in glmultmatrix was not an identity matrix so it was inverting the look of the helicopter.
 *
 * 2. I can't just use glrotate to move the helicopter around.
 * 	+++++glrotate is used to rotate the object like it was really pitching, rolling, and yawing.  This shows more realistic
 * 	flight.
 *
 * 3. After pitching, yawing, rolling, strafing, and flying going back to 0 doesn't get me to my original position.
 * 	+++++This is because I add doubles that change to the these variables.
 *
 * 4. Can't look at helicopter from above with camera.
 * 	+++++I needed to add the height to all dimensions not just the y value.
 *
 */

#include "CSCIx229.h"
#include "Vector.h"


int axes=1;       //  Display axes

int yaw=0;
int pitch=0;
int roll=0;
int strafe=0;
int fly=0;

int fov=55;       //  Field of view (for perspective)
int light=1;      //  Lighting
double asp=1;     //  Aspect ratio
double dim=5.0;   //  Size of world
// Light values
int distance  =   5;  // Light distance
int smooth    =   1;  // Smooth/Flat shading
int emission  =   0;  // Emission intensity (%)
int ambient   =  30;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shinyvec[1];    // Shininess (value)
int zh        =  0;  // Light azimuth
float ylight  =   0;  // Elevation of light

double cameraRight[3];
double cameraUp[3];
double cameraFront[3];

double littleBirdPosition[3];

unsigned int littlebird[10];

int bladeRotation;

Vector* directionVec = new Vector(1,0,0);
Vector* upVec = new Vector(0,1,0);
Vector* rightVec = new Vector(0,0,1);

void init(){

	cameraFront[0] = 1; cameraFront[1] = 0; cameraFront[2] = 0;
	cameraUp[0] =    0; cameraUp[1] =    1; cameraUp[2] =    0;
	cameraRight[0] = 0; cameraRight[1] = 0; cameraRight[2] = 1;
	littleBirdPosition[0] = 0; littleBirdPosition[1] = 0; littleBirdPosition[2] = 0;
	bladeRotation = 0;
}

void HelicopterPitch(int angle){
	pitch += angle;
	pitch %= 360;
	//
	directionVec->x = directionVec->x*Cos(angle)+upVec->x*Sin(angle);
	directionVec->y = directionVec->y*Cos(angle)+upVec->y*Sin(angle);
	directionVec->z = directionVec->z*Cos(angle)+upVec->z*Sin(angle);
	// Normalize the direction vector
	directionVec->normalize();
	// Calculate up vector with cross product of right and direction vectors
	upVec->crossProduct(rightVec, directionVec);
	// Normalize the up vector
	upVec->normalize();
}

void HelicopterRoll(int angle){
	roll += angle;
	roll %= 360;
	//
	rightVec->x = rightVec->x*Cos(angle)+upVec->x*Sin(angle);
	rightVec->y = rightVec->y*Cos(angle)+upVec->y*Sin(angle);
	rightVec->z = rightVec->z*Cos(angle)+upVec->z*Sin(angle);
	// Normalize the right vector
	rightVec->normalize();
	// Calculate the up vector with cross product of right and direction vectors
	upVec->crossProduct(rightVec, directionVec);
	// Normalize the up vector
	upVec->normalize();
}

void HelicopterYaw(int angle){
	yaw += angle;
	yaw %= 360;
	rightVec->x = rightVec->x*Cos(angle)+directionVec->x*Sin(angle);
	rightVec->y = rightVec->y*Cos(angle)+directionVec->y*Sin(angle);
	rightVec->z = rightVec->z*Cos(angle)+directionVec->z*Sin(angle);
	// Normalize the right vector
	rightVec->normalize();
	// Caluclate the direction vector with the cross product of the up and right vectors
	directionVec->crossProduct(upVec,rightVec);
	// Normalize the direciton vector
	directionVec->normalize();
}

void HelicopterFly(int distance){
	fly += distance;
	littleBirdPosition[0] += upVec->x*distance;
	littleBirdPosition[1] += upVec->y*distance;
	littleBirdPosition[2] += upVec->z*distance;
}

void HelicopterStrafe(int distance){
	strafe += distance;
	littleBirdPosition[0] += rightVec->x*distance;
	littleBirdPosition[1] += rightVec->y*distance;
	littleBirdPosition[2] += rightVec->z*distance;
}

/*
 *  Draw vertex in polar coordinates
 */
static void Vertex(int th,int ph, double rep)
{
   double x = Sin(th)*Cos(ph);
   double y =  Cos(th)*Cos(ph);
   double z =          Sin(ph);
   glNormal3d(x,y,z);
   glTexCoord2d(rep*th/360.0,rep*ph/180.0+.5);
   glVertex3d(x,y,z);
}

/*
*  Draw a sphere
* 	with color red, green, blue
*/
void sphere(double red, double green, double blue, double rep)
{
	int th,ph;
	float color[] = {red,green,blue,1.0};
	float Emission[]  = {0.0,0.0,0.01*emission,1.0};


	glColor3f(red, green, blue);
	glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);
	glMaterialfv(GL_FRONT,GL_SPECULAR,color);
	glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

	for (ph=-90;ph<90;ph+=5)
	{
		glBegin(GL_QUAD_STRIP);
		for (th=0;th<=360;th+=5)
		{
			Vertex(th,ph, rep);
			Vertex(th,ph+5, rep);
		}
		glEnd();
	}
}

/*
 * Draw a triangle
 * 	with color red, green, blue
 * 	and texture repetitions rep
 */
void triangle(double red, double green, double blue, double rep)
{
	float color[] = {red, green, blue, 1};
	float Emission[]  = {0.0,0.0,0.01*emission,1.0};

	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,color);
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

	glColor3f(red, green, blue);

	// Front
	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0 ,0); glVertex3f(1, 0, 0);
	glTexCoord2f(rep/2,rep); glVertex3f(0, 1, 0);
	glTexCoord2f(rep,0); glVertex3f(0, 0, 0);
	glEnd();

	// Back
	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0 ,0); glVertex3f(0, 0, -1);
	glTexCoord2f(rep/2,rep); glVertex3f(0, 1, -1);
	glTexCoord2f(rep,0); glVertex3f(1, 0, -1);
	glEnd();

	// Adjacent edge
	glBegin(GL_QUADS);
	glNormal3f(0, -1, 0);
	glTexCoord2f(0,0); glVertex3f(0, 0, 0);
	glTexCoord2f(rep,0); glVertex3f(0, 0, -1);
	glTexCoord2f(rep,rep); glVertex3f(1, 0, -1);
	glTexCoord2f(0,rep); glVertex3f(1, 0, 0);
	glEnd();

	// Opposite edge
	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0,0); glVertex3f(0, 0, 0);
	glTexCoord2f(rep,0); glVertex3f(0, 1, 0);
	glTexCoord2f(rep,rep); glVertex3f(0, 1, -1);
	glTexCoord2f(0,rep); glVertex3f(0, 0, -1);
	glEnd();

	// Hypotenuse edge
	glBegin(GL_QUADS);
	glNormal3f(1, 1, 0);
	glTexCoord2f(0,0); glVertex3f(0, 1, 0);
	glTexCoord2f(rep,0); glVertex3f(1, 0, 0);
	glTexCoord2f(rep,rep); glVertex3f(1, 0, -1);
	glTexCoord2f(0,rep); glVertex3f(0, 1, -1);
	glEnd();
}

/*
 *  Draw a cube
 *     with color red, green, blue
 *
 */
void cube(double red, double green, double blue, double rep)
{
	float color[] = {red, green, blue, 1};
	float Emission[]  = {0.0,0.0,0.01*emission,1.0};

	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,color);
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

	glColor3f(red, green, blue);

	//  Cube
	// Front
	glBegin(GL_QUADS);
	glNormal3f(0, 0, +1);
	glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
	glTexCoord2f(rep,0); glVertex3f(+1,-1, 1);
	glTexCoord2f(rep,rep); glVertex3f(+1,+1, 1);
	glTexCoord2f(0,rep); glVertex3f(-1,+1, 1);
	//  Back
	glNormal3f(0, 0, -1);
	glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
	glTexCoord2f(rep,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(rep,rep); glVertex3f(-1,+1,-1);
	glTexCoord2f(0,rep); glVertex3f(+1,+1,-1);
	//  Right
	glNormal3f(+1, 0, 0);
	glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
	glTexCoord2f(rep,0); glVertex3f(+1,-1,-1);
	glTexCoord2f(rep,rep); glVertex3f(+1,+1,-1);
	glTexCoord2f(0,rep); glVertex3f(+1,+1,+1);
	//  Left
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(rep,0); glVertex3f(-1,-1,+1);
	glTexCoord2f(rep,rep); glVertex3f(-1,+1,+1);
	glTexCoord2f(0,rep); glVertex3f(-1,+1,-1);
	//  Top
	glNormal3f(0, 1, 0);
	glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
	glTexCoord2f(rep,0); glVertex3f(+1,+1,+1);
	glTexCoord2f(rep,rep); glVertex3f(+1,+1,-1);
	glTexCoord2f(rep,0); glVertex3f(-1,+1,-1);
	//  Bottom
	glNormal3f(0, -1, 0);
	glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(rep,0); glVertex3f(+1,-1,-1);
	glTexCoord2f(rep,rep); glVertex3f(+1,-1,+1);
	glTexCoord2f(rep,0); glVertex3f(-1,-1,+1);
	//  End
	glEnd();
}

/*
 *  Draw a cylinder
 */
void cylinder(double red, double green, double blue, double rep)
{
	int th;
	float color[] = {red, green, blue, 1};
	float Emission[]  = {0.0,0.0,0.01*emission,1.0};

	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,color);
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

	glColor3f(red, green, blue);

	//  Cap
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0,0,1);
	glTexCoord2f(0.5,0.5);
	glVertex3f(0,0,1);
	for (th=0;th<=360;th+=10){
		glTexCoord2f(rep/2*Cos(th)+0.5,rep/2*Sin(th)+0.5);
		glVertex3f(Cos(th),Sin(th),1);
	}
	glEnd();

	//  Sides
	glBegin(GL_QUAD_STRIP);
	for (th=0;th<=360;th+=15)
	{
		const float tc = th/(float)360;
		glNormal3f(Cos(th),Sin(th),0);
		glTexCoord2f(rep*tc, 0); glVertex3f(Cos(th),Sin(th),1);
		glTexCoord2f(rep*tc, rep); glVertex3f(Cos(th),Sin(th),0);
	}
	glEnd();
}

/*
 * Draw entire helicopter
 *	with blade rotation (br)
 */
void helicopter(double br){
	glPushMatrix();
	glRotated(180,0,1,0);
	glScaled(.5,.5,.5);

	glEnable(GL_TEXTURE_2D);

	/* Helicopter Body */
	glBindTexture(GL_TEXTURE_2D,littlebird[2]);
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glRotatef(90, 1, 0, 0);
	glScalef(1,1.5,1);
	sphere(1,1,1, 1);
	glPopMatrix();

	/* Engine */
	glBindTexture(GL_TEXTURE_2D,littlebird[3]);
	glPushMatrix();
	glTranslatef(1.1, -.2, 0);
	glRotatef(90, 0, 1, 0);
	glScalef(.2, .2, .5);
	cylinder(1, 1, 1, 2);
	glPopMatrix();

	/* Tail Bloom */
	glBindTexture(GL_TEXTURE_2D,littlebird[1]);
	glPushMatrix();
	glTranslatef(.8, .6, 0);
	glRotatef(90, 0, 1, 0);
	glScalef(.2, .2, 3);
	cylinder(1,1,1,2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.5, .06, .05);
	glRotatef(45, 0, 0, 1);
	glScalef(.5, 1, .1);
	triangle(1, 1, 1, 2);
	glPopMatrix();

	/* Tail Fins */
	glBindTexture(GL_TEXTURE_2D,littlebird[1]);
	glPushMatrix();
	glTranslatef(3.5, .6, -.2);
	glRotatef(-45, 0, 0, 1);
	glScalef(1, 1, .1);
	triangle(1, 1, 1, 2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(3.7, 1.4, -.2);
	glRotatef(-90, 1, 0, 0);
	glRotatef(-45, 0, 0, 1);
	glScalef(1,1,.1);
	triangle(1, 1, 1,2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.2, 1.4, -.9);
	glRotatef(-45, 0, 0, 1);
	glScalef(.4, .4, .1);
	triangle(1, 1, 1,2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.2, 1.4, 0.6);
	glRotatef(-45, 0, 0, 1);
	glScalef(.4, .4, .1);
	triangle(1, 1, 1,2);
	glPopMatrix();

	/* Tail Gearbox */
	glBindTexture(GL_TEXTURE_2D,littlebird[7]);
	glPushMatrix();
	glTranslatef(3.6, .6, .3);
	glScalef(.07, .07, .1);
	cube(1, 1, 1,2);
	glPopMatrix();

	/* Tail Rotor Blades */
	glBindTexture(GL_TEXTURE_2D,littlebird[6]);
	glPushMatrix();
	glTranslatef(3.6, .6, .4);
	glRotatef(br + 45, 0, 0, 1);
	glScalef(.4, .05, .08);
	cube(1, 1, 1,2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(3.6, .6, .4);
	glRotatef(br - 45, 0, 0, 1);
	glScalef(.4, .05, .08);
	cube(1, 1, 1,2);
	glPopMatrix();

	/* Main Rotor Mast */
	glBindTexture(GL_TEXTURE_2D,littlebird[0]);
	glPushMatrix();
	glTranslatef(.4, .8, 0);
	glScalef(.8, .3, .3);
	cube(1, 1, 1,2);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D,littlebird[7]);
	glPushMatrix();
	glTranslatef(.2, 1.1, 0);
	glScalef(.1, .18, .1);
	cube(1, 1, 1,2);
	glPopMatrix();

	/* Main Rotor Blades */
	glBindTexture(GL_TEXTURE_2D,littlebird[6]);
	glPushMatrix();
	glTranslatef(.2, 1.3, 0);
	glRotatef(br + 0, 0, 1, 0);
	glScalef(2, .05, .2);
	cube(1, 1, 1,2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(.2, 1.3, 0);
	glRotatef(br + 90, 0, 1, 0);
	glScalef(2, .05, .2);
	cube(1, 1, 1,2);
	glPopMatrix();

	/* Skids */
	/* Left Skid Mounts */
	glBindTexture(GL_TEXTURE_2D,littlebird[4]);
	glPushMatrix();
	glTranslatef(-.5, -.9, .5);
	glRotatef(-30, 1, 0, 0);
	glRotatef(75, 0, 0, 1);
	glScalef(.3, .06, .06);
	cube(1, 1, 1,2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(.5, -.9, .5);
	glRotatef(-30, 1, 0, 0);
	glRotatef(75, 0, 0, 1);
	glScalef(.3, .06, .06);
	cube(1, 1, 1,2);
	glPopMatrix();

	/* Right Skid Mounts */
	glBindTexture(GL_TEXTURE_2D,littlebird[4]);
	glPushMatrix();
	glTranslatef(-.5, -.9, -.5);
	glRotatef(30, 1, 0, 0);
	glRotatef(75, 0, 0, 1);
	glScalef(.3, .06, .06);
	cube(1, 1, 1,2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(.5, -.9, -.5);
	glRotatef(30, 1, 0, 0);
	glRotatef(75, 0, 0, 1);
	glScalef(.3, .06, .06);
	cube(1, 1, 1,2);
	glPopMatrix();

	/* Right Skid */
	glBindTexture(GL_TEXTURE_2D,littlebird[5]);
	glPushMatrix();
	glTranslatef(-.4, -1.15, .65);
	glScalef(1, .06, .06);
	cube(1, 1, 1,3);
	glPopMatrix();

	/* Left Skid */
	glBindTexture(GL_TEXTURE_2D,littlebird[5]);
	glPushMatrix();
	glTranslatef(-.4, -1.15, -.65);
	glScalef(1, .06, .06);
	cube(1, 1, 1,3);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void DrawHelicopterFlight(){
	/*
	double mat[16];
	mat[0] = cameraFront[0];   mat[4] = cameraUp[0];   mat[ 8] = cameraRight[0];   mat[12] = 0;
	mat[1] = cameraFront[1];   mat[5] = cameraUp[1];   mat[ 9] = cameraRight[1];   mat[13] = 0;
	mat[2] = cameraFront[2];   mat[6] = cameraUp[2];   mat[10] = cameraRight[2];   mat[14] = 0;
	mat[3] =              0;   mat[7] =           0;   mat[11] =              0;   mat[15] = 1;
	*/
	double mat[16];
	mat[0] = directionVec->x;   mat[4] = upVec->x;   mat[ 8] = rightVec->x;   mat[12] = 0;
	mat[1] = directionVec->y;   mat[5] = upVec->y;   mat[ 9] = rightVec->y;   mat[13] = 0;
	mat[2] = directionVec->z;   mat[6] = upVec->z;   mat[10] = rightVec->z;   mat[14] = 0;
	mat[3] =               0;   mat[7] =        0;   mat[11] =           0;   mat[15] = 1;

	glPushMatrix();
	glTranslated(littleBirdPosition[0],littleBirdPosition[1],littleBirdPosition[2]);
	glMultMatrixd(mat);
	//glRotated(yaw,0,1,0);
	//glRotated(pitch,1,0,0);
	//glRotated(roll,0,0,1);
	helicopter(bladeRotation);
	glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
	//double vectorLength = 0;
	const double len=2.0;  //  Length of axes
	//  Erase the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//  Enable Z-buffering in OpenGL
	glEnable(GL_DEPTH_TEST);
	// Enable face culling in OpenGL
	glEnable(GL_CULL_FACE);
	//  Undo previous transformations
	glLoadIdentity();

	double behindX = 10*directionVec->x;
	double behindY = 10*directionVec->y;
	double behindZ = 10*directionVec->z;

	double heightX = 2*upVec->x;
	double heightY = 2*upVec->y;
	double heightZ = 2*upVec->z;

	gluLookAt(littleBirdPosition[0]-behindX+heightX,littleBirdPosition[1]-behindY+heightY,littleBirdPosition[2]-behindZ+heightZ,
			littleBirdPosition[0],littleBirdPosition[1],littleBirdPosition[2],
			upVec->x,upVec->y,upVec->z);

	if(light){
		//  Translate intensity to color vectors
		float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
		float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
		float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
		//  Light position
		float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
		//  Draw light position as ball (still no lighting here)
		glColor3f(1,1,1);
		glPushMatrix();
		glTranslatef(Position[0],Position[1],Position[2]);
		glScalef(0.1, 0.1, 0.1);
		sphere(1.0, 1.0, 1.0, 1);
		glPopMatrix();
		//  OpenGL should normalize normal vectors
		glEnable(GL_NORMALIZE);
		//  Enable lighting
		glEnable(GL_LIGHTING);
		//  glColor sets ambient and diffuse color materials
		glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
		//  Enable light 0
		glEnable(GL_LIGHT0);
		//  Set ambient, diffuse, specular components and position of light 0
		glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
		glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
		glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
		glLightfv(GL_LIGHT0,GL_POSITION,Position);
	}else{
		glDisable(GL_LIGHTING);
	}

	bladeRotation += 5;
	bladeRotation %= 360;
	DrawHelicopterFlight();

	//  Draw axes - no lighting from here on
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	if(axes){
		//  Draw axes
		glBegin(GL_LINES);
		glVertex3d(0.0,0.0,0.0);
		glVertex3d(len,0.0,0.0);
		glVertex3d(0.0,0.0,0.0);
		glVertex3d(0.0,len,0.0);
		glVertex3d(0.0,0.0,0.0);
		glVertex3d(0.0,0.0,len);
		glEnd();
		//  Label axes
		glRasterPos3d(len,0.0,0.0);
		Print("X");
		glRasterPos3d(0.0,len,0.0);
		Print("Y");
		glRasterPos3d(0.0,0.0,len);
		Print("Z");
	}
	//  Display parameters
	glWindowPos2i(5,5);
	Print("Roll=%d Yaw=%d Pitch=%d Stafe=%d Fly=%d", roll,
			  yaw, pitch,strafe,fly);
	glWindowPos2i(5,25);
	Print("X=%f Y=%f Z=%f",littleBirdPosition[0],littleBirdPosition[1],littleBirdPosition[2]);
	// Check for any errors that have occurred
	ErrCheck("display");
	//  Render the scene and make it visible
	glFlush();
	glutSwapBuffers();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
	init();
	//  Initialize GLUT
	glutInit(&argc,argv);
	//  Request double buffered, true color window with Z buffering at 600x600
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(700,700);
	glutCreateWindow("Robert Werthman Project");
	//  Set callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(key);
	//  Tell GLUT to call "idle" when there is nothing else to do
	glutIdleFunc(idle);

	// Load the textures for the helicopter
	littlebird[0] = LoadTexBMP("littlebirdenginetank.bmp");
	littlebird[1] = LoadTexBMP("littlebirdenginetank.bmp");
	littlebird[2] = LoadTexBMP("littlebirdcockpit.bmp");
	littlebird[3] = LoadTexBMP("littlebirdengine.bmp");
	littlebird[4] = LoadTexBMP("littlebirdskidmount.bmp");
	littlebird[5] = LoadTexBMP("littlebirdskid.bmp");
	littlebird[6] = LoadTexBMP("littlebirdrotor.bmp");
	littlebird[7] = LoadTexBMP("littlebirdgear.bmp");

	//  Check if any errors have occurred
	ErrCheck("init");
	//  Pass control to GLUT so it can interact with the user
	glutMainLoop();
	return 0;
}

