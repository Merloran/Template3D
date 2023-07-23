#include "Managers/Resource/resource_manager.hpp"


int main()
{
	SResourceManager& resourceManager = SResourceManager::get();

	resourceManager.startup();
	resourceManager.load_gltf_asset("Resources/Assets/Gun/weapon.gltf");

	
	return 0;
}