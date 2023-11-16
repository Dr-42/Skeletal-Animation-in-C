#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_DS_IMPLEMENTATION
#include <stb/stb_ds.h>
#include <cglm/cglm.h>

#include <stdio.h>
#include <stdint.h>

void framebuffer_size_callback(GLFWwindow* window, int32_t width, int32_t height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow* window);

const uint32_t SCR_WIDTH = 800;
const uint32_t SCR_HEIGHT = 600;

int main() {
	printf("Hello World!\n");
	return 0;
}
