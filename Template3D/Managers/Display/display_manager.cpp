#include "display_manager.hpp"

SDisplayManager& SDisplayManager::get()
{
	static SDisplayManager instance;
	return instance;
}

void SDisplayManager::startup()
{
}

void SDisplayManager::shutdown()
{
}
