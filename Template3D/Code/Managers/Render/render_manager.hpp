#pragma once
#include "Common/graphics_api_concept.hpp"

template <GraphicsAPI API>
class RenderManager
{
private:
    API api;

public:
    Void startup(Simulation<API>& simulation)
    {
        SPDLOG_INFO("Render Manager startup.");
        api.startup(simulation);
    }

    Void draw_model(Simulation<API>& simulation, Model<API> &model)
    {
        api.draw_model(simulation, model);
    }

    API& get_api()
    {
        return api;
    }

    Void shutdown()
    {
        SPDLOG_INFO("Render Manager shutdown.");
        api.shutdown();
    }
};