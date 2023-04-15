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
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// camera



struct Cameras{
    Camera left = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    Camera right = Camera(glm::vec3(0.065f, 0.0f, 3.0f));

    Camera *array[2] = {&left, &right};
} Cameras;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

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

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH * 2, SCR_HEIGHT, "OpenGL SteamVR", NULL, NULL);
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

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 330" );

//#define vr_enabled
#ifdef vr_enabled
    if (vr::VR_IsHmdPresent()) {
        auto VRError = vr::VRInitError_None;
        auto VRSystem = vr::VR_Init(&VRError, vr::VRApplication_Scene);

        if (VRError != vr::VRInitError_None){
            std::cout << "OpenVR initialization failed: " << vr::VR_GetVRInitErrorAsEnglishDescription(VRError) << std::endl;
            return 1;
        }

    }else{
        std::cout << "HMD not found" << std::endl;
    }
#endif



    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH*2, SCR_HEIGHT);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);



    glfwSwapInterval(0);

    GLuint rightEyeTexture;
    glGenTextures(1, &rightEyeTexture);
    glBindTexture(GL_TEXTURE_2D, rightEyeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH*2, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint leftEyeTexture;
    glGenTextures(1, &leftEyeTexture);
    glBindTexture(GL_TEXTURE_2D, leftEyeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH*2, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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

    unsigned int texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    unsigned int chuj = 0xFF00FFFF;
    cv::Mat image = cv::imread("w.jpg");
    cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    Shader ourShader("../camera.vs", "../camera.fs");
    ourShader.use();
    ourShader.setInt("texture1", 0);


    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Camera camera;
        vr::HmdMatrix44_t steamProjectionMatrix;

        for (int i = 0; i < 2; ++i) {

            if( i == 0){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, leftEyeTexture, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                camera = Cameras.left;

                #ifdef vr_enabled
                    steamProjectionMatrix = vr::VRSystem()->GetProjectionMatrix(vr::Eye_Left, 0.1f, 100.0f);
                #endif
            }
            if( i == 1){
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rightEyeTexture, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                camera = Cameras.right;

                #ifdef vr_enabled
                    steamProjectionMatrix = vr::VRSystem()->GetProjectionMatrix(vr::Eye_Right, 0.1f, 100.0f);
                #endif

            }

            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // input
            // -----
            processInput(window);


            // bind textures on corresponding texture units
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);

            // activate shader
            ourShader.use();

            // pass projection matrix to shader (note that in this case it could change every frame)
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//
        #ifdef vr_enabled
            glm::mat4 openglMatrix = glm::transpose(glm::make_mat4(&steamProjectionMatrix.m[0][0]));

            // Convert the coordinate system
            glm::mat4 flipY(1.0f);
            flipY[1][1] = -1.0f;
            projection = flipY * openglMatrix;
        #endif

            ourShader.setMat4("projection", projection);

            glm::mat4 view = camera.GetViewMatrix();

            ourShader.setMat4("view", view);

            // render boxes
            glBindVertexArray(VAO);
            for (unsigned int i = 0; i < 1; i++)
            {
                // calculate the model matrix for each object and pass it to shader before drawing
                glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                model = glm::translate(model, cubePositions[i]);
                ourShader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize( ImVec2(SCR_WIDTH, SCR_HEIGHT) );
        ImGui::Begin("LOL",  nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::Image( (void*)(intptr_t) leftEyeTexture, ImVec2(SCR_WIDTH, SCR_HEIGHT));
        ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH,0));
        ImGui::SetNextWindowSize( ImVec2(SCR_WIDTH, SCR_HEIGHT) );
        ImGui::Begin("LOLXD",  nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::Image( (void*)(intptr_t) rightEyeTexture, ImVec2(SCR_WIDTH, SCR_HEIGHT));
        ImGui::End();


        // Pass textures to OpenVR
#ifdef vr_enabled

        if (vr::VR_IsHmdPresent()) {

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
#endif

        glfwPollEvents();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
        glfwSwapBuffers(window);


    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
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
        Cameras.right.Position[0] += -1 * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        Cameras.right.Position[0] += 1 * deltaTime;

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