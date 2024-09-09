#pragma once
#include "Common/hint.hpp"

enum class EWindowPreset : UInt8
{
	None,
    OpenGL,
	Vulkan,
	Count,
};

class DisplayManager
{
private:
	IVector2 currentWindowSize;
	IVector2 currentFramebufferSize;
	Bool doesFramebufferResized;
	Handle<Window> currentWindow;
	
	HashMap<String, Handle<Window>> nameToIdWindows;
	DynamicArray<Window> windows;

	Array<DynamicArray<Hint>, UInt64(EWindowPreset::Count)> windowPresets;

public:
	DisplayManager(DisplayManager&) = delete;
	static DisplayManager &get();

	Void startup();

	Handle<Window> create_custom_window(const String &name, const IVector2 &size, const DynamicArray<Hint>& hints = {});
	Handle<Window> create_preset_window(const String &name, const IVector2 &size, EWindowPreset preset = EWindowPreset::None);
	Void set_current_window(const Handle<Window> handle);

	Window &get_window_by_name(const String& name);
	Window &get_window_by_handle(const Handle<Window> handle);

	[[nodiscard]]
	const Handle<Window> &get_window_handle_by_name(const String& name) const;
	[[nodiscard]]
	IVector2 get_framebuffer_size();
	[[nodiscard]]
	IVector2 get_window_size();
	[[nodiscard]]
	Float32	get_aspect_ratio() const;
	[[nodiscard]]
	Window &get_current_window();

	Void poll_events();
	[[nodiscard]]
	Bool should_window_close();

	Void shutdown();

private:
	DisplayManager()  = default;
	~DisplayManager() = default;

    static Void reset_hints();
	static Void s_framebuffer_resize_callback(Window window, Int32 width, Int32 height);
};

