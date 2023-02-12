#include "lve_window.hpp"

// std
#include <stdexcept>

namespace lve {

    LveWindow::LveWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
        initWindow();
    }

    LveWindow::~LveWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LveWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); //true to resize the window

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr); //more monitors,sharing searching on xcode
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback); //every resize call this func
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void LveWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
        lveWindow->framebufferResized = true; //the flag
        lveWindow->width = width;
        lveWindow->height = height;
    }

}  // namespace lve