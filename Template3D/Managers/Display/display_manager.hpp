#pragma once
#include "Common/hint.hpp"
#include "Common/window_preset.hpp"

struct GLFWwindow;

class DisplayManager
{
public:
	using Window = GLFWwindow*;

private:
	IVector2 currentWindowSize;
	IVector2 currentFramebufferSize;
	Bool doesFramebufferResized;
	Handle<Window> currentWindow;

	HashMap<String, Handle<Window>> windowsNameMap;
	DynamicArray<Window> windows;

	Array<DynamicArray<Hint>, UInt64(EWindowPreset::Count)> windowPresets;

public:
	Void startup();

	Handle<Window> create_custom_window(const String& name, const IVector2& size, const DynamicArray<Hint>& hints = {});
	Handle<Window> create_preset_window(const String& name, const IVector2& size, EWindowPreset preset = EWindowPreset::None);
	Void set_current_window(const Handle<Window> handle);

	Window& get_window(const String& name);
	Window& get_window(const Handle<Window> handle);

	[[nodiscard]]
	Handle<Window> get_window_handle(const String& name) const;
	[[nodiscard]]
	IVector2 get_framebuffer_size();
	[[nodiscard]]
	IVector2 get_window_size();
	[[nodiscard]]
	Float32	get_aspect_ratio() const;
	[[nodiscard]]
	Window& get_current_window();

	Void swap_buffers();
	Void poll_events();
	[[nodiscard]]
	Bool should_window_close();

	Void shutdown();

private:
	static Void reset_hints();
	static Void s_framebuffer_resize_callback(Window window, Int32 width, Int32 height);
};

