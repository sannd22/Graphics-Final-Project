/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Sueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> cubeProg;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh[20];

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin;
	vec3 gMax;

	vec3 charMin;
	vec3 charMax;

	vec3 gaze;
	float phi;
	float theta;
	float deltaX;
	float deltaY;
	float prevX;
	float prevY;
	float cameraSpeed = 0.2;
	vec3 upVector;
	vec3 lookAtPoint;
	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame
	
	float lastFrameTime;
	float delta;

	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1; 
	shared_ptr<Texture> texture2;
	shared_ptr<Texture> texture3;
	shared_ptr<Texture> texture4;
	shared_ptr<Texture> texture5;
	shared_ptr<Texture> texture6;
	shared_ptr<Texture> texture7;

	//animation data
	float sTheta = 0;
	float gTrans;
	vec3 w;
	vec3 u;
	vec3 v;
	float sBeta = 0;
	float lightTrans;
	int move;
	float mat1;
	vec3 cameraPos;

	bool firstMouse = true;
	float lastX;
	float lastY;

	float playerRotX;
	float playerRotY;
	float playerX;
	float playerZ;
	bool flashON;
	vec3 initMin;
	vec3 initMax;


	unsigned int cubeMapTexture;

	vector<std::string> faces {
 		"northlight_rt.tga",
 		"northlight_lf.tga",
 		"northlight_up.jpg",
 		"northlight_dn.tga",
 		"northlight_ft.tga",
 		"northlight_bk.tga"
 	};

 	struct Box {
		vec3 m_vecMax;
		vec3 m_vecMin;
	};

	Box charBox;
	Box shelfBox[20];


 	struct Light {

	    vec3 position;  
	    vec3 ambient;
	    vec3 diffuse;
	    vec3 specular;
		
	    float constant;
	    float linear;
	    float quadratic;

	    vec3  direction;
	    float cutOff;
	};

	float boxTrans[56] = 
	{
		3.3, -2.0, -2.35, 0,
		3.3, -1.0, 2.35, 3.14159,
		5.1, -2.0, -2.35, 0,
		5.1, -2.0, 2.35, 3.14159,
		6.9, -2.0, -2.35, 0, 
		6.9, -2.0, 2.35, 3.14159,
		-3.3, -2.0, -2.35, 0,
		-3.3, -2.0, 2.35, 3.14159,
		-5.1, -2.0, -2.35, 0,
		-5.1, -2.0, 2.35, 3.14159,
		-6.9, -2.0, -2.35, 0,
		-6.9, -2.0, 2.35, 3.14159,
		3.3, -2.0, -2.35, 0,
		3.3, -2.0, 2.35, 3.14159
	}; 


	bool AABBtoAABB(Box tBox1, Box tBox2)
	{
	//Check if Box1's max is greater than Box2's min and Box1's min is less than Box2's max
    	return(tBox1.m_vecMax.x > tBox2.m_vecMin.x && 
    	tBox1.m_vecMin.x < tBox2.m_vecMax.x &&
    	tBox1.m_vecMax.y > tBox2.m_vecMin.y &&
    	tBox1.m_vecMin.y < tBox2.m_vecMax.y &&
    	tBox1.m_vecMax.z > tBox2.m_vecMin.z &&
    	tBox1.m_vecMin.z < tBox2.m_vecMax.z);
	}

	bool checkCollisions(Box shelf[20], Box char1) {
		for (int i = 0; i < 18; i++) {
			if (AABBtoAABB(shelf[i], char1)) {
				return true;
			}
		}
		return false;
	}

	//Create all bounding boxes
	void makeBoxes(Box boxes[20], float tran[56]) {
		for (int i = 0; i < 18; i ++) {
			if (tran[(i*4) + 3] != 0) {
				boxes[i].m_vecMin.x = tran[(i*4) + 0] + (gMin.x * 0.02);
				boxes[i].m_vecMin.y = tran[(i*4) + 1] + (gMin.y * 0.02);
				boxes[i].m_vecMin.z = tran[(i*4) + 2] + (gMin.z * 0.02) - 1;

				boxes[i].m_vecMax.x = tran[(i*4) + 0] + (gMax.x * 0.02);
				boxes[i].m_vecMax.y = tran[(i*4) + 1] + (gMax.y * 0.02);
				boxes[i].m_vecMax.z = tran[(i*4) + 2] + (gMax.z * 0.02) - 1;
			} else {
				boxes[i].m_vecMin.x = tran[(i*4) + 0] + (gMin.x * 0.02);
				boxes[i].m_vecMin.y = tran[(i*4) + 1] + (gMin.y * 0.02);
				boxes[i].m_vecMin.z = tran[(i*4) + 2] + (gMin.z * 0.02);

				boxes[i].m_vecMax.x = tran[(i*4) + 0] + (gMax.x * 0.02);
				boxes[i].m_vecMax.y = tran[(i*4) + 1] + (gMax.y * 0.02);
				boxes[i].m_vecMax.z = tran[(i*4) + 2] + (gMax.z * 0.02);
			}
		}
		boxes[14].m_vecMin.x = -2.3;
		boxes[14].m_vecMin.y = -2.0;
		boxes[14].m_vecMin.z = -20.0;

		boxes[14].m_vecMax.x = -1.7;
		boxes[14].m_vecMax.y = 2.0;
		boxes[14].m_vecMax.z = -1.5;

		boxes[15].m_vecMin.x = 1.7;
		boxes[15].m_vecMin.y = -2.0;
		boxes[15].m_vecMin.z = -20.0;

		boxes[15].m_vecMax.x = 2.3;
		boxes[15].m_vecMax.y = 2.0;
		boxes[15].m_vecMax.z = -1.5;

		boxes[16].m_vecMin = vec3( -0.65, -2.0, 11);
		boxes[16].m_vecMax = vec3(0.65, 2.0, 12);

		boxes[17].m_vecMin = vec3( -7.65, -2.0, 5);
		boxes[17].m_vecMax = vec3(-6.35, 2.0, 7);

	}


 	unsigned int createSky(string dir, vector<string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for(GLuint i = 0; i < faces.size(); i++) {
			unsigned char *data =
		 	stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
		 			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		 	} else {
		 		cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
		 	}
		 }
		 glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		 glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		 glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		 glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		 cout << " creating cube map any errors : " << glGetError() << endl;
		 return textureID;
	} 



	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (glfwGetKey(window, GLFW_KEY_A)) {
			float dx = 0.5 * sin(playerRotX - 3.145/2);
			float dz = 0.5 * cos(playerRotX - 3.145/2);
			playerX -= dx;
			playerZ -= dz;
			charBox = updateChar();
			charBox = updateChar();
			if (checkCollisions(shelfBox, charBox)) {
				playerX += dx;
				playerZ += dz;
				charBox = updateChar();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			float dx = 0.5 * sin(playerRotX - 3.145/2);
			float dz = 0.5 * cos(playerRotX - 3.145/2);
			playerX += dx;
			playerZ += dz;
			charBox = updateChar();
			if (checkCollisions(shelfBox, charBox)) {
				playerX -= dx;
				playerZ -= dz;
				charBox = updateChar();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			float dx = 0.5 * sin(playerRotX);
			float dz = 0.5 * cos(playerRotX);
			playerX += dx;
			playerZ += dz;
			charBox = updateChar();
			if (checkCollisions(shelfBox, charBox)) {
				playerX -= dx;
				playerZ -= dz;
				charBox = updateChar();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			float dx = 0.5 * sin(playerRotX);
			float dz = 0.5 * cos(playerRotX);
			playerX -= dx;
			playerZ -= dz;
			charBox = updateChar();
			if (checkCollisions(shelfBox, charBox)) {
				playerX += dx;
				playerZ += dz;
				charBox = updateChar();
			}
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS) {
			if (flashON) {
				flashON = false;
			} else {
				flashON = true;
			}
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			if (mat1 > 3) {
				mat1 = 0;
			} else {
				mat1 ++;
			}
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS) {
			lightTrans += 1.0;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_X && action == GLFW_PRESS) {
			if (move == 0) {
				move = 1;
			}
			else {
				move = 0;
			}
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		playerRotX -= (deltaX/12);
		playerRotY += (deltaY/12);
		if (playerRotY  > 1.39) {
			playerRotY  = 1.39;
		}
		else if (playerRotY < -1.39) {
			playerRotY = -1.39;
		}
	}

	void initTex(const std::string& resourceDirectory){
 		texture0 = make_shared<Texture>();
		texture0->setFilename(resourceDirectory + "/crate.jpg");
 		texture0->init();
 		texture0->setUnit(0);
 		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
 		texture1 = make_shared<Texture>(); 
		texture1->setFilename(resourceDirectory + "/world.jpg");
 		texture1->init();
 		texture1->setUnit(1);
 		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
 		texture2 = make_shared<Texture>(); 
		texture2->setFilename(resourceDirectory + "/grass.jpg");
 		texture2->init();
 		texture2->setUnit(2);
 		texture2->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
 		texture3 = make_shared<Texture>(); 
		texture3->setFilename(resourceDirectory + "/stone.jpg");
 		texture3->init();
 		texture3->setUnit(3);
 		texture3->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
 		texture4 = make_shared<Texture>(); 
		texture4->setFilename(resourceDirectory + "/stone_ground.jpg");
 		texture4->init();
 		texture4->setUnit(4);
 		texture4->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
 		texture5 = make_shared<Texture>(); 
		texture5->setFilename(resourceDirectory + "/white.jpg");
 		texture5->init();
 		texture5->setUnit(5);
 		texture5->setWrapModes(GL_TEXTURE_WRAP_S, GL_REPEAT);
 		texture6 = make_shared<Texture>(); 
		texture6->setFilename(resourceDirectory + "/white.jpg");
 		texture6->init();
 		texture6->setUnit(6);
 		texture6->setWrapModes(GL_TEXTURE_WRAP_S, GL_REPEAT);
 		texture7 = make_shared<Texture>(); 
		texture7->setFilename(resourceDirectory + "/Cage0.png");
 		texture7->init();
 		texture7->setUnit(7);
 		texture7->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}


	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
		
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("lightCol");

		
		prog->addUniform("positionFlash");
		prog->addUniform("constantFlash");
		prog->addUniform("linearFlash");
		prog->addUniform("quadraticFlash");
		prog->addUniform("directionFlash");
		prog->addUniform("cutOff");
		prog->addUniform("outerCutOff");
		prog->addUniform("viewPos");
		prog->addUniform("ambFlash");
		prog->addUniform("difFlash");
		prog->addUniform("specFlash");
		
		prog->addUniform("Texture0");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addUniform("skybox");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");

		cubeMapTexture = createSky(resourceDirectory + "/cracks/", faces);

		initTex(resourceDirectory);

		mat1 = 2;
		gTrans = 0;
		playerRotX = M_PI;
		playerX = 0;
		playerZ = 0;

		move = 1;
		lightTrans = 2;
		phi = 0;
		theta = -M_PI/2;
		prevX = 0;
		prevY = 0;

		upVector.x = 0;
		upVector.y = 1.0;
		upVector.z = 0;

		flashON = true;
		
		cameraPos   = glm::vec3(0.0f, 1.0f,  3.0f);
		

		
	}
	void initGeom(const std::string& resourceDirectory)
	{

		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)

 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/SmoothSphere.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[0] = make_shared<Shape>();
			mesh[0]->createShape(TOshapes[0]);
			mesh[0]->measure();
			mesh[0]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/cube.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[1] = make_shared<Shape>();
			mesh[1]->createShape(TOshapes[0]);
			mesh[1]->measure();
			mesh[1]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/shelf.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[2] = make_shared<Shape>();
			mesh[2]->createShape(TOshapes[0]);
			mesh[2]->measure();
			mesh[2]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/BOOKS.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[3] = make_shared<Shape>();
			mesh[3]->createShape(TOshapes[0]);
			mesh[3]->measure();
			mesh[3]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/MrHumpty.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[4] = make_shared<Shape>();
			mesh[4]->createShape(TOshapes[0]);
			mesh[4]->measure();
			mesh[4]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/dummy.obj").c_str());
                if (!rc) {
                        cerr << errStr << endl;
                } else {
                    for (int i = 0; i < 6; i++) {
                        mesh[i + 5] = make_shared<Shape>();
                        mesh[i + 5]->createShape(TOshapes[i]);
                        mesh[i + 5]->measure();
                        mesh[i + 5]->init();
                    }
                }
        
        rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/stonewall.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[11] = make_shared<Shape>();
			mesh[11]->createShape(TOshapes[0]);
			mesh[11]->measure();
			mesh[11]->init();
		}
		

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/skinny_creature.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[12] = make_shared<Shape>();
			mesh[12]->createShape(TOshapes[0]);
			mesh[12]->measure();
			mesh[12]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/ghost.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[14] = make_shared<Shape>();
			mesh[14]->createShape(TOshapes[0]);
			mesh[14]->measure();
			mesh[14]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/flashlight.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[13] = make_shared<Shape>();
			mesh[13]->createShape(TOshapes[0]);
			mesh[13]->measure();
			mesh[13]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/Cage.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[15] = make_shared<Shape>();
			mesh[15]->createShape(TOshapes[0]);
			mesh[15]->measure();
			mesh[15]->init();
		}

		rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/Headstone.obj").c_str());
        if (!rc) {
			cerr << errStr << endl;
		} else {
			mesh[16] = make_shared<Shape>();
			mesh[16]->createShape(TOshapes[0]);
			mesh[16]->measure();
			mesh[16]->init();
		}
		
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = mesh[2]->min.x;
		gMin.y = mesh[2]->min.y;
		gMin.z = mesh[2]->min.z;

		gMax.x = mesh[2]->max.x;
		gMax.y = mesh[2]->max.y;
		gMax.z = mesh[2]->max.z;

		//cout << gMin.x <<" "<<gMin.y <<" "<<gMin.z << "\n" << gMax.x <<" "<<gMax.y <<" " <<gMax.z << "\n";

		charMin.x = mesh[12]->min.x;
		charMin.y = mesh[12]->min.y;
		charMin.z = mesh[12]->min.z;

		charMax.x = mesh[12]->max.x;
		charMax.y = mesh[12]->max.y;
		charMax.z = mesh[12]->max.z;

		initMin.x = mesh[12]->min.x * 0.1;
		initMin.y = mesh[12]->min.y;
		initMin.z = mesh[12]->min.z * 0.1;

		initMax.x = mesh[12]->max.x * 0.1;
		initMax.y = mesh[12]->max.y;
		initMax.z = mesh[12]->max.z * 0.1;

		charBox.m_vecMin.x = charMin.x * 0.1;
		charBox.m_vecMin.y = -2.0;
		charBox.m_vecMin.z = charMin.z * 0.1;

		charBox.m_vecMax.x = charMax.x * 0.1;
		charBox.m_vecMax.y = 2.0;
		charBox.m_vecMax.z = charMax.z * 0.1;

		makeBoxes(shelfBox, boxTrans);
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}
	

   	void drawShelf(float x, float y, float z, float angle, std::__1::shared_ptr<MatrixStack> Model) {
   			texture0->bind(prog->getUniform("Texture0"));
   		//draw the shelf with these transforms
			Model->pushMatrix();
			  Model->translate(vec3(x, y, z));
			  Model->scale(vec3(0.02, 0.02, 0.02));
			  Model->rotate(angle,vec3(0,1,0));
			  setModel(prog, Model);
			  mesh[2]->draw(prog);
			Model->popMatrix();

			Model->pushMatrix();
				Model->translate(vec3(x, y, z));
				Model->scale(vec3(0.02, 0.02, 0.02));
			    Model->rotate(angle,vec3(0,1,0));
				setModel(prog, Model);
				mesh[3]->draw(prog);
			Model->popMatrix();

			texture0->unbind();

   	}

    void SetMaterial(int i) {
 		switch (i) {
 			case 0: //shiny blue plastic
 				glUniform3f(prog->getUniform("MatAmb"), 0.02, 0.04, 0.2);
 				glUniform3f(prog->getUniform("MatDif"), 0.0, 0.16, 0.9);
 				glUniform3f(prog->getUniform("MatSpec"), 0.14, 0.2, 0.8);
 				glUniform1f(prog->getUniform("shine"), 10.0);
 				break;
 			case 1: // flat grey
				 glUniform3f(prog->getUniform("MatAmb"), 0.13, 0.13, 0.14);
				 glUniform3f(prog->getUniform("MatDif"), 0.3, 0.3, 0.4);
				 glUniform3f(prog->getUniform("MatSpec"), 0.3, 0.3, 0.4);
				 glUniform1f(prog->getUniform("shine"), 4.0);
				 break;
			 case 2: //brass
				 glUniform3f(prog->getUniform("MatAmb"), 0.3294, 0.2235, 0.02745);
				 glUniform3f(prog->getUniform("MatDif"), 0.7804, 0.5686, 0.11373);
				 glUniform3f(prog->getUniform("MatSpec"), 0.9922, 0.941176, 0.80784);
				 glUniform1f(prog->getUniform("shine"), 27.9);
				 break;
			case 3: //Emerald
				 glUniform3f(prog->getUniform("MatAmb"), 0.0215, 0.1745, 0.0215);
				 glUniform3f(prog->getUniform("MatDif"), 0.07568, 0.61424, 0.07568);
				 glUniform3f(prog->getUniform("MatSpec"), 0.633, 0.727811, 0.633);
				 glUniform1f(prog->getUniform("shine"), 0.6);
				 break;
			case 4: //White Rubber
				glUniform3f(prog->getUniform("MatAmb"), 0.5, 0.55, 0.5);
				glUniform3f(prog->getUniform("MatDif"), 0.5, 0.5, 0.5);
				glUniform3f(prog->getUniform("MatSpec"), 0.7, 0.7, 0.7);
				glUniform1f(prog->getUniform("shine"), 0.25);
			case 5: //BLACK
				glUniform3f(prog->getUniform("MatAmb"), 0.0, 0.0, 0.0);
				glUniform3f(prog->getUniform("MatDif"), 0.0, 0.0, 0.0);
				glUniform3f(prog->getUniform("MatSpec"), 0.01, 0.01, 0.01);
				glUniform1f(prog->getUniform("shine"), 0.0875);
		}
	}

	void drawPlayer(float playerX, float playerZ, float playerRot, std::__1::shared_ptr<MatrixStack> Model) {
		texture6->bind(prog->getUniform("Texture0"));
		Model->pushMatrix();
			Model->translate(vec3(playerX, 0.0, playerZ));
			Model->scale(vec3(0.1, 0.1, 0.1));
			Model->rotate(playerRot, vec3(0, 1, 0));
			setModel(prog, Model);
			mesh[12]->draw(prog);
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(playerX, 0.03, playerZ));
			Model->scale(vec3(0.03, 0.03, 0.03));
			Model->rotate(playerRot, vec3(0, 1, 0));
			setModel(prog, Model);
			mesh[13]->draw(prog);
		Model->popMatrix();
		texture6->unbind();
	}

	void drawGhost(float x, float y, float z, float rot, std::__1::shared_ptr<MatrixStack> Model) {

		SetMaterial(3);
		Model->pushMatrix();
			
			Model->translate(vec3(x, y + (sTheta/ (randFloat(2.8f, 3.0f))), z));
			Model->scale(vec3(0.013, 0.013, 0.013));
			Model->rotate(rot, vec3(0, 1, 0));
			Model->rotate(-3.14159/2, vec3(1, 0, 0));
			
			Model->translate(vec3( (mesh[14]->min.x - mesh[14]->max.x)/2, (mesh[14]->min.y - mesh[14]->max.y)/2, (mesh[14]->min.z - mesh[14]->max.z)/2));

			setModel(prog, Model);
			mesh[14]->draw(prog);
		Model->popMatrix();

	}

	Box updateChar() {
		Box char1;
		char1.m_vecMin.x = initMin.x + playerX;
		char1.m_vecMin.y = charBox.m_vecMin.y;
		char1.m_vecMin.z = initMin.z + playerZ;

		char1.m_vecMax.x = initMax.x + playerX;
		char1.m_vecMax.y = charBox.m_vecMax.y;
		char1.m_vecMax.z = initMax.z + playerZ;
		return char1;
	}

	float randFloat(float l, float h)
	{	
		float r = rand() / (float) RAND_MAX;
		return (1.0f - r) * l + r * h;
	}

	void drawTomb(float x, float z, std::__1::shared_ptr<MatrixStack> Model) {
			texture7->bind(prog->getUniform("Texture0"));
			Model->pushMatrix();
			  Model->translate(vec3(x, -1.75, z));
			  Model->scale(vec3(0.02, 0.02, 0.02));
			  Model->rotate(-3.1415/2, vec3(1, 0, 0));
			  setModel(prog, Model);
			  mesh[16]->draw(prog);
			Model->popMatrix();
			texture7->unbind();

	}
	
	void render() {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		lookAtPoint.x = playerX;
		lookAtPoint.z = playerZ;

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		cameraSpeed = 20.0f * deltaTime;

		
		cameraPos.x = playerX + 2 * cos(-playerRotY) * sin(-playerRotX);
		cameraPos.y = 1.0 + 2 * sin(-playerRotY);
		cameraPos.z = playerZ - 2 * cos(-playerRotX);


		glm::mat4 view;
		view = glm::lookAt(cameraPos, lookAtPoint, upVector);

		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);
		

		// Draw a stack of cubes with indiviudal transforms
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(view));

		glUniform3f(prog->getUniform("lightCol"), 1, 0.54, 0);


		glUniform1f(prog->getUniform("constantFlash"), 1.0f);

		vec3 playerPos = vec3(playerX, 1.0, playerZ);
		vec3 dirFlash = normalize(vec3(-(cameraPos.x - playerX), -cameraPos.y - 0.5, -(cameraPos.z - playerZ)));

		glUniform3f(prog->getUniform("directionFlash"), dirFlash.x, dirFlash.y, dirFlash.z); //0.0f, 0.0f, -1.0f);
		glUniform1f(prog->getUniform("linearFlash"), 0.09f);
		glUniform1f(prog->getUniform("quadraticFlash"), 0.032f);

		glUniform3f(prog->getUniform("ambFlash"), 0.24725, 0.1995, 0.0745);
 		glUniform3f(prog->getUniform("difFlash"), 0.75164, 0.60648, 0.22648);
 		glUniform3f(prog->getUniform("specFlash"), 0.628281, 0.555802, 0.366065);
		
		glUniform1f(prog->getUniform("cutOff"), cos(glm::radians(12.5f)));
		glUniform1f(prog->getUniform("outerCutOff"), cos(glm::radians(35.5f)));

		if (flashON == false) {
			glUniform3f(prog->getUniform("positionFlash"), playerPos.x, -50.0, playerPos.z);
		} else {
			glUniform3f(prog->getUniform("positionFlash"), playerPos.x, playerPos.y, playerPos.z);
		}

		glUniform3f(prog->getUniform("viewPos"), playerX, 0.0, playerZ);

		charBox = updateChar();


		SetMaterial(mat1);
		// draw mesh 
		Model->pushMatrix();
			
			Model->loadIdentity();
			//Model->rotate(gTrans, vec3(0, 1, 0));
			
			Model->pushMatrix();
				
				Model->loadIdentity();
				Model->scale(vec3(100.0, 100.0, 100.0));

				cubeProg->bind();
				setModel(cubeProg, Model);
				glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())); 
				glDepthFunc(GL_LEQUAL); 
				glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE,value_ptr(view));
				glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()));
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
				mesh[1]->draw(cubeProg);
				
				glDepthFunc(GL_LESS);
				cubeProg->unbind(); 
			Model->popMatrix();
			
			prog->bind();
			// draw floor plane 
			for (float i = 0; i < 4; i ++) {
				texture4->bind(prog->getUniform("Texture0"));			
				Model->pushMatrix();
					Model->translate(vec3(-5 + (i * 5.2), -2.1, -20));
					Model->scale(vec3(5.0, 0.1, 3.0));
					Model->rotate(3.14/2, vec3(0, 1, 0));
					Model->translate(vec3( (mesh[11]->min.x - mesh[11]->max.x)/2, (mesh[11]->min.y - mesh[11]->max.y)/2, (mesh[11]->min.z - mesh[11]->max.z)/2));
					setModel(prog, Model);
					mesh[11]->draw(prog);
				Model->popMatrix();
				texture4->unbind();
			}

			texture2->bind(prog->getUniform("Texture0"));		
			/* Draw character */
			Model->pushMatrix();
			  Model->translate(vec3(5.0, -2.0, 5.0));
			  Model->scale(vec3(0.01, 0.01, 0.01));
			  setModel(prog, Model);
			  mesh[4]->draw(prog);
			Model->popMatrix();
			texture2->unbind();

			texture1->bind(prog->getUniform("Texture0"));
			/* Draw 2nd character */
			Model->pushMatrix();
			  Model->translate(vec3(-8.0, -2.0, 0.0));
			  Model->scale(vec3(0.012, 0.012, 0.012));
			  Model->rotate(-4.0, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[4]->draw(prog);
			Model->popMatrix();
			texture2->unbind();

			texture0->bind(prog->getUniform("Texture0"));
			/* Draw 3nd character */
			Model->pushMatrix();
			  Model->translate(vec3(6.0, -2.0, 0.0));
			  Model->scale(vec3(0.005, 0.005, 0.005));
			  Model->rotate(-2.0, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[4]->draw(prog);
			Model->popMatrix();
			texture0->unbind();
			
			// Build Walls
			texture3->bind(prog->getUniform("Texture0"));
			Model->pushMatrix();
			  Model->translate(vec3(3.5, -4.0, 17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(-3.5, -4.0, 17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(-10.5, -4.0, 17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(10.5, -4.0, 17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();

			Model->pushMatrix();
			  Model->translate(vec3(3.5, -4.0, -17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(-3.5, -4.0, -17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(-10.5, -4.0, -17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(10.5, -4.0, -17.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();

			Model->pushMatrix();
			  Model->translate(vec3(10.5, -4.0, 10.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  Model->rotate(3.14/2, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(10.5, -4.0, -2.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  Model->rotate(3.14/2, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(10.5, -4.0, -14.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  Model->rotate(3.14/2, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();

			Model->pushMatrix();
			  Model->translate(vec3(-10.5, -4.0, 10.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  Model->rotate(3.14/2, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(-10.5, -4.0, -2.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  Model->rotate(3.14/2, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();
			Model->pushMatrix();
			  Model->translate(vec3(-10.5, -4.0, -14.0));
			  Model->scale(vec3(0.5, 2.0, 1.0));
			  Model->rotate(3.14/2, vec3(0, 1, 0));
			  setModel(prog, Model);
			  mesh[11]->draw(prog);
			Model->popMatrix();

			texture3->unbind();


			



			drawShelf(3.3, -2.0, -2.35, 0, Model);
			drawShelf(3.3, -2.0, 2.35, 3.14159, Model);
			drawShelf(5.1, -2.0, -2.35, 0, Model);
			drawShelf(5.1, -2.0, 2.35, 3.14159, Model);
			drawShelf(6.9, -2.0, -2.35, 0, Model);
			drawShelf(6.9, -2.0, 2.35, 3.14159, Model);

			drawShelf(-3.3, -2.0, -2.35, 0, Model);
			drawShelf(-3.3, -2.0, 2.35, 3.14159, Model);
			drawShelf(-5.1, -2.0, -2.35, 0, Model);
			drawShelf(-5.1, -2.0, 2.35, 3.14159, Model);
			drawShelf(-6.9, -2.0, -2.35, 0, Model);
			drawShelf(-6.9, -2.0, 2.35, 3.14159, Model);

			drawShelf(-2.5, -2.0, -6.0, 1.57, Model);
			drawShelf(2.5, -2.0, -6.0, -1.57, Model);
			drawShelf(-2.5, -2.0, -7.80, 1.57, Model);
			drawShelf(2.5, -2.0, -7.80, -1.57, Model);

			drawShelf(-2.5, -2.0, -9.6, 1.57, Model);
			drawShelf(2.5, -2.0, -9.6, -1.57, Model);
			drawShelf(-2.5, -2.0, -11.4, 1.57, Model);
			drawShelf(2.5, -2.0, -11.4, -1.57, Model);

			drawShelf(-2.5, -2.0, -13.2, 1.57, Model);
			drawShelf(2.5, -2.0, -13.2, -1.57, Model);
			drawShelf(-2.5, -2.0, -15.0, 1.57, Model);
			drawShelf(2.5, -2.0, -15.0, -1.57, Model);

			drawShelf(-2.5, -2.0, -4.2, 1.57, Model);
			drawShelf(2.5, -2.0, -4.2, -1.57, Model);
			drawShelf(-2.5, -2.0, -2.4, 1.57, Model);
			drawShelf(2.5, -2.0, -2.4, -1.57, Model);
			drawShelf(2.5, -2.0, -2.4, 0, Model);


			SetMaterial(4);

			drawTomb(0, 12, Model);
			drawTomb(-7, 6, Model);
			drawPlayer(playerX, playerZ, playerRotX, Model);
			drawGhost(0, 0, 7, 3.14, Model);
			drawGhost(5, .3, 4, 0, Model);
			drawGhost(-3 + sBeta, 1.2, 6 - sBeta, 0, Model);
			drawGhost(0, 0, -5, 0, Model);
			drawGhost(-4, 2, 5, 0, Model);
			drawGhost(-7, 1, 5, 0, Model);
			drawGhost(9, .3, 4, 0, Model);

			drawGhost(3 + sBeta, 1.2, 16, 1.0, Model);
			drawGhost(2, 1 + sBeta, 8, 3.14, Model);
			drawGhost(-8.5, 3, 12, 2.4, Model);
			drawGhost(7 - sBeta, 0, 12 - sBeta, 4.2, Model);
			
		

		
		Model->popMatrix();

		prog->unbind();
		
		if (move == 1) {
			//animation update example
			sTheta = sin(glfwGetTime());
			sBeta = sTheta * (1/2.0);
		}
		// Pop matrix stacks.
		Projection->popMatrix();
		//View->popMatrix();

	}
	
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}

