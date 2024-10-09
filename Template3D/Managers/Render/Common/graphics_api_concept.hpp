#pragma once

template <typename GraphicsAPI>
class Simulation;
template <typename GraphicsAPI>
class Model;

template <typename Type>
concept GraphicsAPI = requires(Type api, Simulation<Type> &simulation, Model<Type> &model)
{
    { api.startup(simulation) } -> std::same_as<Void>;
    { api.draw_model(simulation, model) } -> std::same_as<Void>;
    { api.shutdown() } -> std::same_as<Void>;
};
