#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace lve {

	class LveWindow {
	public:
		LveWindow(int w, int h, std::string name);
		~LveWindow();

		LveWindow(const LveWindow&) = delete; //cuz we use pointer to our window(glfw) glfwindow *window we deleting that
		LveWindow& operator=(const LveWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); } //to firstapp while(shouldclose()!)
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }

		GLFWwindow* getGLFWwindow() const { return window; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		int width;
		int height;
		bool framebufferResized = false; //usiing as a flag

		std::string windowName;
		GLFWwindow* window;
	};
}  // namespace lve