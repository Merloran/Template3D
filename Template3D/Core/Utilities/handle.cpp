#include "handle.hpp"
#include "../../Managers/Resource/Common/texture.hpp"
#include "../../Managers/Resource/Common/mesh.hpp"
#include "../../Managers/Resource/Common/model.hpp"
#include "../../Managers/Resource/Common/material.hpp"

const Handle<Model> Handle<Model>::NONE       = { UInt64(-1) };
const Handle<Mesh> Handle<Mesh>::NONE         = { UInt64(-1) };
const Handle<Material> Handle<Material>::NONE = { UInt64(-1) };
const Handle<Texture> Handle<Texture>::NONE   = { UInt64(-1) };
const Handle<Window> Handle<Window>::NONE     = { UInt64(-1) };