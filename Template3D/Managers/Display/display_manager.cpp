#include "display_manager.hpp"

#include <GLFW/glfw3.h>

DisplayManager& DisplayManager::get()
{
	static DisplayManager instance;
	return instance;
}

Void DisplayManager::startup()
{
	SPDLOG_INFO("Display Manager startup.");
	if (!glfwInit())
	{
		SPDLOG_ERROR("Failed to initialize GLFW!");
		return;
	}


	windowPresets[UInt64(EWindowPreset::None)] =
	{
        { GLFW_CLIENT_API, GLFW_NO_API },
        { GLFW_RESIZABLE, GLFW_FALSE },
        { GLFW_VISIBLE, GLFW_FALSE },
	};
	windowPresets[UInt64(EWindowPreset::OpenGL)] =
	{
	    {GLFW_CONTEXT_VERSION_MAJOR, 4},
	    {GLFW_CONTEXT_VERSION_MINOR, 6},
	    {GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE},
	    {GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE},
	    {GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE},
	    {GLFW_RESIZABLE, GLFW_TRUE},
	};

	windowPresets[UInt64(EWindowPreset::Vulkan)] =
	{
		{ GLFW_CLIENT_API, GLFW_NO_API },
		{ GLFW_RESIZABLE, GLFW_FALSE },
	};

	doesFramebufferResized = false;
	currentWindow = create_preset_window("DefaultWindow", { 1, 1 }, EWindowPreset::None);
	set_current_window(currentWindow);
}

Handle<Window> DisplayManager::create_custom_window(const String& name, const IVector2& size, const DynamicArray<Hint>& hints)
{
	const auto iterator = nameToIdWindows.find(name);
	if (iterator != nameToIdWindows.end())
	{
		SPDLOG_ERROR("Window with name: {}, already exist.", name);
		return iterator->second;
	}

	reset_hints();
	for (const Hint& hint : hints)
	{
		glfwWindowHint(hint.hint, hint.value);
	}

	Handle<Window> handle = { windows.size() };
	Window &window = windows.emplace_back(glfwCreateWindow(size.x, 
                                                          size.y, 
                                                          name.c_str(), 
                                                          nullptr, 
                                                          nullptr));

	if (!window)
	{
		SPDLOG_ERROR("Failed to create window!");
		windows.pop_back();
		return Handle<Window>::NONE;
	}

	glfwSetFramebufferSizeCallback(window, s_framebuffer_resize_callback);
	glfwSetWindowUserPointer(window, this);

	nameToIdWindows[name] = handle;

	return handle;
}

Handle<Window> DisplayManager::create_preset_window(const String& name, const IVector2& size, EWindowPreset preset)
{
	return create_custom_window(name, size, windowPresets[UInt64(preset)]);
}

Void DisplayManager::set_current_window(const Handle<Window> handle)
{
	if (handle.id >= windows.size())
	{
		SPDLOG_WARN("Failed to change window, window with id {} does not exist", handle.id);
		return;
	}

	currentWindow = handle;
	doesFramebufferResized = true;
	glfwMakeContextCurrent(get_current_window());
}

Window& DisplayManager::get_window_by_name(const String& name)
{
	const auto& iterator = nameToIdWindows.find(name);
	if (iterator == nameToIdWindows.end() || iterator->second.id >= windows.size())
	{
		SPDLOG_WARN("Window {} not found, returned default.", name);
		return windows[0];
	}

	return windows[iterator->second.id];
}

Window& DisplayManager::get_window_by_handle(const Handle<Window> handle)
{
	if (handle.id >= windows.size())
	{
		SPDLOG_WARN("Window {} not found, returned default.", handle.id);
		return windows[0];
	}
	return windows[handle.id];
}

const Handle<Window>& DisplayManager::get_window_handle_by_name(const String& name) const
{
	const auto& iterator = nameToIdWindows.find(name);
	if (iterator == nameToIdWindows.end())
	{
		SPDLOG_WARN("Window handle {} not found, returned none.", name);
		return Handle<Window>::NONE;
	}
	return iterator->second;
}

IVector2 DisplayManager::get_framebuffer_size()
{
	glfwGetFramebufferSize(get_current_window(), &currentFramebufferSize.x, &currentFramebufferSize.y);
	return currentFramebufferSize;
}

IVector2 DisplayManager::get_window_size()
{
	glfwGetWindowSize(get_current_window(), &currentWindowSize.x, &currentWindowSize.y);
	return currentWindowSize;
}

Float32 DisplayManager::get_aspect_ratio() const
{
	if (currentWindowSize.y == 0)
	{
		return 0.0f;
	}

	return Float32(currentWindowSize.x) / Float32(currentWindowSize.y);
}

Window& DisplayManager::get_current_window()
{
	return windows[currentWindow.id];
}

Void DisplayManager::poll_events()
{
	glfwPollEvents();
	for (Window& window : windows)
	{
		if (!window)
		{
			continue;
		}

		glfwGetWindowSize(window, &currentWindowSize.x, &currentWindowSize.y);
		glfwGetFramebufferSize(window, &currentFramebufferSize.x, &currentFramebufferSize.y);
		if (window != get_current_window() && glfwWindowShouldClose(window))
		{
			glfwDestroyWindow(window);
			window = nullptr;
		}
	}
}

Bool DisplayManager::should_window_close()
{
	return glfwWindowShouldClose(get_current_window());
}

Void DisplayManager::shutdown()
{
	SPDLOG_INFO("Display Manager shutdown.");
	for (Window& window : windows)
	{
		if (window)
		{
			glfwDestroyWindow(window);
		}
	}
	windows.clear();

	glfwTerminate();
}

Void DisplayManager::reset_hints()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
}

Void DisplayManager::s_framebuffer_resize_callback(Window window, Int32 width, Int32 height)
{
	DisplayManager& displayManager = *static_cast<DisplayManager*>(glfwGetWindowUserPointer(window));
	displayManager.doesFramebufferResized = true;
}
