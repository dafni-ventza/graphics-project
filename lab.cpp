// Include C++ headers
#include <iostream>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <model.h>
#include <texture.h>
#include "FountainEmitter.h"
#include "OrbitEmitter.h"

//for terrain 
#include "terrainFiles/TerrainPCH.h"
#include "terrainFiles/Skydome.h"
#include "terrainFiles/Terrain.h"
//#include "gl/glut.h"

//TODO delete the includes afterwards
#include <chrono>
using namespace std::chrono;
//

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void renderScene();
void displayGL();
void windXManipulation();
void windZManipulation();

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Rain Apallaktiki"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint particleShaderProgram, normalShaderProgram, terrainShaderProgram;
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation, projectionAndViewMatrix;
GLuint translationMatrixLocation, rotationMatrixLocation, scaleMatrixLocation;
GLuint sceneTexture, waterSampler, waterTexture, sceneSampler;

//Terrain stuff
//Skydome g_SkyDome;
//Terrain g_Terrain(30, 2);

particleAttributes particle;

//Lighting for terrain
GLfloat g_LighDir[] = { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat g_LightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
GLfloat g_LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat g_LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat g_LighAttenuation0 = 1.0f;
GLfloat g_LighAttenuation1 = 0.0f;
GLfloat g_LighAttenuation2 = 0.0f;

glm::vec3 slider_emitter_pos(0.0f, 60.0f, 0.0f);
//Particles in the beginning... INCREASE IT
int particles_slider = 1;

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

bool game_paused = false;

bool use_sorting = false;
bool use_rotations = false;

//float height_threshold = W_HEIGHT / 2.0f;
float height_threshold = 1.0f;

//Model Scene Load
GLuint modelVAO, modelVerticiesVBO, planeVAO, planeVerticiesVBO;
std::vector<vec3> modelVertices, modelNormals;
std::vector<vec2> modelUVs;
GLuint MVPLocation, MLocation;

glm::vec4 background_color = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);

void renderScene() {
	//Scene Loading
	mat4 projectionMatrix = camera->projectionMatrix;
	mat4 viewMatrix = camera->viewMatrix;

	auto* scene = new Drawable("TerrainScenes/mountain/mount.blend1.obj");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glUniform1i(sceneSampler, 1);

	glUseProgram(normalShaderProgram);
	scene->bind();

	auto r = glm::rotate(glm::mat4(), 1.0f, glm::vec3(1, 1, 0));
	auto s = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
	auto tr = glm::translate(glm::mat4(), glm::vec3(RAND * 30 - 60, RAND * 30 - 60, RAND * 30 - 60));
	glUniformMatrix4fv(translationMatrixLocation, 1, GL_FALSE, &tr[0][0]);
	glUniformMatrix4fv(rotationMatrixLocation, 1, GL_FALSE, &r[0][0]);
	glUniformMatrix4fv(scaleMatrixLocation, 1, GL_FALSE, &s[0][0]);

	auto modelMatrix = tr * r * s;
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	scene->draw();



}

void renderHelpingWindow() {
	static int counter = 0;

	ImGui::Begin("Helper Window");                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("Use arrow keys for zoom in/out and A,W,S,D for moving around world space. Press ESC for exit.\n");               // Display some text (you can use a format strings too)

	ImGui::ColorEdit3("Background", &background_color[0]);

	ImGui::SliderFloat("x position", &slider_emitter_pos[0], -30.0f, 30.0f);
	ImGui::SliderFloat("y position", &slider_emitter_pos[1], -30.0f, 30.0f);
	ImGui::SliderFloat("z position", &slider_emitter_pos[2], -30.0f, 30.0f);
	ImGui::SliderFloat("height", &height_threshold, 0, 500);

	ImGui::SliderInt("particles", &particles_slider, 0, 20000);


	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Checkbox("Use sorting", &use_sorting);
	ImGui::Checkbox("Use rotations", &use_rotations);

	ImGui::Text("Performance %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void createContext() {
	particleShaderProgram = loadShaders(
		"ParticleShader.vertexshader",
		"ParticleShader.fragmentshader");

	normalShaderProgram = loadShaders(
		"StandardShading.vertexshader",
		"StandardShading.fragmentshader");

	//TO DO - separate shader for the scene to be loaded
	/*terrainShaderProgram = loadShaders(
		"TerrainShading.vertexshader",
		"TerrainShading.fragmentshader");*/

	projectionAndViewMatrix = glGetUniformLocation(particleShaderProgram, "PV");

	translationMatrixLocation = glGetUniformLocation(normalShaderProgram, "T");
	rotationMatrixLocation = glGetUniformLocation(normalShaderProgram, "R");
	scaleMatrixLocation = glGetUniformLocation(normalShaderProgram, "S");

	modelMatrixLocation = glGetUniformLocation(normalShaderProgram, "M");

	viewMatrixLocation = glGetUniformLocation(normalShaderProgram, "V");
	projectionMatrixLocation = glGetUniformLocation(normalShaderProgram, "P");


	// Draw wire frame triangles or fill: GL_LINE, or GL_FILL
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//// MODEL STUFF
	// Get a pointer location to model matrix in the vertex shader
	MVPLocation = glGetUniformLocation(normalShaderProgram, "MVP");
	MLocation = glGetUniformLocation(normalShaderProgram, "M");



	waterSampler = glGetUniformLocation(particleShaderProgram, "texture0");
	//Attribution for water texture: <a href='https://www.freepik.com/photos/water'>Water photo created by rawpixel.com - www.freepik.com</a>
	waterTexture = loadSOIL("blue.jpg");
	waterSampler = glGetUniformLocation(particleShaderProgram, "texture0");


	sceneSampler = glGetUniformLocation(normalShaderProgram, "texture1");
	sceneTexture = loadSOIL("TerrainScenes/mountain/ground_grass_3264_4062_Small.jpg");


	loadOBJWithTiny("TerrainScenes/mountain/mount.blend1.obj",
		modelVertices,
		modelUVs,
		modelNormals);
	glGenVertexArrays(1, &modelVAO);
	glBindVertexArray(modelVAO);
	glGenBuffers(1, &modelVerticiesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelVerticiesVBO);
	glBufferData(GL_ARRAY_BUFFER, modelVertices.size() * sizeof(glm::vec3),
		&modelVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glfwSetKeyCallback(window, pollKeyboard);


}

void free() {
	glDeleteProgram(particleShaderProgram);
	glDeleteProgram(normalShaderProgram);
	glfwTerminate();
}

void mainLoop() {



	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");



	////Terrain stuff
	//g_SkyDome.Init();
	//if (!g_SkyDome.LoadTexture("Data/Textures/skydome4.jpg"))
	//{
	//	std::cerr << "Failed to load texture for skydome!" << std::endl;
	//}
	//if (GLFW_KEY_F1)

	//	if (!g_Terrain.LoadHeightmap("Data/Terrain/terrain0-16bbp-257x257.raw", 16, 257, 257))
	//	{
	//		std::cerr << "Failed to load heightmap for terrain!" << std::endl;
	//	}
	//if (!g_Terrain.LoadTexture("Data/Textures/grass.jpg", 0))
	//{
	//	std::cerr << "Failed to load terrain texture for texture stage 0!" << std::endl;
	//}
	//if (!g_Terrain.LoadTexture("Data/Textures/rock.png", 1))
	//{
	//	std::cerr << "Failed to load terrain texture for texture stage 1!" << std::endl;
	//}
	//if (!g_Terrain.LoadTexture("Data/Textures/snow.jpg", 2))
	//{
	//	std::cerr << "Failed to load terrain texture for texture stage 2!" << std::endl;
	//}




	camera->position = vec3(0, 2, 10);

	//Find a more realistic obj in future
	auto* quad = new Drawable("earth.obj");

	FountainEmitter f_emitter = FountainEmitter(quad, particles_slider);
	OrbitEmitter o_emitter = OrbitEmitter(quad, particles_slider, 0.5f, 1.5f);

	float t = glfwGetTime();
	do {
		f_emitter.changeParticleNumber(particles_slider);
		f_emitter.emitter_pos = slider_emitter_pos;
		f_emitter.use_rotations = use_rotations;
		f_emitter.use_sorting = use_sorting;
		f_emitter.height_threshold = height_threshold;

		float currentTime = glfwGetTime();
		float dt = currentTime - t;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(particleShaderProgram);

		// camera
		camera->update();
		mat4 projectionMatrix = camera->projectionMatrix;
		mat4 viewMatrix = camera->viewMatrix;

		auto PV = projectionMatrix * viewMatrix;
		glUniformMatrix4fv(projectionAndViewMatrix, 1, GL_FALSE, &PV[0][0]);

		//*/ Use particle based drawing
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, waterTexture);
		glUniform1i(waterSampler, 0);
		if (!game_paused) {
			f_emitter.updateParticles(currentTime, dt, camera->position);
			o_emitter.updateParticles(currentTime, dt, camera->position);
		}

		//Particles draw
		f_emitter.renderParticles();
		o_emitter.renderParticles();

		//*/


		/*// Use standard drawing procedure
		glUseProgram(normalShaderProgram);
		monkey->bind();
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		for (int i = 0; i < particles_slider; i++) {
			auto p = f_emitter.p_attributes[i];
			//auto modelMatrix = glm::scale(mat4(1.0f), vec3(4.0f, 4.0f, 4.0f));
			auto r = glm::rotate(glm::mat4(), 1.0f, glm::vec3(1,1,0));
			auto s = glm::scale(glm::mat4(), glm::vec3(1,1,1));
			auto t = glm::translate(glm::mat4(), glm::vec3(RAND*30-60, RAND * 30 - 60, RAND * 30 - 60));

			glUniformMatrix4fv(translationMatrixLocation, 1, GL_FALSE, &t[0][0]);
			glUniformMatrix4fv(rotationMatrixLocation, 1, GL_FALSE, &r[0][0]);
			glUniformMatrix4fv(scaleMatrixLocation, 1, GL_FALSE, &s[0][0]);

			auto modelMatrix = t * r * s;
			glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

			monkey->draw();
		}
		//*/


		//renderScene();

		//displayGL();

		//Model
		/*glBindVertexArray(modelVAO);
		mat4 modelModelMatrix = mat4(1);
		mat4 modelMVP = projectionMatrix * viewMatrix * modelModelMatrix;
		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &modelMVP[0][0]);
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelModelMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, modelVertices.size());*/




		renderHelpingWindow();
		glfwPollEvents();
		glfwSwapBuffers(window);
		t = currentTime;

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


}

void initialize() {
	// Initialize GLFW
	if (!glfwInit()) {
		throw runtime_error("Failed to initialize GLFW\n");
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		throw runtime_error(string(string("Failed to open GLFW window.") +
			" If you have an Intel GPU, they are not 3.3 compatible." +
			"Try the 2.1 version.\n"));
	}

	glfwMakeContextCurrent(window);

	// Start GLEW extension handler
	glewExperimental = GL_TRUE;

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		glfwTerminate();
		throw runtime_error("Failed to initialize GLEW\n");
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Hide the mouse and enable unlimited movement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

	// Gray background color
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	//Terrain stuff until glColorMaterial
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, g_LightAmbient);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, g_LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, g_LightSpecular);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, g_LighAttenuation0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, g_LighAttenuation1);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, g_LighAttenuation2);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Cull triangles which normal is not towards the camera
	 /*glEnable(GL_CULL_FACE);
	 glFrontFace(GL_CW);
	 glFrontFace(GL_CCW);*/

	 // enable point size when drawing points
	glEnable(GL_PROGRAM_POINT_SIZE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Log
	logGLParameters();

	// Create camera
	camera = new Camera(window);

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		camera->onMouseMove(xpos, ypos);
		}
	);




}

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Pause
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		game_paused = !game_paused;
	}

	//Less droplets - L
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		particles_slider--;
		particle.position--;
	}
	//More droplets - M
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		particle.position++;
		particles_slider++;
	}

	//Add wind in X Axis - I
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		windXManipulation();

	}
	//More Wind in Z Axis - U
	if (key == GLFW_KEY_U && action == GLFW_PRESS) {
		windZManipulation();
		//particle.factorZWind += particle.factorZWind * 1.1f;

		//particle.velocity = glm::vec3(
		//	5 - RAND*particle.factorXWind,
		//	-particle.speedYDroplet,
		//	5 - RAND*particle.factorZWind)*particle.speedDropFall;


		//particle.rot_axis = glm::normalize(glm::vec3(
		//1 - particle.factorXWind * RAND,
		//1 - 2 * RAND,
		//1 - particle.factorZWind * RAND));

		///*std::cout << particle.velocity << std::endl;*/
		//std::cout << "Velocity - U: " << glm::to_string(particle.velocity) << std::endl;
		//std::cout << "Particle factorXWind - U: " << (particle.factorZWind) << std::endl;

	}


	if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			camera->active = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			camera->active = false;
		}

	}
}

int main(void) {
	try {
		initialize();
		createContext();



		mainLoop();
		free();
	}
	catch (exception& ex) {
		cout << ex.what() << endl;
		getchar();
		free();
		return -1;
	}

	return 0;
}

//Terrain additions
void DrawAxis(float fScale, glm::vec3 translate = glm::vec3(0))
{
	glPushAttrib(GL_ENABLE_BIT);

	//    glDisable( GL_DEPTH_TEST );
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(translate.x, translate.y, translate.z);
	glScalef(fScale, fScale, fScale);

	glBegin(GL_LINES);
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0);
		glVertex3f(1.0f, 0.0f, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
	}
	glEnd();

	glPopMatrix();
	glPopAttrib();
}


void windXManipulation() {
	particle.factorXWind += particle.factorXWind * 5.1f;

	particle.velocity = glm::vec3(5 - RAND * particle.factorXWind,
		-particle.speedYDroplet,
		5 - RAND * particle.factorZWind) * particle.speedDropFall;

	/*std::cout << particle.velocity << std::endl;*/
	std::cout << "Velocity - I: " << glm::to_string(particle.velocity) << std::endl;
	std::cout << "Particle factorXWind - I: " << (particle.factorXWind) << std::endl;
}

void windZManipulation() {
	particle.factorZWind += particle.factorZWind * 1.1f;

	//particle.velocity = glm::vec3(
	//	5 - RAND*particle.factorXWind,
	//	-particle.speedYDroplet,
	//	5 - RAND*particle.factorZWind)*particle.speedDropFall;


	//particle.rot_axis = glm::normalize(glm::vec3(
	//1 - particle.factorXWind * RAND,
	//1 - 2 * RAND,
	//1 - particle.factorZWind * RAND));

	///*std::cout << particle.velocity << std::endl;*/
	//std::cout << "Velocity - U: " << glm::to_string(particle.velocity) << std::endl;
	//std::cout << "Particle factorXWind - U: " << (particle.factorZWind) << std::endl;
}

void displayGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                   // Clear the color buffer, and the depth buffer.

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	cout << "DisplayGL - RenderTerrain";

	//g_SkyDome.Render();

	//DrawAxis( 20.0f, g_Camera.GetPivot() );

	glLightfv(GL_LIGHT0, GL_POSITION, g_LighDir);
	//g_Terrain.Render();

	glfwSwapBuffers(window);
	//glutSwapBuffers();
	//glutPostRedisplay();
	//g_Terrain.DebugRender();
}