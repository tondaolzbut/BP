#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace ep {

	class EpOkno {
	public:
		EpOkno(int w, int h, std::string name); //konstruktor
		~EpOkno();								//destruktor

		EpOkno(const EpOkno&) = delete;				
		EpOkno& operator=(const EpOkno&) = delete;

		bool oknoZavreno() { 
			return glfwWindowShouldClose(okno); }

		VkExtent2D getExtent() {
			return { static_cast<uint32_t>(sirka), static_cast<uint32_t>(vyska) }; }

		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }
		
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		int sirka;
		int vyska;
		bool framebufferResized = false;

		std::string nazevOkna;
		GLFWwindow* okno;
	};
}