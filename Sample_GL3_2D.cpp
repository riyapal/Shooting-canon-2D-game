#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

float DEG2RAD(float i)
{
	return ((i*3.14)/180);
}

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
float canon_rot_dir = 1;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool canon_rot_status = false;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int flag = 0;
int flag1=0;
int flag_s, flag_f;
double can_x, can_y;
double canon_rotation;
double start_t;
double u=10;
double v;
int score=0;
int up = 0;
int down = 0;
int panleft=0;
int panright=0;
int rot_a=0;
int rot_b=0;
double angle = 0;
int gaga=0;
int score1=0;
int score2=0;
int score3=0;
int over=0;
double ay;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.


	if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_C:
				rectangle_rot_status = !rectangle_rot_status;
				break;
			case GLFW_KEY_P:
				triangle_rot_status = !triangle_rot_status;
				break;
			case GLFW_KEY_A:
				canon_rot_status = false;
				rot_a=0;
				//cout << canon_rotation << endl;
				// do something ..
				//cout << can_x << endl;
				//cout << can_y << endl;
				break;
			case GLFW_KEY_B:
				//	cout << canon_rotation << endl ;
				canon_rot_status = false;
				rot_b=0;
				//			cout << canon_rotation;
				//cout << can_x << endl;
				//cout << can_y << endl;
				break;
			case GLFW_KEY_SPACE:
				//				start_t=glfwGetTime();
				flag1 = 1;
				//	gaga=0;
			case GLFW_KEY_F:
				flag_f=0;
				break;
			case GLFW_KEY_S:
				flag_s=0;
				break;
			case GLFW_KEY_UP:
				up=0;
				//flag=0;
				break;
			case GLFW_KEY_DOWN:
				down=0;
				break;
			case GLFW_KEY_LEFT:
				panleft=0;
				break;
			case GLFW_KEY_RIGHT:
				panright=0;
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				cout << "GAME OVER! " << endl;
				cout << "SCORE : " << score << endl;
				quit(window);
				break;

			case GLFW_KEY_A:
				rot_a=1;
				//can_x=-12 + 2*cos(DEG2RAD(canon_rotation + atan(0.5/2)));
				//can_y = -6.5 + 2*sin(DEG2RAD(canon_rotation + atan(0.5/2)));
				break;
			case GLFW_KEY_B:
				rot_b=1;
				//can_x=-12 + 2*cos(DEG2RAD(canon_rotation + atan(0.5/2)));
				//can_y = -6.5 + 2*sin(DEG2RAD(canon_rotation + atan(0.5/2)));
				break;
			case GLFW_KEY_SPACE:
				if (flag==0)
				{
					angle = canon_rotation;
					can_x=-12 + 2*cos(DEG2RAD(angle + atan(0.5/2)));
					can_y = -6.5 + 2*sin(DEG2RAD(angle + atan(0.5/2)));
					flag=1;
				}
				break;
			case GLFW_KEY_F:
				//		u+=0.1;
				flag_f=1;
				break;
			case GLFW_KEY_S:
				//u-=0.1;
				flag_s=1;
				break;
			case GLFW_KEY_UP:
				up=1;
				break;
			case GLFW_KEY_DOWN:
				down=1;
				break;
			case GLFW_KEY_LEFT:
				panleft=1;
				break;
			case GLFW_KEY_RIGHT:
				panright=1;
				break;
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}
int right_click=0;
int scroll_left=0;
int scroll_right=0;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE)
				//triangle_rot_dir *= -1;
				if (flag==0)
				{
					angle = canon_rotation;
					can_x=-12 + 2*cos(DEG2RAD(angle + atan(0.5/2)));
					can_y = -6.5 + 2*sin(DEG2RAD(angle + atan(0.5/2)));
					flag=1;
					flag_f=0;
				}
			if (action == GLFW_PRESS)
			{
				flag_f=1;
				break;
			}

			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_PRESS) {
				//rectangle_rot_dir *= -1;
				right_click=1;
			}

			if (action == GLFW_RELEASE) {
				right_click=0;
				scroll_left=0;
				scroll_right=0;
			}
			break;
		default:
			break;
	}
}
int scroll_up=0;
int scroll_down=0;
//int scroll_left=0;
//int scroll_right=0;
void scroll ( GLFWwindow *window , double x, double y)
{
	float p,g;	
	p=float(y)/4;
	g=float(x)/4;
	cout << g;
	if ( p<0 )
	{
		scroll_down=1;
	}
	if (p>0 )
	{
		scroll_up=1;
	}
	if ( g>0 )
	{
		scroll_left=1;
	}
	if ( g<0 )
	{
		scroll_right=1;
	}
	//float g;
	//g=float(x)/4;
	//cout << g;

}
//int up=0;
/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	/*	if (up==0)
		{ 
		lx=-16.0;
		rx=16.0;
		dy=-8.0;
		uy=8.0;


		}
		else if ( up==1)
		{
		lx++;
		rx--;
		dy++;
		uy--;
		up=0;
		}	
		Matrices.projection = glm::ortho(lx, rx, dy, uy, 0.1f, 500.0f);
		}*/
	}
VAO *triangle, *rectangle;
VAO *circle;
VAO *base, *canon;
VAO *ground, *sky;
VAO *ball1;
VAO *stick, *stand;
VAO *target1, *target2, *target3;
VAO *triangle1, *triangle2;
VAO *fly, *arrow, *speedbar;
// Creates the triangle object used in this sample code

void createTriangle1 ()
{
	// ONLY vertices between the bounds specified in glm::ortho will be visible on screen 

	// Define vertex array as used in glBegin (GL_TRIANGLES) 
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle1 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTriangle2 ()
{
	// ONLY vertices between the bounds specified in glm::ortho will be visible on screen 

	// Define vertex array as used in glBegin (GL_TRIANGLES) 
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle2 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createCircle(float radius, float cirx, float ciry)

{
	GLfloat* vertex_buffer_data = new GLfloat [3*360];
	GLfloat* color_buffer_data = new GLfloat [3*360];

	for (int i=0; i<360; i++)
	{
		vertex_buffer_data [3*i] = (radius * cos(DEG2RAD(i)));
		vertex_buffer_data [3*i + 1] = (radius * sin(DEG2RAD(i)));
		vertex_buffer_data [3*i +2] = 0;

		color_buffer_data [3*i] = 0.5;
		color_buffer_data [3*i + 1] = 0.2;
		color_buffer_data [3*i + 2] = 0.05;
	}
	circle =  create3DObject(GL_TRIANGLE_FAN, 360, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBall1(float radius, float cirx, float ciry)

{
	GLfloat* vertex_buffer_data = new GLfloat [3*360];
	GLfloat* color_buffer_data = new GLfloat [3*360];

	for (int i=0; i<360; i++)
	{
		vertex_buffer_data [3*i] = (radius * cos(DEG2RAD(i)));
		vertex_buffer_data [3*i + 1] = (radius * sin(DEG2RAD(i)));
		vertex_buffer_data [3*i +2] = 0;

		color_buffer_data [3*i] = 0.5;
		color_buffer_data [3*i + 1] = 0.2;
		color_buffer_data [3*i + 2] = 0.5;
	}
	ball1 =  create3DObject(GL_TRIANGLE_FAN, 360, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTarget1(float radius, float cirx, float ciry)

{
	GLfloat* vertex_buffer_data = new GLfloat [3*360];
	GLfloat* color_buffer_data = new GLfloat [3*360];

	for (int i=0; i<360; i++)
	{
		vertex_buffer_data [3*i] = (radius * cos(DEG2RAD(i)));
		vertex_buffer_data [3*i + 1] = (radius * sin(DEG2RAD(i)));
		vertex_buffer_data [3*i +2] = 0;

		color_buffer_data [3*i] = 0;
		color_buffer_data [3*i + 1] = 0;
		color_buffer_data [3*i + 2] = 0;
	}
	target1 =  create3DObject(GL_TRIANGLE_FAN, 360, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTarget2(float radius, float cirx, float ciry)

{
	GLfloat* vertex_buffer_data = new GLfloat [3*360];
	GLfloat* color_buffer_data = new GLfloat [3*360];

	for (int i=0; i<360; i++)
	{
		vertex_buffer_data [3*i] = (radius * cos(DEG2RAD(i)));
		vertex_buffer_data [3*i + 1] = (radius * sin(DEG2RAD(i)));
		vertex_buffer_data [3*i +2] = 0;

		color_buffer_data [3*i] = 0;
		color_buffer_data [3*i + 1] = 0;
		color_buffer_data [3*i + 2] = 0;
	}
	target2 =  create3DObject(GL_TRIANGLE_FAN, 360, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTarget3(float radius, float cirx, float ciry)

{
	GLfloat* vertex_buffer_data = new GLfloat [3*360];
	GLfloat* color_buffer_data = new GLfloat [3*360];

	for (int i=0; i<360; i++)
	{
		vertex_buffer_data [3*i] = (radius * cos(DEG2RAD(i)));
		vertex_buffer_data [3*i + 1] = (radius * sin(DEG2RAD(i)));
		vertex_buffer_data [3*i +2] = 0;

		color_buffer_data [3*i] = 0;
		color_buffer_data [3*i + 1] = 0;
		color_buffer_data [3*i + 2] = 0;
	}
	target3 =  create3DObject(GL_TRIANGLE_FAN, 360, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code

void createGround ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-16,-7.75,0, // vertex 1
		-16,-8,0, // vertex 2
		16,-8,0, // vertex 3

		16,-8,0, // vertex 3
		16,-7.75,0, // vertex 4
		-16,-7.75,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0,0,0, // color 1
		0,0,0, // color 2
		0,0,0, // color 3

		0,0,0, // color 3
		0,0,0, // color 4
		0,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	ground = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSky ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-16,8,0, // vertex 1
		-16,-3,0, // vertex 2
		16,-3,0, // vertex 3

		16,-3,0, // vertex 3
		16,8,0, // vertex 4
		-16,8,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0,1,1, // color 1
		1,1,1, // color 2
		1,1,1, // color 3

		1,1,1, // color 3
		0,1,1, // color 4
		0,1,1  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	sky = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSpeedbar()
{
	static const GLfloat vertex_buffer_data [] = {
		-15, 5, 0,
		-15, 0, 0,
		-14.25, 0, 0,

		-14.25, 0, 0,
		-14.25, 5, 0,
		-15, 5, 0
	};
	static const GLfloat color_buffer_data [] = {
		1,0,0,
		0,1,0,
		0,1,0,

		0,1,0,
		1,0,0,
		1,0,0
	};

	speedbar = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBase ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-11.25,-7.25,0, // vertex 1
		-11.25,-7.75,0, // vertex 2
		-12.75, -7.75,0, // vertex 3

		-12.75, -7.75,0, // vertex 3
		-12.75, -7.25,0, // vertex 4
		-11.25,-7.25,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0.5,0.2,0.05, // color 1
		0.5,0.2,0.05, // color 2
		0.5,0.2,0.05, // color 3

		0.5,0.2,0.05, // color 3
		0.5,0.2,0.05, // color 4
		0.5,0.2,0.05  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	base = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createFly ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		7,3.25,0, // vertex 1
		11,3.25,0, // vertex 2
		11, 2.75,0, // vertex 3

		11, 2.75,0, // vertex 3
		7, 2.75,0, // vertex 4
		7,3.25,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		0.5,0.2,0.05, // color 1
		0.5,0.2,0.05, // color 2
		0.5,0.2,0.05, // color 3

		0.5,0.2,0.05, // color 3
		0.5,0.2,0.05, // color 4
		0.5,0.2,0.05  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	fly = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createStick ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		0,0,0, // vertex 1
		-1,0,0, // vertex 2
		-1,4,0, // vertex 3

		-1,4,0, // vertex 3
		0,4, 0, // vertex 4
		0,0,0,  // vertex 5

	};

	static const GLfloat color_buffer_data [] = {
		0.5,0.2,0.05, // color 1
		0.5,0.2,0.05, // color 2
		0.5,0.2,0.05, // color 3

		0.5,0.2,0.05, // color 3
		0.5,0.2,0.05, // color 4
		0.5,0.2,0.05,  // color 1

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	stick = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createStand ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		0,0,0, // vertex 1
		0,2,0, // vertex 2
		3,2,0, // vertex 3

		3,2,0, // vertex 3
		5,0,0, // vertex 4
		0,0,0,  // vertex 5

	};

	static const GLfloat color_buffer_data [] = {
		0.5,0.2,0.05, // color 1
		0.5,0.2,0.05, // color 2
		0.5,0.2,0.05, // color 3

		0.5,0.2,0.05, // color 3
		0.5,0.2,0.05, // color 4
		0.5,0.2,0.05,  // color 1

	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	stand = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createArrow()
{
	static const GLfloat vertex_buffer_data [] = {
		-1.5, -0.25, 0,
		-1.5, 0.25, 0,
		0, 0.25, 0,

		0, 0.25, 0,
		0, -0.25, 0,
		-1.5, -0.25, 0,

		-1.5, -0.25, 0,
		-1.75, 0, 0,
		-1.5, 0.25, 0	
	};

	static const GLfloat color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0
	};
	arrow = create3DObject(GL_TRIANGLES, 9, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createCanon ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		/* -12,-6.75,0, // vertex 1
		   -12,-6.25,0, // vertex 2
		   -10,-6.25,0, // vertex 3

		   -10, -6.25,0, // vertex 3
		   -10, -6.75,0, // vertex 4
		   -12,-6.75,0  // vertex 1
		 */
		0,0,0,
		0,0.5,0,
		2,0.5,0,

		2,0.5,0,
		2,0,0,
		0,0,0
	};

	static const GLfloat color_buffer_data [] = {
		0.5,0.2,0.05, // color 1
		0.5,0.2,0.05, // color 2
		0.5,0.2,0.05, // color 3

		0.5,0.2,0.05, // color 3
		0.5,0.2,0.05, // color 4
		0.5,0.2,0.05  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	canon = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
double t, curr_t;
double pos_x, pos_y;
double  bx, by;
int t1=1;
int t2=1;
int t3=1;
int f=1;
float lx=-16.0;
float rx=16.0;
float dy=-8.0;
float upy=8.0;
int theta;
double ux=u*cos(DEG2RAD(theta));
double uy=u*sin(DEG2RAD(theta));
double vx=v*cos(DEG2RAD(theta));
double vy=v*sin(DEG2RAD(theta));
int yay = 0;
int haha=0;
int arrowy;
//int x,y;
//float canon_rotation = 0;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	if (canon_rotation<=90 && rot_a==1)
	{

		canon_rot_dir = 1;
		canon_rot_status = true;
	}
	if (canon_rotation>=0 && rot_b==1)
	{
		canon_rot_dir = -1;
		canon_rot_status = true;
	}
	//	cout << "in draw yo";


	/*	if (up==0)
		{ 
		lx=-16.0;
		rx=16.0;
		dy=-8.0;
		uy=8.0;


		}*/
	if ( up==1 || scroll_up==1)
	{
		lx+=0.1;
		rx-=0.1;
		dy+=0.1;
		upy-=0.1;
		scroll_up=0;

	}
	if (down==1 || scroll_down==1)
	{
		lx-=0.1;
		rx+=0.1;
		dy-=0.1;
		upy+=0.1;
		scroll_down=0;
	}


	if (panleft==1 || (right_click==1 && scroll_right==1))
	{
		lx-=0.1;
		rx-=0.1;
	}
	if (panright==1 || (right_click==1 && scroll_left==1))
	{
		lx+=0.1;
		rx+=0.1;
	}	
	Matrices.projection = glm::ortho(lx, rx, dy, upy, 0.1f, 500.0f);

	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */


	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(sky);

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(speedbar);

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(ground);

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(fly);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateStick = glm::translate (glm::vec3(13, -6, 0));        // glTranslatef
	Matrices.model *= translateStick;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(stick);

	/*	Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateArrow = glm::translate (glm::vec3(-12.5, 0.25 + arrowy, 0));        // glTranslatef
		Matrices.model *= translateArrow;
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(arrow);
	 */	

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateStand = glm::translate (glm::vec3(-2, -6, 0));        // glTranslatef
	//		glm::mat4 translateStand = glm::translate (glm::vec3(x, y, 0)); 	// glTranslatef

	//	x=0;
	//	y=-6;
	//	while (x<5)
	//		x++;
	Matrices.model *= translateStand;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(stand);


	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	Matrices.model *= triangleTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	//  Don't change unless you are sure!!
	//draw3DObject(triangle);

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTriangle1 = glm::translate (glm::vec3(6, -5, 0)); // glTranslatef
	//glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangle1Transform = translateTriangle1;
	Matrices.model *= triangle1Transform; 
	MVP = VP * Matrices.model; // MVP = p * V * M
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(triangle1);


	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTriangle2 = glm::translate (glm::vec3(10, -5, 0.0f)); // glTranslatef
	//glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangle2Transform = translateTriangle2;
	Matrices.model *= triangle2Transform; 
	MVP = VP * Matrices.model; // MVP = p * V * M
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(triangle2);
	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(triangle);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(rectangle);

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateCircle = glm::translate (glm::vec3(-12, -6.50, 0));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateCircle;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(circle);


	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTarget1 = glm::translate (glm::vec3(0, -3.25, 0));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateTarget1;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if(t1==1)
	{
		draw3DObject(target1);
	}

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTarget2 = glm::translate (glm::vec3(8, -5, 0));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateTarget2;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if (t2==1)
	{
		draw3DObject(target2);
	}

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTarget3 = glm::translate (glm::vec3(9,4 , 0));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateTarget3;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if (t3==1)
	{
		draw3DObject(target3);
	}

	if (flag==1 || flag==2)
	{
		gaga=0;
		haha=0;
		//ay=0;
		//	cout<<"boo"<<endl;
		Matrices.model = glm::mat4(1.0f);
		bx=can_x + pos_x;
		by=can_y + pos_y;
		glm::mat4 translateBall1 = glm::translate (glm::vec3(bx, by, 0));        // glTranslatef
		Matrices.model *= translateBall1;

		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(ball1);

		if(flag == 1)
		{
			start_t = glfwGetTime();
			flag ++;
				ay=0;
		}
		curr_t=glfwGetTime();
		t=curr_t-start_t;
		//theta=canon_rotation;
		//	cout << pos_x << endl;
		//	cout << pos_y << endl;
		//		draw3DObject(ball1);
		/*	if (gaga==1)
			pos_x=-1*u*(cos(DEG2RAD(angle)))*t;
			else*/
		pos_x=u*(cos(DEG2RAD(angle)))*t;

		if (by>-7.25)
			pos_y=u*(sin(DEG2RAD(angle)))*t - 0.5*9.8*t*t;
		if (bx>16.0 || by<-8.0 || by>8.0 || bx<-16.0)
		{
			//		cout<<"yo"<<endl;
			flag=0;
			u=5;
			pos_x=0;
			pos_y=0;
			//	start_t=glfwGetTime();
		}
		if (by<=-7.25 )
		{
			int lala;
			lala=u - 0.1*9.8*t;

			//	cout << "this";
			//	cout << by << endl;
			by=pos_y;
			pos_x=u*cos(DEG2RAD(0))*t - 0.5*0.1*9.8*t*t;
			//	cout << "u";
			//	cout << lala << endl;
			if (lala<-.02)
			{
				bx=19.0;
				by=-10;
				pos_x=0;
				pos_y=0;
			}

		}
		if( (((bx)*(bx) +(by+3.25)*(by+3.25))*((bx)*(bx) +(by+3.25)*(by+3.25))) <=1.5625   )
		{
			if (t1=1)
				score1=1;
			t1=0;

			//score+=1;
			//cout << score << endl;
		}
		if( bx>6.75 && bx<9.25 && by<-3.75 && by>-6.25)
		{
			if (t2=1)
				score2=1;
			t2=0;
			//score+=1;
			//cout << score << endl; 
		}
		if( (((bx-9)*(bx-9) +(by-4)*(by-4))*((bx-9)*(bx-9) +(by-4)*(by-4))) <=1.5625   )
		{
			if (t3=1)
				score3=1;
			t3=0;
			//score+=1;
			//cout << score;
		}
		score=score1+score2+score3;
		//int haha=0;
		if (score==3)
		{
			over=1;
		}

		if ((bx >=-2 && bx<=1 && by<=-4 && by>=-6) || (by>=2.75 && by<=3.25 && bx>=7 && bx<=11) || (bx>=12 && bx<=13 && by >=-6 && by<=-2))
		{
			if (haha=0)
			{	can_x=0;
				can_y=0;
				start_t=glfwGetTime();
				haha=1;
			}
			curr_t=glfwGetTime();
			//	t=curr_t-start_t;
			//can_x=0
			pos_x=-1*u*cos(DEG2RAD(angle))*t;
			pos_y=u*(sin(DEG2RAD(angle)))*t - 0.5*9.8*t*t;
			gaga=1;
		}
		/*	if (bx>=-2 && bx<=1 && by>=-6 && by<=-4)
			{
			can_x=bx;
			can_y=by;
		//draw3Dobject(ball1);
		curr_t = glfwGetTime();
		start_t = glfwGetTime();
		t=curr_t - start_t;

		pos_x = u*cos(DEG2RAD(canon_rotation))*t;
		pos_y = u*sin(DEG2RAD(canon_rotation))*t - 0.5*9.8*t*t;

		u*=1;
		}
		 */

		/*		if ( bx>=-1.5 && by>=-6 && by<=-4 && yay!=1)

				{
		//			start_t=glfwGetTime();
		cout << "GRUEUGRGFUGFRWUGRYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYU" << endl;
		cout << bx << endl;
		cout << by << endl;
		cout << pos_x << endl;
		cout << pos_y << endl;

		yay = 1;
		gaga=1;
		pos_x=0;
		pos_y=0;

		can_x=-1.5;
		can_y=by;


		}
		if (gaga==1)
		{
		//			gaga=0;
		flag = 1;
		//t=curr_t-start_t;
		//bx+=pos_x;
		//by+=pos_y;
		//pos_x=u*cos(DEG2RAD(180))*t*0.01;
		//pos_y=u*sin(canon_rotation)*t - 0.5*9.8*t*t;
		//			t=curr_t - start_t;

		//pos_y -= 0.5*9.8*t*t;
		//bx += pos_x;
		//by += pos_y;
		cout <<"lalallalalala" << endl;
		cout << bx << endl;
		cout << by << endl;
		cout << pos_x << endl;
		cout << pos_y << endl;
		angle = 180;		

		pos_x=u*(cos(DEG2RAD(angle)))*t;
		//		if (by>-7.25)
		pos_y=u*(sin(DEG2RAD(angle)))*t - 0.5*9.8*t*t;
		//			pos_x=-u*cos(DEG2RAD(canon_rotation))*t;
		//			pos_y =( u*sin(DEG2RAD(canon_rotation)) - (0.5*9.8*t*t));
		//bx+=pos_x;
		//by+=pos_y;

		cout << "final bx" << bx << endl;
		cout << "final by" << by << endl;

		}

		 */
	}
	if(!flag)
	{
		if (flag_f==1)
		{
			if (ay<4.0)
				//		arrowy+=0.25;
				ay+=0.05;

			//cout << ay << endl;
			u+=0.2;
		}
		if (flag_s==1 && u>=0.2){
			if (ay>0)
				ay-=0.05;
			u-=0.2;
			//cout << ay << endl;
		}
	}

	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(base);

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateArrow = glm::translate (glm::vec3(-12.5, ay + 0.5, 0));        // glTranslatef
	Matrices.model *= translateArrow;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(arrow);


	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateCanon = glm::translate (glm::vec3(-12, -6.5, 0));      
	glm::mat4 rotateCanon = glm::rotate((float)(canon_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateCanon * rotateCanon);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(canon);

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
	canon_rotation = canon_rotation + increments*canon_rot_dir*canon_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window, scroll);
	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
	createBase();
	createCanon();
	createGround();
	createSky();
	//createBall1(0.5,0,0);
	createCircle (0.75, 0.0, 0.0);
	createBall1(0.5,0,0);
	createStick();
	createStand();
	createArrow();
	if (t1==1)
	{
		createTarget1(0.75, 0, 0);
	}
	if (t2==1)
	{
		createTarget2(0.75, 0, 0);
	}
	if (t3==1)
	{
		createTarget3(0.75, 0, 0);
	}
	createTriangle1();
	createTriangle2();
	createFly();
	createSpeedbar();
	cout << score << endl;
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
	

	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.0f, 1.0f, 0.30f, 1.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1200;
	int height = 600;
	//	cout << score << endl;
	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;
	//lala(window);
	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 0.1) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
		}
		//		cout << score << endl;
	}
	//	cout << score << endl;

	glfwTerminate();
	//cout << score << endl;
	exit(EXIT_SUCCESS);
}
