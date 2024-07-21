#include "okno.h"
#include<stdexcept>

namespace ep {

	EpOkno::EpOkno(int s, int v, std::string nazev) : sirka{s}, vyska{v}, nazevOkna{nazev} { //konstruktor
		initWindow();}

	EpOkno::~EpOkno() {
		glfwDestroyWindow(okno);
		glfwTerminate();}

	void EpOkno::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   //nevytvarej kontext - vytvari kontext pro OpenGL
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);		//nechceme aby se okno prizpusobovalo, implementováno jinde

		okno = glfwCreateWindow(sirka, vyska, nazevOkna.c_str(), nullptr, nullptr); //posledni dva parametry - 1 okno, ne fullscreen 2 - souvisí s opengl
		glfwSetWindowUserPointer(okno, this);
		glfwSetFramebufferSizeCallback(okno, framebufferResizeCallback);
	}
	void EpOkno::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, okno, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("nepovedlo se vytvorit okno");
		}
	}
	void EpOkno::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto lveWindow = reinterpret_cast<EpOkno*>(glfwGetWindowUserPointer(window));
		lveWindow->framebufferResized = true;
		lveWindow->sirka = width;
		lveWindow->vyska = height;
	};
}