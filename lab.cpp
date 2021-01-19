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
void renderTerrainScene();
void renderSkydome();
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
GLuint sceneTexture, waterSampler, waterTexture, sceneSampler, cloudTexture, cloudSampler;


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
int particles_slider = 50;

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


#define g 9.80665f

void renderTerrainScene() {
	//Terrain Loading
	mat4 projectionMatrix = camera->projectionMatrix;
	mat4 viewMatrix = camera->viewMatrix;

	/*auto* scene = new Drawable("TerrainScenes/Small_Tropical_Island/Small_Tropical_Island.obj");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glUniform1i(sceneSampler, 1);

	glUseProgram(normalShaderProgram);
	scene->bind();

	scene->draw();*/
	
	glUseProgram(normalShaderProgram);

	glBindVertexArray(modelVAO);
	mat4 modelModelMatrix = mat4(1);
	mat4 modelMVP = projectionMatrix * viewMatrix * modelModelMatrix;
	glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &modelMVP[0][0]);
	glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelModelMatrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneTexture);
	glUniform1i(sceneSampler, 0);

	glDrawArrays(GL_TRIANGLES, 0, modelVertices.size());

}

//TO DO

//void renderSkydome() {
//	//Sky Loading
//	mat4 projectionMatrix = camera->projectionMatrix;
//	mat4 viewMatrix = camera->viewMatrix;
//
//
//	glUseProgram(normalShaderProgram);
//
//	glBindVertexArray(skyVAO);
//	mat4 skyModelMatrix = mat4(1);
//	mat4 skyMVP = projectionMatrix * viewMatrix * skyModelMatrix;
//	glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &skyMVP[0][0]);
//	glUniformMatrix4fv(MLocation, 1, GL_FALSE, &skyModelMatrix[0][0]);
//
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, skyTexture);
//	glUniform1i(skySampler, 0);
//
//	glDrawArrays(GL_TRIANGLES, 0, skyVertices.size());
//}

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//// MODEL STUFF
	// Get a pointer location to model matrix in the vertex shader
	MVPLocation = glGetUniformLocation(normalShaderProgram, "MVP");
	MLocation = glGetUniformLocation(normalShaderProgram, "M");



    waterSampler = glGetUniformLocation(particleShaderProgram, "texture0");
	//Attribution for water texture: <a href='https://www.freepik.com/photos/water'>Water photo created by rawpixel.com - www.freepik.com</a>
	waterTexture = loadSOIL("blue.jpg");


	cloudSampler = glGetUniformLocation(particleShaderProgram, "texture2");
	cloudTexture = loadSOIL("cloud.jpg");


	loadOBJWithTiny("TerrainScenes/Small_Tropical_Island/Small_Tropical_Island.obj", 
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
	
	sceneTexture = loadSOIL("TerrainScenes/Small_Tropical_Island/Maps/arl1b.jpg");
	sceneSampler = glGetUniformLocation(normalShaderProgram, "texture1");
	
    glfwSetKeyCallback(window, pollKeyboard);

	
}

void free() {

	glDeleteBuffers(1, &modelVerticiesVBO);
	glDeleteVertexArrays(1, &modelVAO);

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

	
	
	float t = glfwGetTime();
	vec3 lightPos = vec3(10, 10, 10);

	//TO DO - viewport of scene for the camera
	//camera->position = glm::vec3(box->size / 2, box->size / 2, 20);
    camera->position = vec3(10, 10, 10);
	
	//Find a more realistic obj in future
    auto* sphere = new Drawable("earth.obj");

	FountainEmitter f_emitter = FountainEmitter(sphere, particles_slider);
	
	auto* cloud = new Drawable("earth.obj");
	OrbitEmitter cloud_emitter = OrbitEmitter(cloud,10,5,6);
	//FountainEmitter cloud_emitter = FountainEmitter(cloud, particles_slider);
	

    
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
        if(!game_paused) {
            f_emitter.updateParticles(currentTime, dt, camera->position);
			cloud_emitter.updateParticles(currentTime, dt, camera->position);
		}

		//Particles draw
		f_emitter.renderParticles();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cloudTexture);
		glUniform1i(cloudSampler, 0);
		cloud_emitter.renderParticles();



        //*/
        
        //After rendering the first object - insert collision function
		//handleBoxSphereCollision(*box, *sphere);


        //*// Use standard drawing procedure

		/*auto* sceneObj = new Drawable("TerrainScenes/Small_Tropical_Island/Small_Tropical_Island.obj");
		
        glUseProgram(normalShaderProgram);
		sceneObj->bind();      
		sceneObj->draw();*/

		
        
        //*/

		/*auto* sceneObj = new Drawable("TerrainScenes/Small_Tropical_Island/Small_Tropical_Island.obj");
		sceneObj->draw();*/

		/*sphere->update(t, dt);
		glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere->modelMatrix[0][0]);
		sphere->draw();*/
		
		//renderScene();

		//displayGL();

		////Model Loading - passed to renderScene();
		/*glUseProgram(normalShaderProgram);

		glBindVertexArray(modelVAO);
		mat4 modelModelMatrix = mat4(1);
		mat4 modelMVP = projectionMatrix * viewMatrix * modelModelMatrix;
		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &modelMVP[0][0]);
		glUniformMatrix4fv(MLocation, 1, GL_FALSE, &modelModelMatrix[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneTexture);
		glUniform1i(sceneSampler, 0);

		glDrawArrays(GL_TRIANGLES, 0, modelVertices.size());*/
		
		renderTerrainScene();
		


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
     glEnable(GL_CULL_FACE);
	 glEnable(GL_FILL);
	 //
     glFrontFace(GL_CW);
     glFrontFace(GL_CCW);

    // enable point size when drawing points
    //glEnable(GL_PROGRAM_POINT_SIZE);

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
    } catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }

    return 0;
}


void windXManipulation() {
	particle.factorXWind += particle.factorXWind * 5.1f;

	particle.velocity = glm::vec3(5 - RAND*particle.factorXWind,
		-particle.speedYDroplet,
		5 - RAND*particle.factorZWind)*particle.speedDropFall;

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

