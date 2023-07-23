#include "display_manager.hpp"
#include <GLFW/glfw3.h>


SDisplayManager& SDisplayManager::get()
{
	static SDisplayManager instance;
	return instance;
}

void SDisplayManager::startup()
{
	SPDLOG_INFO("Display Manager startup.");
	if (!glfwInit())
	{
		SPDLOG_ERROR("Failed to initialize GLFW!");
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(windowSize.x, windowSize.y, name.c_str(), nullptr, nullptr);
}

const glm::ivec2& SDisplayManager::get_framebuffer_size()
{
	if (window == nullptr)
	{
		SPDLOG_ERROR("Window is null!");
		return { -1, -1 };
	}
	glfwGetFramebufferSize(window, &framebufferSize.x, &framebufferSize.y);
	return framebufferSize;
}

const glm::ivec2& SDisplayManager::get_window_size()
{
	if (window == nullptr)
	{
		SPDLOG_ERROR("Window is null!");
		return { -1, -1 };
	}
	glfwGetWindowSize(window, &windowSize.x, &windowSize.y);
	return windowSize;
}

void SDisplayManager::update()
{
	glfwPollEvents();
}

bool SDisplayManager::should_window_close()
{
	if (window == nullptr)
	{
		SPDLOG_ERROR("Window is null!");
		return true;
	}

	return glfwWindowShouldClose(window);
}

void SDisplayManager::shutdown()
{
	SPDLOG_INFO("Display Manager shutdown.");
	glfwDestroyWindow(window);
	glfwTerminate();
	window = nullptr;
}
