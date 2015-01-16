/* terrain.c by Alexander Zaranek
 *  Random 3D terrain generator using the circles and fault algorithms.
 *  Features lighting and different shading models.
 */
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <gl/glut.h>
#include <gl/gl.h>

int main_id;
int terrainsize_id;
int genmode_id;

int terrainSize = 100;	//amount of vertices in the x and z direction, max of 300
float terrain[300][300];	//holds the y values of the terrain for each x and z coordinate

float xcamp = -4.0, ycamp = 3.0, zcamp = -4.0; //initial viewing position
float xcamr = 10, ycamr = 135; //initial viewing rotation

int polyMode = 0;  //0 = Solid, 1 = Wireframe, 2 = Solid with wireframe
int shadeMode = 0; //0 = Gourard, 1 = Flat
bool snowman = false; //determines if the snowman is displayed
int genMode = 0;	  //0 = Circles, 1 = Fault
int lighting = false; //determines if the first light is displayed
int lighting2 = false; //determines if the second light is displayed

float smx = 0.1; //snowman's x position
float smz = 0.1; //snowman's x position

float normals[300][300][3];	//Stores the normal vector for every vertex on the terrain.
float lightpos[] = {0, 2.0, 0, 1.0};	//initial position of first light
float light2pos[] = {10, 2.0, 10, 1.0}; //initial position of second light
float lightamb[] = { 0.0, 0.0, 0.0, 1.0 }; //ambient component of the second light
float lightdiff[] = { 1.0, 0.0, 0.0, 1.0 }; //diffuse component of the second light
float lightspec[] = { 1.0, 1.0, 1.0, 1.0 }; //specular component of the second light

/* createCircle(x, z, height, szie) - Takes a point on the terrain at the x and z coordinate
 * and raises it to the given height. The terrain is then gradually lowered depending on the
 * size. Used for the circles algorithm.
 */
void createCircle(int x, int z, double height, double size){
	for (int i = 0; i<terrainSize; i++){
		for (int j = 0; j<terrainSize; j++){
				int dx = abs(x - i);
				int dy = abs(z - j);
				double param = (dx*dx) + (dy*dy);
				double dist = sqrt(param);

				double pd = dist*2 / size;
				if (fabs(pd) <= 1.0){
					terrain[i][j] +=  height/2 + cos(pd*3.14)*height/2;
				}
		}
	}
}

/* createFault() - Divides the terrain into 2 sections as determined by a randomly
 * drawn line through the terrain. One section is lowered and the other section is
 * raised to create a fault used for the fault algorithm.
 */
void createFault(){
	float r1 = ((double) rand() / (RAND_MAX))*terrainSize;
	float a = sin(r1);
	float b = cos(r1);
	float d = 1.4*terrainSize;
	float c = ((double) rand() / (RAND_MAX))*d - d/2;
	
	for (int i = 0; i<terrainSize; i++){
		for (int j = 0; j<terrainSize; j++){
			if ((a*i + b*j - c) > 0){ 
				terrain[i][j] += 0.04;
			}else {
				terrain[i][j] -= 0.04;
			}
		}
	}
}

/* findNormals() - Determines the normal vector for each vertex in the terrain array.
 */
void findNormals(){
	for (int x = 0; x<terrainSize-1; x++){
		for (int z = 0; z<terrainSize-1; z++){
			float o[3] = {x, terrain[x][z], z};
			float b[3] = {x - o[0], terrain[x][z+1] - o[1], z+1 - o[2]};
			float c[3] = {x+1 - o[0], terrain[x+1][z] - o[1], z - o[2]};

			float ax = (b[1]*c[2]) - (b[2]*c[1]);
			float ay = (b[2]*c[0]) - (b[0]*c[2]);
			float az = (b[0]*c[1]) - (b[1]*c[0]);

			float len = sqrt((ax*ax) + (ay*ay) + (az*az));

			normals[x][z][0] = ax/len;
			normals[x][z][1] = ay/len;
			normals[x][z][2] = az/len;
		}
	}
}

/* generateTerrain() - Generates the terrain array to be displayed. Determines the height of the terrain
 * for each x and z coordinate using the circles or fault algorithm.
 */
void generateTerrain(){
	for (int i = 0; i<terrainSize; i++){
		for (int j = 0; j<terrainSize; j++){
			terrain[i][j] = 0;
		}
	}
	if (genMode == 0){
		int iter = terrainSize/5;	//used to appropriately scale the amount of iterations used for circle generation
		for (int i = 0; i<(iter*iter); i++){
			int rx = ((double) rand()/RAND_MAX)*terrainSize;
			int rz = ((double) rand()/RAND_MAX)*terrainSize;

			createCircle(rx,rz,0.1,20);
		}
	}else {
		for (int i = 0; i<(terrainSize*5); i++){
			createFault();
		}
	}

	findNormals();
}

/* drawStrip(z) - Draws a triangular strip representing one of the z rows in the
 * terrain. The colour of the terrain is determined by the height of a vertex
 * with white as the highest point and black as the lowest.
 */
void drawStrip(int z){
	float highest = 0;	//stores the highest height value
	for (int i = 0; i<terrainSize; i++){
		for (int j = 0; j<terrainSize; j++){
			if (terrain[i][j] > highest){
				highest = terrain[i][j];
			}
		}
	}

	float lowest = 0;	//stores the lowest height value
	for (int i = 0; i<terrainSize; i++){
		for (int j = 0; j<terrainSize; j++){
			if (terrain[i][j] < lowest){
				lowest = terrain[i][j];
			}
		}
	}

	highest = highest - lowest;
	
	glBegin(GL_TRIANGLE_STRIP);	
		for(int x = 0; x < terrainSize; x++){
			float shade = ((terrain[x][z] - lowest)/highest);
			float shade2 = ((terrain[x][z+1] - lowest)/highest);
			glColor3f(shade, shade, shade);
			glNormal3f(normals[x][z][0], normals[x][z][1], normals[x][z][2]);
			glVertex3f(0.1*x, terrain[x][z], 0.1*z);
			glColor3f(shade2, shade2, shade2);
			glNormal3f(normals[x][z+1][0], normals[x][z+1][1], normals[x][z+1][2]);
			glVertex3f(0.1*x, terrain[x][z+1], 0.1*(z+1));
		}
	glEnd();
}

/* drawTerrain() - Draws the terrain row by row using triangular strips.
 */
void drawTerrain(){	
	for(int i = 0; i < terrainSize-1; i++){
		drawStrip(i);
	}
}


/* drawWireframe() - Draws the terrain as a black wireframe.
 */
void drawWireframe(){	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for(int i = 0; i < terrainSize-1; i++){
		glBegin(GL_TRIANGLE_STRIP);	
			for(int x = 0; x < terrainSize; x++){
				glColor3f(0, 0, 0);
				glVertex3f(0.1*x, terrain[x][i], 0.1*i);
				glVertex3f(0.1*x, terrain[x][i+1], 0.1*(i+1));
			}
		glEnd();
		}
}

/* drawSnowman() - Creates a snowman model using primitives. Also determines where
 * the snowman is drawn depending on terrain coordinates and height.
 */
void drawSnowman(){
	glPushMatrix();

	glColor3f(1, 1, 1);
	int smxscaled = (smx*10);
	int smzscaled = (smz*10);
	float smy = terrain[smxscaled][smzscaled] + 0.08;
	glTranslatef(smx, smy, smz);
	glutSolidSphere(0.1, 16, 16);

	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslatef(-0.095, 0.0, 0.0);
	glutSolidSphere(0.01, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.095, 0.025, 0.0);
	glutSolidSphere(0.01, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.095, -0.025, 0.0);
	glutSolidSphere(0.01, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(0, 0.125, 0);
	glutSolidSphere(0.05, 16, 16);

	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslatef(-0.04, 0.01, 0.02);
	glutSolidSphere(0.01, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.04, 0.01, -0.02);
	glutSolidSphere(0.01, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 0.5, 0.5);
	glTranslatef(-0.04, -0.01, 0);
	glRotatef(-90, 0, 1, 0);
	glutSolidCone(0.01, 0.05, 16, 16);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
}

/* display() - the OpenGL display function, this draws the screen
 *  it displays a spinning cube
 */
void display()
{
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//camera angle and location
	glRotatef(xcamr,1.0,0.0,0.0);
    glRotatef(ycamr,0.0,1.0,0.0);
    glTranslated(-xcamp,-ycamp,-zcamp);

	//displays the lighting
	if (lighting){
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	}if (lighting2){
		glLightfv(GL_LIGHT1, GL_POSITION, light2pos);
		glLightfv(GL_LIGHT1, GL_AMBIENT, lightamb);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightdiff);
		glLightfv(GL_LIGHT1, GL_SPECULAR, lightspec);
	}

	//displays the terrain and its different modes
	if (polyMode == 2){
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawTerrain();
		drawWireframe();
	}else{
		drawTerrain();
	}

	//display snowman
	if (snowman == true){
		drawSnowman();
	}
	
	//swap buffers - rendering is done to the back buffer, bring it forward to display
	glutSwapBuffers();

	//force a redisplay, to keep the animation running
	glutPostRedisplay();
}

/* defaultCam() - Resets the camera to the default position.
 */
void defaultCam(){
	xcamp = -4.0, ycamp = 3.0, zcamp = -4.0;
	xcamr = 10, ycamr = 135;
}

/* snowmanMid() - Moves the snowman to the centre of the terrain.
 */
void snowmanMid(){
	smx = terrainSize/20.0;
	smz = terrainSize/20.0;
}

/* kbd -- the GLUT keyboard function 
 *  key -- the key pressed
 */
void kbd(unsigned char key, int x, int y)
{
	//if the "q" key is pressed, quit the program
	if(key == 'q' || key == 'Q')
	{
		exit(0);
	}
	if(key == 'w' || key == 'W')
	{
		if (polyMode == 0){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			polyMode = 1;
		}
		else if (polyMode == 1){
			polyMode = 2;
		}
		else if (polyMode == 2){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			polyMode = 0;
		}
	}
	if(key == 's' || key == 'S')
	{
		if (shadeMode == 0){
			glShadeModel(GL_FLAT);
			shadeMode = 1;
		}
		else if (shadeMode == 1){
			glShadeModel(GL_SMOOTH);
			shadeMode = 0;
		}
	}
	if(key == 'r' || key == 'R'){
		generateTerrain();
	}
	if(key == 'c' || key == 'C'){
		defaultCam();
	}
	if(key == 'o' || key == 'O'){
		snowman = !snowman;
		snowmanMid();
	}
	if(key == 'i' || key == 'I'){
		if (smx > (terrainSize/10 - 0.1)){
		}else {
			smx = smx + 0.1;
		}
	}
	if(key == 'k' || key == 'K'){
		if (smx < 0.1){
		}else {
			smx = smx - 0.1;
		}
	}
	if(key == 'j' || key == 'J'){
		if (smz < 0.1){
		}else {
			smz = smz - 0.1;
		}
	}
	if(key == 'l' || key == 'L'){
		if (smz > (terrainSize/10 - 0.1)){
		}else {
			smz = smz + 0.1;
		}
	}

	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT){
		if(key == 'y' || key == 'Y'){
			if (lighting2 == false){
				light2pos[0] = terrainSize/10;
				light2pos[2] = terrainSize/10;
				if (lighting == false){
					glEnable(GL_LIGHTING);
				}
				glEnable(GL_LIGHT1);
				lighting2 = true;
			}else {
				glDisable(GL_LIGHT1);
				lighting2 = false;
				if (lighting == false){
					glDisable(GL_LIGHTING);
				}
			}
		}
		if(key == 't' || key == 'T'){
		light2pos[0] += 0.1;
		}
		if(key == 'g' || key == 'G'){
			light2pos[0] -= 0.1;
		}
		if(key == 'f' || key == 'F'){
			light2pos[2] -= 0.1;
		}
		if(key == 'h' || key == 'H'){
			light2pos[2] += 0.1;
		}
	}else {
		if(key == 'y' || key == 'Y'){
			if (lighting == false){
				lightpos[0] = 0;
				lightpos[2] = 0;
				if (lighting2 == false){
					glEnable(GL_LIGHTING);
				}
				glEnable(GL_LIGHT0);
				lighting = true;
			}else {
				glDisable(GL_LIGHT0);
				lighting = false;
				if (lighting2 == false){
					glDisable(GL_LIGHTING);
				}
			}
		}
		if(key == 't' || key == 'T'){
			lightpos[0] += 0.1;
		}
		if(key == 'g' || key == 'G'){
			lightpos[0] -= 0.1;
		}
		if(key == 'f' || key == 'F'){
			lightpos[2] -= 0.1;
		}
		if(key == 'h' || key == 'H'){
			lightpos[2] += 0.1;
		}
	}
}

/* special -- the GLUT special key function 
 *  key -- the key pressed
 */
void special(int key, int x, int y){
	float xrotrad, yrotrad;
	switch(key)
	{
		case GLUT_KEY_LEFT:
			ycamr -= 1;
			if (ycamr < -360)ycamr += 360;
			break;

		case GLUT_KEY_RIGHT:
			ycamr += 1;
			if (ycamr >360) ycamr -= 360;
			break;

		case GLUT_KEY_UP:
			yrotrad = (ycamr / 180 * 3.141592654f);
			xrotrad = (xcamr / 180 * 3.141592654f);
			xcamp += float(0.1*sin(yrotrad));
			zcamp -= float(0.1*cos(yrotrad));
			ycamp -= float(0.1*sin(xrotrad));
			break;

		case GLUT_KEY_DOWN:
			yrotrad = (ycamr / 180 * 3.141592654f);
			xrotrad = (xcamr / 180 * 3.141592654f);
			xcamp -= float(0.1*sin(yrotrad));
			zcamp += float(0.1*cos(yrotrad));
			ycamp += float(0.1*sin(xrotrad));
			break;

		case GLUT_KEY_PAGE_UP:
			xcamr -= 1;
			if (xcamr < -90) xcamr = -90;
			break;

		case GLUT_KEY_PAGE_DOWN:
			xcamr += 1;
			if (xcamr >90) xcamr = 90;
			break;
	}
}

/* menuGM - Options change the terrain generation algorithm that is used.
 */
void menuGM(int value){
	if(value == 1){
		genMode = 0;
		generateTerrain();
		defaultCam();
	}
	if(value == 2){
		genMode = 1;
		generateTerrain();
		defaultCam();
	}
}

/* menuTS - Options will change the size of the currently generated terrain and regenerate it.
 */
void menuTS(int value){
	if(value == 1){
		terrainSize = 50;
		generateTerrain();
		snowmanMid();
		defaultCam();
		lightpos[0] = 0;
		lightpos[2] = 0;
		light2pos[0] = terrainSize/10;
		light2pos[2] = terrainSize/10;
	}
	if(value == 2){
		terrainSize = 100;
		generateTerrain();
		snowmanMid();
		defaultCam();
		lightpos[0] = 0;
		lightpos[2] = 0;
		light2pos[0] = terrainSize/10;
		light2pos[2] = terrainSize/10;
	}
	if(value == 3){
		terrainSize = 150;
		generateTerrain();
		snowmanMid();
		defaultCam();
		lightpos[0] = 0;
		lightpos[2] = 0;
		light2pos[0] = terrainSize/10;
		light2pos[2] = terrainSize/10;
	}
	if(value == 4){
		terrainSize = 200;
		generateTerrain();
		snowmanMid();
		defaultCam();
		lightpos[0] = 0;
		lightpos[2] = 0;
		light2pos[0] = terrainSize/10;
		light2pos[2] = terrainSize/10;
	}
	if(value == 5){
		terrainSize = 250;
		generateTerrain();
		snowmanMid();
		defaultCam();
		lightpos[0] = 0;
		lightpos[2] = 0;
		light2pos[0] = terrainSize/10;
		light2pos[2] = terrainSize/10;
	}
	if(value == 6){
		terrainSize = 300;
		generateTerrain();
		snowmanMid();
		defaultCam();
		lightpos[0] = 0;
		lightpos[2] = 0;
		light2pos[0] = terrainSize/10;
		light2pos[2] = terrainSize/10;
	}
}

/* menuTS - Determines what functions the options perform in the main menu.
 */
void menuProc(int value){
	if(value == 1)
		exit(0);
}

/* main function - program entry point */
int main(int argc, char** argv)
{
	//glut initialization stuff:
	// set the window size, display mode, and create the window
	srand(time(NULL));

	glutInit(&argc, argv);

	glutInitWindowSize(600, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Random Terrain Generator");

	//print program instructions to console
	printf("Random Terrain Generator Instructions\n\n");
	printf("Right click to access terrain options.\n");
	printf("Select 'Terrain Size' to change the amount of vertices in the x and z \ndirections.\n");
	printf("Select 'Generation Type' to change the algorithm used to generate the terrain.\n");
	printf("Press R to generate a random terrain with the current settings.\n");
	printf("Press W to toggle the wireframe. Press it again to toggle the solid and \nwireframe together.\n");
	printf("Press S to toggle between flat shading and Gourard shading.\n");
	printf("Use the arrow keys to navigate the camera.\n");
	printf("Use PgUp and PgDn to angle the camera up and down.\n");
	printf("Press C to return the camera to the default position.\n");
	printf("Press Y to toggle the white light.\n");
	printf("Press Shift Y to toggle the red light.\n");
	printf("Press F and H to move the white light in the x-direction.\n");
	printf("Press T and G to move the white light in the z-direction.\n");
	printf("Hold shift while pressing F, H, T, and G to move the red light.\n");
	printf("Press O to toggle the snowman model.\n");
	printf("Press J and L to move the snowman in the x-direction.\n");
	printf("Press I and K to move the snowman in the z-direction.\n");
	printf("Press Q to exit out of the program.\n");

	//enable Z buffer test, otherwise things appear in the order they're drawn
	glEnable(GL_DEPTH_TEST);

	//setup the initial view
	// change to projection matrix mode, set the extents of our viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 0.1, 100);
	
	//enable backface culling
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	//set clear colour to black
	glClearColor(0, 0, 0, 0);
	
	//generate new terrain for initialization
	generateTerrain();

	//menu options
	genmode_id = glutCreateMenu(menuGM);
	glutAddMenuEntry("Circles", 1);
	glutAddMenuEntry("Fault", 2);

	terrainsize_id = glutCreateMenu(menuTS);
	glutAddMenuEntry("50", 1);
	glutAddMenuEntry("100", 2);
	glutAddMenuEntry("150", 3);
	glutAddMenuEntry("200", 4);
	glutAddMenuEntry("250", 5);
	glutAddMenuEntry("300", 6);

	main_id = glutCreateMenu(menuProc);
	glutAddSubMenu("Terrain Size", terrainsize_id);
	glutAddSubMenu("Generation Type", genmode_id);
	glutAddMenuEntry("Quit", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	//register glut callbacks for keyboard and display function
	glutKeyboardFunc(kbd);
	glutSpecialFunc(special);
	glutDisplayFunc(display);

	//start the program!
	glutMainLoop();

	return 0;
}