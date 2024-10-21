add_subdirectory(OpenGL)
add_subdirectory(Vulkan)

target_sources(Template3D PRIVATE
               render_manager.hpp
               Common/graphics_api_concept.hpp 
               Common/pipeline_type.hpp 
               Common/shader_type.hpp)
