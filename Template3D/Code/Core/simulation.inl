#pragma once
#include "simulation.hpp"
// Do not remove this includes, even if intellisense screaming about it
#include "Render/render_manager.hpp"
#include "Resource/resource_manager.hpp"

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
