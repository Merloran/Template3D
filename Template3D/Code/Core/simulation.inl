#pragma once
#include "simulation.hpp"
#include "../Managers/Render/render_manager.hpp"
#include "../Managers/Resource/resource_manager.hpp"

template <typename GraphicsAPI>
Void Simulation<GraphicsAPI>::startup()
{
    resourceManager.startup();
    displayManager.startup();
    renderManager.startup();
}

template <typename GraphicsAPI>
Void Simulation<GraphicsAPI>::shutdown()
{
    resourceManager.shutdown();
    displayManager.shutdown();
    renderManager.shutdown();
}
