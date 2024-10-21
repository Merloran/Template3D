#include "simulation.hpp"
#include "Managers/Resource/resource_manager.hpp"
#include "Managers/Display/display_manager.hpp"
#include "Managers/Render/OpenGL/opengl_api.hpp"
#include "Managers/Render/Vulkan/vulkan_api.hpp"


Int32 main()
{
    Simulation<Vulkan> simulation;
    // simulation.startup();
    simulation.displayManager.startup();
    DisplayManager& displayManager = simulation.displayManager;
    simulation.resourceManager.startup();

    displayManager.set_current_window(displayManager.create_preset_window("Template3D", 
                                                                                { 1024, 768 }, 
                                                                                EWindowPreset::Vulkan));
    
    simulation.renderManager.startup(simulation);
    while (!displayManager.should_window_close())
    {
        displayManager.poll_events();
        simulation.renderManager.draw_model(simulation, simulation.resourceManager.get_default_model());
        displayManager.swap_buffers();
    }

    simulation.shutdown();
    return 0;
}
