#include "Managers/Resource/resource_manager.hpp"
#include "Managers/Display/display_manager.hpp"


int main()
{
	SResourceManager& resourceManager = SResourceManager::get();
	SDisplayManager& displayManager = SDisplayManager::get();

	displayManager.startup();
	resourceManager.startup();
	resourceManager.load_gltf_asset("Resources/Assets/Gun/weapon.gltf");

	while (!displayManager.should_window_close())
	{
		displayManager.update();
	}

	resourceManager.shutdown();
	displayManager.shutdown();
	return 0;
}