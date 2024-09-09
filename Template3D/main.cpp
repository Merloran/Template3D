#include "Managers/Resource/resource_manager.hpp"
#include "Managers/Display/display_manager.hpp"


Int32 main()
{
	ResourceManager& resourceManager = ResourceManager::get();
	DisplayManager& displayManager = DisplayManager::get();

	displayManager.startup();
	displayManager.set_current_window(displayManager.create_preset_window("Template3D", 
                                                                                { 1024, 768 }, 
                                                                                EWindowPreset::OpenGL));
	resourceManager.startup();

	while (!displayManager.should_window_close())
	{
		displayManager.poll_events();
	}

	resourceManager.shutdown();
	displayManager.shutdown();
	return 0;
}