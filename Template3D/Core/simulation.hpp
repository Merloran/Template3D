#pragma once
#include "../Managers/Render/Common/graphics_api_concept.hpp"
#include "../Managers/Display/display_manager.hpp"

template <GraphicsAPI GraphicsType>
class ResourceManager;
template <GraphicsAPI GraphicsType>
class RenderManager;

template <typename GraphicsAPI>
class Simulation
{
public:
    ResourceManager<GraphicsAPI> resourceManager;
    RenderManager<GraphicsAPI> renderManager;
    DisplayManager displayManager;

public:
    Void startup();

    Void shutdown();
};

#include "simulation.inl"
