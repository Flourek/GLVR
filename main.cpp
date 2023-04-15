#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

#include "opencv2/opencv.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "shader.h"
#include "camera.h"
#include <iostream>
#include "openvr.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
bool vr_enabled = true;

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 500;
const unsigned int RENDER_WIDTH =  4000;
const unsigned int RENDER_HEIGHT = 4000;

// camera


struct Cameras{
private:
    glm::vec3 worldup = {0.0f, 1.0f, 0.0f};

public:
    Camera left  = Camera(glm::vec3(0.0f, 0.0f, 3.0f), worldup);
    Camera right = Camera(glm::vec3(0.065f, 0.0f, 3.0f),  worldup);

    Camera *array[2] = {&left, &right};

} Cameras;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float imageAspect = 1.0f;

float vertices[] = {
        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,   1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, 0.0f,
};

// world space positions of our cubes
glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  -8.0f),
};

GLuint makeQuadTexture(const char* path){
    GLuint texture;
    cv::Mat image = cv::imread(path);
    cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
    imageAspect = (float) image.rows / image.cols;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL SteamVR", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(0);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 330" );

    // Initialize OpenVR
    if (vr::VR_IsHmdPresent() && vr_enabled) {
        auto VRError = vr::VRInitError_None;
        auto VRSystem = vr::VR_Init(&VRError, vr::VRApplication_Scene);

        if (VRError != vr::VRInitError_None){
            std::cout << "OpenVR initialization failed: " << vr::VR_GetVRInitErrorAsEnglishDescription(VRError) << std::endl;
            return 1;
        }

    }else{
        std::cout << "HMD not found" << std::endl;
    }



    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, RENDER_WIDTH, RENDER_HEIGHT);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);


    GLuint rightEyeTexture;
    glGenTextures(1, &rightEyeTexture);
    glBindTexture(GL_TEXTURE_2D, rightEyeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint leftEyeTexture;
    glGenTextures(1, &leftEyeTexture);
    glBindTexture(GL_TEXTURE_2D, leftEyeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Quad texture

    GLuint leftColor = makeQuadTexture("wR.jpg");
    GLuint rightColor = makeQuadTexture("wL.jpg");


    Shader ourShader("../camera.vs", "../camera.fs");
    ourShader.use();
//    ourShader.setInt("leftColor", 0);


    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, RENDER_WIDTH, RENDER_HEIGHT);
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        vr::HmdMatrix44_t steamProjectionMatrix{};

        for (Camera *cam : Cameras.array) {

            if(cam == &Cameras.left){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, leftEyeTexture, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glBindTexture(GL_TEXTURE_2D, leftColor);
                if(vr_enabled)
                    steamProjectionMatrix = vr::VRSystem()->GetProjectionMatrix(vr::Eye_Left, 0.1f, 100.0f);
            }

            if(cam == &Cameras.right){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rightEyeTexture, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glBindTexture(GL_TEXTURE_2D, rightColor);

                if(vr_enabled)
                    steamProjectionMatrix = vr::VRSystem()->GetProjectionMatrix(vr::Eye_Right, 0.1f, 100.0f);
            }



            // pass projection matrix to shader (note that in this case it could change every frame)
            glm::mat4 projection = glm::perspective(glm::radians(cam->Zoom), (float)RENDER_WIDTH / (float) RENDER_HEIGHT, 0.1f, 100.0f);

            if(vr_enabled){
                glm::mat4 openglMatrix = glm::transpose(glm::make_mat4(&steamProjectionMatrix.m[0][0]));

                // Convert the coordinate system
                glm::mat4 flipY(1.0f);
                flipY[1][1] = -1.0f;
                projection = flipY * openglMatrix;
            }

            glm::mat4 view = cam->GetViewMatrix();

            glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            model = glm::translate(model, cubePositions[0]);
            model = glm::scale(model, glm::vec3(1.0f, imageAspect, 1.0f) );
            ourShader.use();

            ourShader.setMat4("model", model);
            ourShader.setMat4("projection", projection);
            ourShader.setMat4("view", view);

            // Render quads
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // Render ImGui
        int flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;
        ImVec2 size = ImVec2(SCR_WIDTH / 2, SCR_HEIGHT);

        // Flip verically
        ImVec2 uv0 = {0, 1};
        ImVec2 uv1 = {1, 0};

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize( size );
        ImGui::Begin("LOL", nullptr, flags);
            ImGui::Image( (void*)(intptr_t) leftEyeTexture, size, uv0, uv1);
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH / 2,0));
        ImGui::SetNextWindowSize( size );
        ImGui::Begin("LOLXD", nullptr, flags);
            ImGui::Image( (void*)(intptr_t) rightEyeTexture, size, uv0, uv1 );
        ImGui::End();


        // Pass textures to OpenVR
        if(vr_enabled){

            vr::VRTextureBounds_t textureBounds = {0.0f, 0.0f, 1.0f, 1.0f};
            vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];

            vr::Texture_t leftEye = {(void *) (uintptr_t) leftEyeTexture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
            vr::Texture_t rightEye = {(void *) (uintptr_t) rightEyeTexture, vr::TextureType_OpenGL,
                                      vr::ColorSpace_Gamma};

            vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

            vr::EVRCompositorError error;
            error = vr::VRCompositor()->Submit(vr::Eye_Left, &leftEye, &textureBounds);
            error = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEye, &textureBounds);

        }

        // End of frame
        glfwPollEvents();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    for (Camera *cam : Cameras.array) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cam->ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam->ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam->ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam->ProcessKeyboard(RIGHT, deltaTime);

    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        Cameras.right.Position[0] += -0.5 * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        Cameras.right.Position[0] += 0.5 * deltaTime;

//    std::cout << Cameras.right.Position[0] << std::endl;


    float zoomSpeed = 0;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
        zoomSpeed = 3;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
        zoomSpeed = -3;
    }
    if(zoomSpeed){
        cubePositions[0][2] += zoomSpeed * deltaTime;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    for (Camera *cam : Cameras.array) {
        cam->ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    for (Camera *cam : Cameras.array) {
        cam->ProcessMouseScroll(static_cast<float>(yoffset));
    }
}