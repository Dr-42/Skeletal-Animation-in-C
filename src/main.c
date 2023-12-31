#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"
#include "animator.h"
#include "model.h"
#include "utils/heim_mat.h"
#include "utils/heim_vec.h"

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

const unsigned int SCR_WIDTH = 1980 / 2;
const unsigned int SCR_HEIGHT = 1080 / 2;
const float FPS = 60.0f;

camera_t* camera;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool full_screen = false;

void segfaulter(int signum) {
    fprintf(stderr, "Segmentation fault\n");
    print_trace();
    exit(signum);
}

void set_signal_handlers() {
    signal(SIGSEGV, segfaulter);
}

typedef struct BoneMatrices {
    HeimMat4 finalBonesMatrices[100];
} BoneMatrices;

animator_t* animator;

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
    HeimVec3f cameraPos = {0.0f, 0.0f, 2.5f};
    camera = camera_init(cameraPos);
    shader_t* shader = shader_init("src/assets/shaders/anim_model.vs", "src/assets/shaders/anim_model.fs");
    model_t* our_model = model_init("src/assets/models/Maria/Maria.fbx", true);

    model_set_albedo(our_model, "src/assets/models/Maria/textures/maria_diffuse.png");
    model_set_normal(our_model, "src/assets/models/Maria/textures/maria_normal.png");
    model_set_metallic(our_model, "src/assets/models/Maria/textures/maria_specular.png");

    animator = animator_init("src/assets/models/Maria/Maria.fbx", our_model);
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        processInput(window);
        animator_update(animator, deltaTime);
        //printf("FPS: %f\n", 1.0f / deltaTime);
        glClearColor(0.15f, 0.05f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader_use(shader);
        HeimMat4 projection = heim_mat4_perspective(camera->zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        HeimMat4 view = camera_get_view_matrix(camera);
        shader_set_mat4(shader, "projection", projection);
        shader_set_mat4(shader, "view", view);
        shader_set_int(shader, "numBones", (int)arrlenu(animator->final_bone_matrices));

        BoneMatrices boneMatrices;
        memcpy(boneMatrices.finalBonesMatrices, animator->final_bone_matrices, sizeof(HeimMat4) * arrlenu(animator->final_bone_matrices));
        GLuint ubo;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(boneMatrices), NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(boneMatrices), &boneMatrices);
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        HeimMat4 model = heim_mat4_identity();
        HeimVec3f tranlation_vec = {0.0f, -0.8f, 0.0f};
        model = heim_mat4_translate(model, tranlation_vec);
        HeimVec3f scale_vec = {0.009f, 0.009f, 0.009f};
        model = heim_mat4_scale(model, scale_vec);
        shader_set_mat4(shader, "model", model);
        model_draw(our_model, shader);
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (currentFrame - lastFrame < 1.0f / FPS) {
            glfwWaitEventsTimeout(1.0f / FPS - (currentFrame - lastFrame));
        }
        lastFrame = currentFrame;
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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera_process_keyboard(camera, UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera_process_keyboard(camera, DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
        //Full screen
        if(!full_screen){
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            size_t width = mode->width;
            size_t height = mode->height;
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 200, 200, width, height, GLFW_DONT_CARE);
            full_screen = true;
        }
        else{
            glfwSetWindowMonitor(window, NULL, 0, 0, SCR_WIDTH, SCR_HEIGHT, GLFW_DONT_CARE);
            full_screen = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        animator_switch_animation(animator, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        animator_switch_animation(animator, 1);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        animator_switch_animation(animator, 2);
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        animator_switch_animation(animator, 3);
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        animator_switch_animation(animator, 4);
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        animator_switch_animation(animator, 5);
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        animator_switch_animation(animator, 6);
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
        animator_switch_animation(animator, 7);
    }
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
        animator_switch_animation(animator, 8);
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        animator_switch_animation(animator, 9);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        //reset camera
        camera->position = (HeimVec3f){0.0f, 0.0f, 2.5f};
        camera->front = (HeimVec3f){0.0f, 0.0f, -1.0f};
        camera->up = (HeimVec3f){0.0f, 1.0f, 0.0f};
    }

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
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
    (void)window;
    (void)xoffset;
    camera_process_mouse_scroll(camera, yoffset);
}
