#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"
#include "animator.h"
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>
#define DR42_TRACE_IMPLEMENTATION
#include "trace.h"

#include <stdio.h>
#include <signal.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

camera_t* camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void segfaulter(int signum) {
    fprintf(stderr, "Segmentation fault\n");
    print_trace();
    exit(1);
}

void set_signal_handlers() {
    signal(SIGSEGV, segfaulter);
}

int main() {
    set_signal_handlers();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    vec3 cameraPos;
    cameraPos[0] = 0.0f;
    cameraPos[1] = 0.0f;
    cameraPos[2] = 3.0f;
    camera = camera_init(cameraPos);
    shader_t* shader = shader_init("src/assets/shaders/anim_model.vs", "src/assets/shaders/anim_model.fs");
    model_t* our_model = model_init("src/assets/models/dancing_vampire.dae", true);
    animation_t* danceAnimation = animation_init("src/assets/models/dancing_vampire.dae", our_model);
    animator_t* animator = animator_init(danceAnimation);
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        animator_update(animator, deltaTime);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader_use(shader);
        /*
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
glm::mat4 view = camera.GetViewMatrix();
ourShader.setMat4("projection", projection);
ourShader.setMat4("view", view);
        */
        mat4 projection;
        glm_perspective(glm_rad(camera->zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, projection);
        shader_set_mat4(shader, "projection", projection);
        mat4 view;
        camera_get_view_matrix(camera, view);
        shader_set_mat4(shader, "view", view);

        for (size_t i = 0; i < arrlenu(animator->final_bone_matrices); ++i) {
            char name[100];
            sprintf(name, "finalBonesMatrices[%d]", i);
            shader_set_mat4(shader, name, animator->final_bone_matrices[i]);
        }
        mat4 model;
        glm_mat4_identity(model);
        vec3 tranlation_vec;
        tranlation_vec[0] = 0.0f;
        tranlation_vec[1] = -1.75f;
        tranlation_vec[2] = 0.0f;
        glm_translate(model, tranlation_vec);
        vec3 scale_vec;
        scale_vec[0] = 0.5f;
        scale_vec[1] = 0.5f;
        scale_vec[2] = 0.5f;
        glm_scale(model, scale_vec);
        shader_set_mat4(shader, "model", model);
        model_draw(our_model, shader);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_process_keyboard(camera, FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_process_keyboard(camera, BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_process_keyboard(camera, LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_process_keyboard(camera, RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera_process_mouse_movement(camera, xoffset, yoffset, true);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera_process_mouse_scroll(camera, yoffset);
}
