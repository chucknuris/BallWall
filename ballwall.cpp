#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h> // OpenGL 3.1 or higher versions. GLSL related functions
#include <GL/glut.h>
#endif

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include "loadCompileShaders.h"
#include "camera.h"

int WWIDTH = 600, WHEIGHT = 450; // glut window width and height
bool started, help; // started game flag, print help flag
int moveup, movedown, moveright, moveleft; // flags to indicate the active movements

GLuint sphereId, wallId[9]; // Vertex array ID for the sphere and the walls
int walltype, NumWallTriangles, NumSphereIndices;

float posx, posy, step, ballradius; // sphere x and y center coordinates, movement step and radius
float zwall, stepwall, holex, holey, holedist, holeradius; // wall z coordinate, movement step, hole center coordinates, etc.
float wallcolor[4], ballcolor[4]; // rgba colors

int life, score, highscore, numberofsteps; // game lives, scores. And number of movements steps to the wall reaches the sphere.

camera cam; // camera

// GLSL
GLuint progId = 0;                  // ID of GLSL program
GLint uniformMatrixModelView;
GLint uniformMatrixModelViewProjection;
GLint uniformMatrixNormal;
GLint uniformDisplacement;
GLint uniformLightPosition;
GLint uniformLightAmbient;
GLint uniformLightDiffuse;
GLint uniformLightSpecular;
GLint uniformMaterialColor;
GLint uniformMaterialSpecular;
GLint uniformMaterialShininess;

#define zwallinit 25.0f
// set wall parameters
void nextwall() {
	walltype = rand()%9; // wall type
	zwall = -zwallinit; // initial z position
	holex = (walltype%3 - 1)*holedist; // x coordinate of the hole center
	holey = (walltype/3 - 1)*holedist; // y coordinate of the hole center
	// saturated color:
	int i = rand()%3, j = rand()%2;
	wallcolor[i] = 1.0f;
	wallcolor[(1 + i + j)%3] = 0.0f;
//	wallcolor[(1 + i + !j)%3] = (float)rand()/RAND_MAX;
	wallcolor[(1 + i + !j)%3] = (rand()%5)*0.25f;
}

// calculate the movement steps
inline void calcsteps(int n) {
	numberofsteps = n;
	stepwall = zwallinit/numberofsteps;
	if (step < 0.125f)
		step = stepwall*0.5f; // sphere xy step
}

// set the sphere color
inline void setballcolor() {
	ballcolor[0] = 0.0f; ballcolor[1] = 0.7f; ballcolor[2] = 1.0f;
}

void initgame() {
	score = 0; // reset the score
	life = 3; // reset the lives
	if (highscore < 0)
		highscore = 0;
	calcsteps(250); // initialize movement steps
	moveup = movedown = moveright = moveleft = 0;
}

void initgameparam() {
	holedist = 1.0f;
	holeradius = 0.5f;
	ballradius = 0.4f;
	ballcolor[3] = wallcolor[3] = 1.0f;
	setballcolor();
	highscore = score = life = 0;
	started = false;
	help = true;
	zwall = -zwallinit;
	numberofsteps = -1; // used to print the initial message
}

void continuegame() {
	posx = posy = 0.0f;
	if (moveup == 1)
		moveup = 0;
	if (movedown == 1)
		movedown = 0;
	if (moveright == 1)
		moveright = 0;
	if (moveleft == 1)
		moveleft = 0;
}

void collision() {
	glutIdleFunc(0); // unregister the idle function
	started = false;
	for (int i = 0; i != 3; i++)
		ballcolor[i] = wallcolor[i] = 0.35f; // dark gray
}

void setVertexArrays() {
	/*********************** WALLS *********************/
	{
		struct v6f {
			float v[6]; // the 3 first floats are the point coordinates and the other 3 are the normal coordinates.
		};

		float r = holeradius; // hole radius
		float h = holedist; // distance between hole centers
		int N = 36; // sides of circle, this value must be a multiple of 4.
		float d = h + 1.4f * r; // half side length
		float z = 0.09f * d; // half wall thickness
		glGenVertexArrays(9, wallId); // generate the vertex array indices for all 9 walls
		for (int j = 0; j != 9; j++) {
			vector<v6f> vnbuffer; // CREATE A NEW VECTOR TO STORE THE THE POINT COORDINATES AND NORMALS.
			{
				float cx = (j % 3 - 1) * h; // hole center x coordinate           |6 7 8|
				float cy = (j / 3 - 1) * h; // hole center y coordinate           |3 4 5|
										    //          hole position for each j: |0 1 2|
				double angle = 0.0, delta = 2 * M_PI / N;
				float c = 1.0, s = 0.0;
				for (int i = 0; i != N; i++) { // border of circular hole (RED AREA)
					// 2 triangle are generated. All normals point to the xy hole center coordinates
										  //    x           y       z  nx  ny  nz
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, -z, -c, -s, 0.0f }); // POINT 1, FIGURE 3
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, -c, -s, 0.0f }); // POINT 2, FIGURE 3
					angle += delta;
					s = (float)sin(angle); c = (float)cos(angle);
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, -z, -c, -s, 0.0f }); // POINT 3, FIGURE 3
					vnbuffer.push_back(vnbuffer[vnbuffer.size() - 2]); // POINT 4 (SAME POINT 2), FIGURE 3
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, -c, -s, 0.0f }); // POINT 5, FIGURE 3
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, -z, -c, -s, 0.0f }); // POINT 6 (SAME POINT 3), FIGURE 3
				}

				angle = 0.0;
				c = 1.0; s = 0.0;
				for (int i = 0; i < N / 4; i++) { // (FIGURE 1, YELLOW) triangles from the upper right wall corner to the hole border
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
					vnbuffer.push_back(v6f{ d, d, z, 0.0f, 0.0f, 1.0f });
					angle += delta;
					s = (float)sin(angle); c = (float)cos(angle);
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
				}

				angle = M_PI / 2;
				c = 0.0; s = 1.0;
				for (int i = 0; i < N / 4; i++) { // (FIGURE 1, GREEN) triangles from the upper left wall corner to the hole border
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
					vnbuffer.push_back(v6f{ -d, d, z, 0.0f, 0.0f, 1.0f });
					angle += delta;
					s = (float)sin(angle); c = (float)cos(angle);
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
				}

				angle = M_PI;
				c = -1.0; s = 0.0;
				for (int i = 0; i < N / 4; i++) { // (FIGURE 1, CYAN) triangles from the lower left wall corner to the hole border
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
					vnbuffer.push_back(v6f{ -d, -d, z, 0.0f, 0.0f, 1.0f });
					angle += delta;
					s = (float)sin(angle); c = (float)cos(angle);
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
				}

				angle = 1.5 * M_PI;
				c = 0.0; s = -1.0;
				for (int i = 0; i < N / 4; i++) { // (FIGURE 1, MAGENTA) triangles from the lower right wall corner to the hole border
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
					vnbuffer.push_back(v6f{ d, -d, z, 0.0f, 0.0f, 1.0f });
					angle += delta;
					s = (float)sin(angle); c = (float)cos(angle);
					vnbuffer.push_back(v6f{ r * c + cx, r * s + cy, z, 0.0f, 0.0f, 1.0f });
				}

				// missing wall front face triangles  (FIGURE 1, BLUE):
				vnbuffer.push_back(v6f{ d, -d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ d, d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ r + cx, cy, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ d, d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ -d, d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ cx, r + cy, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ -d, d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ -d, -d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ cx - r, cy, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ -d, -d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ d, -d, z, 0.0f, 0.0f, 1.0f });
				vnbuffer.push_back(v6f{ cx, cy - r, z, 0.0f, 0.0f, 1.0f });

				// THE NEXT AREA (LIGHT GRAY) CAN BE SEEN ONLY IN THE FIGURE 4 (BACK VIEW):
				// wall top side (+y)
				vnbuffer.push_back(v6f{ d, d, z, 0.0f, 1.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, d, -z, 0.0f, 1.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, d, -z, 0.0f, 1.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, d, -z, 0.0f, 1.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, d, z, 0.0f, 1.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, d, z, 0.0f, 1.0f, 0.0f });

				// wall bottom side (-y)
				vnbuffer.push_back(v6f{ d, -d, -z, 0.0f, -1.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, -d, z, 0.0f, -1.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, -d, -z, 0.0f, -1.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, -d, z, 0.0f, -1.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, -d, -z, 0.0f, -1.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, -d, z, 0.0f, -1.0f, 0.0f });

				// wall left side (-x)
				vnbuffer.push_back(v6f{ -d, d, z, -1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, d, -z, -1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, -d, -z, -1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, -d, -z, -1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, -d, z, -1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ -d, d, z, -1.0f, 0.0f, 0.0f });

				// wall right side (+x)
				vnbuffer.push_back(v6f{ d, d, -z, 1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, d, z, 1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, -d, -z, 1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, -d, z, 1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, -d, -z, 1.0f, 0.0f, 0.0f });
				vnbuffer.push_back(v6f{ d, d, z, 1.0f, 0.0f, 0.0f });
			}
			glBindVertexArray(wallId[j]); // bind the wall j id
			GLuint vboID;
			glGenBuffers(1, &vboID); // Generate a Vertex buffer object
			glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the Vertex buffer object
			NumWallTriangles = (int)vnbuffer.size();
			// fill the buffer
			glBufferData(GL_ARRAY_BUFFER, NumWallTriangles * sizeof(v6f), vnbuffer[0].v, GL_STATIC_DRAW);
			// specify the location and data format of the vertex buffer array
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v6f), (void*)0);
			glEnableVertexAttribArray(0); // 0 FOR VERTEX COORDINATES
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(v6f), (void*)(3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(1); // 1 FOR NORMAL COORDINATES
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0); // unbind the wall j id
		} // BEFORE TO START A NEW CYCLE vnbuffer IS DESTROYED
	}

	/*********************SPHERE************************/
	{
		/* adapted from http://www.songho.ca/opengl/gl_sphere.html */
		int sectorCount = 36, stackCount = 18;
		float radius = ballradius;
		float Pi = (float)M_PI;
		vector<float> vnbuffer2;

		float x, y, z, xy;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal

		float sectorStep = 2 * Pi / sectorCount;
		float stackStep = Pi / stackCount;
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i)
		{
			stackAngle = Pi / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);             // r * cos(u)
			z = radius * sinf(stackAngle);              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= sectorCount; ++j)
			{
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
				y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
				vnbuffer2.push_back(x);
				vnbuffer2.push_back(y);
				vnbuffer2.push_back(z);

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				vnbuffer2.push_back(nx);
				vnbuffer2.push_back(ny);
				vnbuffer2.push_back(nz);
			}
		}
		// generate CCW index list of sphere triangles
		// k1--k1+1
		// |  / |
		// | /  |
		// k2--k2+1
		vector<int> indices;
		int k1, k2;
		for (int i = 0; i < stackCount; ++i)
		{
			k1 = i * (sectorCount + 1);     // beginning of current stack
			k2 = k1 + sectorCount + 1;      // beginning of next stack

			for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				// k1+1 => k2 => k2+1
				if (i != (stackCount - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}
		GLuint VBO, EBO;
		glGenVertexArrays(1, &sphereId); // generate the vertex array index for the sphere
		glGenBuffers(1, &VBO); // generate a vertex buffer
		glGenBuffers(1, &EBO); // generate a index buffer
		glBindVertexArray(sphereId); // bind the sphere id
		glBindBuffer(GL_ARRAY_BUFFER, VBO); // bind the vertex buffer
		// fill the vertex buffer
		glBufferData(GL_ARRAY_BUFFER, vnbuffer2.size() * sizeof(float), &vnbuffer2[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // bind the index buffer
		NumSphereIndices = (int)indices.size();
		// fill the index buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumSphereIndices * sizeof(float), &indices[0], GL_STATIC_DRAW);
		// specify the location and data format of the vertex buffer array
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0); // unbind the sphere id
	}
}

void drawball(float x, float y) {
	float d[3] = {x, y, 0.0f};
	glUniform4fv(uniformMaterialColor, 1, ballcolor); // set color
	glUniform3fv(uniformDisplacement, 1, d); // set position
	glBindVertexArray(sphereId);
	glDrawElements(GL_TRIANGLES, NumSphereIndices, GL_UNSIGNED_INT, 0); // draw the sphere
	glBindVertexArray(0);
}

void drawwall(GLuint id, float z, float *color) {
	float d[3] = {0.0f, 0.0f, z};
	glUniform4fv(uniformMaterialColor, 1, color); // set color
	glUniform3fv(uniformDisplacement, 1, d); // set position
	glBindVertexArray(id);
	glDrawArrays(GL_TRIANGLES, 0, NumWallTriangles); // draw the wall
	glBindVertexArray(0);
}

// display a char string in the glut window
void printchars(const char *s, float x, float y) {
	glRasterPos2f(x, y);
	while (*s)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *(s++));
}


void printinfo() {
	ostringstream out;
	glColor3f(0.8f, 0.8f, 0.8f);
	out << "High Score: " << highscore;
	float h = 35.0f/WHEIGHT;
	float x = -0.95f;
	float f = -9.0f/WWIDTH;
	printchars(out.str().c_str(), f*(2*15) - x, 1.0f - h);
	out.str("");
	out << "Score: " << score;
	printchars(out.str().c_str(), x, 1.0f - h);
	out.str("");
	out << "Lives: " << life;
	printchars(out.str().c_str(), x, 1.0f - 2*h);

	if (!started) {
		if (help) {
			printchars("Move the ball to match the position of the hole in the", f*54, 7*h);
			printchars("approaching wall.  Use the w (move up), a (move left),", f*54, 6*h);
			printchars("s (move down) and d (move right) keys.  Press h key to", f*54, 5*h);
			printchars("hide this information.", f*54, 4*h);
			printchars("", f*56, 3*h);
		}
		if (numberofsteps < 0)
			printchars("**** Press the Enter key to start ****", f*38, h);
		else if (!life) {
			printchars("**** GAME END ****", f*18, 2*h);
			printchars("Press the Enter key to start again", f*34, h);
		} else {
			printchars("Collision", f*9, 2*h);
			printchars("Press the space bar to continue", f*31, h);
		}
		printchars("h key: Display info.", x, 2*h - 1.0f);
		printchars("Escape key: Exit the game.", x, h - 1.0f);
	}
}

// render function
void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	printinfo();
	glUseProgram(progId); //
	drawball(posx, posy);
	drawwall(wallId[walltype], zwall, wallcolor);
	glUseProgram(0);
	glutSwapBuffers();
}

// change here the perspective parameters
void cameraperspective() {
	cam.perspective(45, double(WHEIGHT)/WWIDTH, 1.0, 200.0);
}

// glut window resize callback
void Reshape(int w, int h)
{
	WWIDTH = w; WHEIGHT = h;
	glViewport(0, 0, w, h);
	cameraperspective();
	cam.calcMatrixModelViewProjection();
	glUseProgram(progId); // installs the program progId
	// update the MatrixModelViewProjection in the vertex shader
	glUniformMatrix4fv(uniformMatrixModelViewProjection, 1, GL_TRUE, cam.getMatrixModelViewProjection());
	glUseProgram(0); // removes the program progId from use
	glutPostRedisplay(); // update the drawing
}


// idle function
void Idle() {
	static int gametime = glutGet(GLUT_ELAPSED_TIME);
	int dt = glutGet(GLUT_ELAPSED_TIME) - gametime; // elapsed time from the last action, in milliseconds
	if (dt < 20) // the highest frame rate permitted is 50
		return;
	gametime += (dt < 40? 20: dt); // update gametime
	// ***************** sphere movement ******************
	if (moveup) {
		posy += step;
		if (posy > 0) {
			if (holedist - posy < step) { // if the sphere is near of upper limit
				posy = holedist; // maximum y allowed coordinate
			}
		} else {
			if (posy > -step) { // if the sphere is near of the center
				posy = 0.0f;
				if (moveup == 1) // stop moving if the key is not pressed
					moveup = 0;
			}
		}
	}
	else if (movedown) {
		posy -= step;
		if (posy < 0) {
			if (holedist + posy < step) { // if the sphere is near of lower limit
				posy = -holedist; // minimum y allowed coordinate
			}
		} else {
			if (posy < step) {
				posy = 0.0f;
				if (movedown == 1)
					movedown = 0;
			}
		}
	}
	if (moveright) {
		posx += step;
		if (posx > 0) {
			if (holedist - posx < step) { // if the sphere is near of right limit
				posx = holedist; // maximum x allowed coordinate
			}
		} else {
			if (posx > -step) {
				posx = 0.0f;
				if (moveright == 1)
					moveright = 0;
			}
		}
	}
	else if (moveleft) {
		posx -= step;
		if (posx < 0) {
			if (holedist + posx < step) { // if the sphere is near of left limit
				posx = -holedist; // minimum y allowed coordinate
			}
		} else {
			if (posx < step) {
				posx = 0.0f;
				if (moveleft == 1)
					moveleft = 0;
			}
		}
	}

	// ***************** wall movement ******************
	if (started) {
		zwall += stepwall;
		if (fabs(zwall) < stepwall) { // if the wall is close to the sphere
			// differences of hole center coordinates and sphere center coordinates
			float dx = holex - posx, dy = holey - posy, dr = holeradius - ballradius;
			if (dx*dx + dy*dy > dr*dr) { // collision condition
				life--;
				collision();
			}
		} else if (zwall > 0.0f) {
			if (zwall > 5.0f) {
				if (++score%10 == 0) { // each 10 walls successfully passed 
					life++;
					if (numberofsteps > 25)
						calcsteps(numberofsteps - numberofsteps/8); // update the steps, faster movements
				}
				if (score > highscore)
					highscore = score; // update the high score
				nextwall(); // next wall
			}
		}
	}

	glutPostRedisplay(); // update the glut window
}

void nextwall2() {
	setballcolor(); // restore the sphere color changed in collision function
	glutIdleFunc(Idle); // register the idle function (unregistered in collision function)
	nextwall();
	started = true;
	help = false;
}

// key down callback
void Keyboard(unsigned char key, int, int) {
	switch (key) {
	case 27: // escape key
		exit(0);
		break;
	case '\r': // enter key
		if (life)
			return;
		initgame();
		nextwall2();
		break;
	case ' ':
		if (started || !life)
			return; // ignored if the game is running
		continuegame();
		nextwall2();
		break;
	case 'W':
	case 'w':
		moveup = 2;
		movedown = 0;
		break;
	case 'A':
	case 'a':
		moveleft = 2;
		moveright = 0;
		break;
	case 'S':
	case 's':
		movedown = 2;
		moveup = 0;
		break;
	case 'D':
	case 'd':
		moveright = 2;
		moveleft = 0;
		break;
	case 'H':
	case 'h':
		if (started)
			return;
		help = !help;
		break;
	case 'p':
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'l':
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}
	if (!started)
		glutPostRedisplay();
}

// key up callback
void KeyboardUp(unsigned char key, int, int) {
	switch (key) {
	case 'W':
	case 'w':
		moveup = 1;
		break;
	case 'A':
	case 'a':
		moveleft = 1;
		break;
	case 'S':
	case 's':
		movedown = 1;
		break;
	case 'D':
	case 'd':
		moveright = 1;
		break;
	}
}

void initGLSL() {
	progId = loadCompileShader("shader.vert", "shader.frag"); // compile the shaders

	glUseProgram(progId);
	// get uniform/attrib locations
	uniformMatrixModelView           = glGetUniformLocation(progId, "matrixModelView");
	uniformMatrixModelViewProjection = glGetUniformLocation(progId, "matrixModelViewProjection");
	uniformMatrixNormal              = glGetUniformLocation(progId, "matrixNormal");
	uniformDisplacement              = glGetUniformLocation(progId, "displacement");
	uniformLightPosition             = glGetUniformLocation(progId, "lightPosition");
	uniformLightAmbient              = glGetUniformLocation(progId, "lightAmbient");
	uniformLightDiffuse              = glGetUniformLocation(progId, "lightDiffuse");
	uniformLightSpecular             = glGetUniformLocation(progId, "lightSpecular");
	uniformMaterialColor             = glGetUniformLocation(progId, "materialColor");
	uniformMaterialSpecular          = glGetUniformLocation(progId, "materialSpecular");
	uniformMaterialShininess         = glGetUniformLocation(progId, "materialShininess");

	//Set up light source
	GLfloat lightPosition[] = {20.0f, 10.0f, -5.0f, 1.0f};
	GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
	GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat materialSpecular[] = {0.4f, 0.4f, 0.4f, 1.0f};
	GLfloat materialShininess  = 50;

	// set lighting related variables in the fragment shader
	glUniform4fv(uniformLightPosition, 1, lightPosition);
	glUniform4fv(uniformLightAmbient, 1, lightAmbient);
	glUniform4fv(uniformLightDiffuse, 1, lightDiffuse);
	glUniform4fv(uniformLightSpecular, 1, lightSpecular);
	glUniform4fv(uniformMaterialSpecular, 1, materialSpecular);
	glUniform1f(uniformMaterialShininess, materialShininess);

	// Camera inits
	cameraperspective();
	cam.LookAt(0.0, 3.0, 10.0, 0.0, -1.0, -20.0, 0.0, 1.0, 0.0);
	glUniformMatrix3fv(uniformMatrixNormal, 1, GL_TRUE, cam.getMatrixNormal());
	glUniformMatrix4fv(uniformMatrixModelView, 1, GL_TRUE, cam.getMatrixModelView());
	glUniformMatrix4fv(uniformMatrixModelViewProjection, 1, GL_TRUE, cam.getMatrixModelViewProjection());

	glUseProgram(0); // stop using progId

	//OpenGL inits:
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(WWIDTH, WHEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Ball Wall Game");

#ifndef __APPLE__
	if (glewInit())
	{
		cout << "Unable to initialize GLEW ... exiting\n";
		exit(EXIT_FAILURE);
	}
#endif

	initgameparam();
	setVertexArrays();
	initGLSL();

	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
//	glutSpecialFunc(Special);
//	glutSpecialUpFunc(SpecialUp);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutIdleFunc(Idle);

	glutMainLoop();
	return 0;
}