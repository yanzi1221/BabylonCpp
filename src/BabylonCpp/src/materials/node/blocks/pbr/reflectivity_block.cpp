#include <babylon/materials/node/blocks/pbr/reflectivity_block.h>

#include <babylon/core/json_util.h>
#include <babylon/materials/node/node_material_build_state.h>
#include <babylon/materials/node/node_material_build_state_shared_data.h>
#include <babylon/materials/node/node_material_connection_point.h>
#include <babylon/materials/node/node_material_connection_point_custom_object.h>
#include <babylon/materials/node/node_material_defines.h>
#include <babylon/misc/string_tools.h>

namespace BABYLON {

ReflectivityBlock::ReflectivityBlock(const std::string& iName)
    : NodeMaterialBlock(iName, NodeMaterialBlockTargets::Fragment)
    , useAmbientOcclusionFromMetallicTextureRed{false}
    , useMetallnessFromMetallicTextureBlue{true}
    , useRoughnessFromMetallicTextureAlpha{false}
    , useRoughnessFromMetallicTextureGreen{true}
    , metallic{this, &ReflectivityBlock::get_metallic}
    , roughness{this, &ReflectivityBlock::get_roughness}
    , texture{this, &ReflectivityBlock::get_texture}
    , reflectivity{this, &ReflectivityBlock::get_reflectivity}
{
  _isUnique = true;
}

ReflectivityBlock::~ReflectivityBlock() = default;

void ReflectivityBlock::RegisterConnections(const ReflectivityBlockPtr& reflectivityBlock)
{
  reflectivityBlock->registerInput("metallic", NodeMaterialBlockConnectionPointTypes::Float, false,
                                   NodeMaterialBlockTargets::Fragment);
  reflectivityBlock->registerInput("roughness", NodeMaterialBlockConnectionPointTypes::Float, false,
                                   NodeMaterialBlockTargets::Fragment);
  reflectivityBlock->registerInput("texture", NodeMaterialBlockConnectionPointTypes::Color4, true,
                                   NodeMaterialBlockTargets::Fragment);

  reflectivityBlock->registerOutput("reflectivity", NodeMaterialBlockConnectionPointTypes::Object,
                                    NodeMaterialBlockTargets::Fragment,
                                    NodeMaterialConnectionPointCustomObject::New(
                                      "reflectivity", reflectivityBlock,
                                      NodeMaterialConnectionPointDirection::Output,
                                      [](const std::string& iName) -> ReflectivityBlockPtr {
                                        return ReflectivityBlock::New(iName);
                                      },
                                      "ReflectivityBlock"));
}

void ReflectivityBlock::initialize(NodeMaterialBuildState& state)
{
  state._excludeVariableName("baseColor");
  state._excludeVariableName("reflectivityOut");
  state._excludeVariableName("microSurface");
  state._excludeVariableName("roughness");
}

std::string ReflectivityBlock::getClassName() const
{
  return "ReflectivityBlock";
}

NodeMaterialConnectionPointPtr& ReflectivityBlock::get_metallic()
{
  return _inputs[0];
}

NodeMaterialConnectionPointPtr& ReflectivityBlock::get_roughness()
{
  return _inputs[1];
}

NodeMaterialConnectionPointPtr& ReflectivityBlock::get_texture()
{
  return _inputs[2];
}

NodeMaterialConnectionPointPtr& ReflectivityBlock::get_reflectivity()
{
  return _outputs[0];
}

std::string ReflectivityBlock::getCode(const std::string& aoIntensityVarName) const
{
  const auto metalRoughTexture
    = texture()->isConnected() ? texture()->connectedPoint()->associatedVariableName() : "null";

  // note: metallic F0 factor = 0.04
  const auto code = StringTools::printf(
    R"(vec3 baseColor = surfaceAlbedo;
       vec4 metallicReflectanceFactors = vec4(1.);
       reflectivityOutParams reflectivityOut;

       reflectivityBlock(
           vec4(%s, %s, 0., 0.),
       #ifdef METALLICWORKFLOW
           surfaceAlbedo,
           metallicReflectanceFactors,
       #endif
       #ifdef REFLECTIVITY
           vec3(0., 0., %s),
           %s,
       #endif
       #if defined(METALLICWORKFLOW) && defined(REFLECTIVITY)  && defined(AOSTOREINMETALMAPRED)
           aoOut.ambientOcclusionColor,
       #endif
       #ifdef MICROSURFACEMAP
           microSurfaceTexel, <== not handled!
       #endif
           reflectivityOut
       );

       float microSurface = reflectivityOut.microSurface;
       float roughness = reflectivityOut.roughness;

       #ifdef METALLICWORKFLOW
           surfaceAlbedo = reflectivityOut.surfaceAlbedo;
       #endif
       #if defined(METALLICWORKFLOW) && defined(REFLECTIVITY) && defined(AOSTOREINMETALMAPRED)
           aoOut.ambientOcclusionColor = reflectivityOut.ambientOcclusionColor;
       #endif\r\n
      )",
    metallic()->associatedVariableName().c_str(), roughness()->associatedVariableName().c_str(), //
    aoIntensityVarName.c_str(),                                                                  //
    metalRoughTexture.c_str());

  return code;
}

void ReflectivityBlock::prepareDefines(AbstractMesh* mesh, const NodeMaterialPtr& nodeMaterial,
                                       NodeMaterialDefines& defines, bool /*useInstances*/,
                                       const SubMeshPtr& /*subMesh*/)
{
  NodeMaterialBlock::prepareDefines(mesh, nodeMaterial, defines);

  defines.setValue("REFLECTIVITY", texture()->isConnected(), true);
  defines.setValue("AOSTOREINMETALMAPRED", useAmbientOcclusionFromMetallicTextureRed, true);
  defines.setValue("METALLNESSSTOREINMETALMAPBLUE", useMetallnessFromMetallicTextureBlue, true);
  defines.setValue("ROUGHNESSSTOREINMETALMAPALPHA", useRoughnessFromMetallicTextureAlpha, true);
  defines.setValue("ROUGHNESSSTOREINMETALMAPGREEN",
                   !useRoughnessFromMetallicTextureAlpha && useRoughnessFromMetallicTextureGreen,
                   true);
}

ReflectivityBlock& ReflectivityBlock::_buildBlock(NodeMaterialBuildState& state)
{
  if (state.target == NodeMaterialBlockTargets::Fragment) {
    state.sharedData->blocksWithDefines.emplace_back(shared_from_this());
  }

  return *this;
}

std::string ReflectivityBlock::_dumpPropertiesCode()
{
  std::string codeString = "";

  codeString += StringTools::printf("%s.useAmbientOcclusionFromMetallicTextureRed = %s;\r\n",
                                    _codeVariableName.c_str(),
                                    useAmbientOcclusionFromMetallicTextureRed ? "true" : "false");
  codeString += StringTools::printf("%s.useMetallnessFromMetallicTextureBlue = %s;\r\n",
                                    _codeVariableName.c_str(),
                                    useMetallnessFromMetallicTextureBlue ? "true" : "false");
  codeString += StringTools::printf("%s.useRoughnessFromMetallicTextureAlpha = %s;\r\n",
                                    _codeVariableName.c_str(),
                                    useRoughnessFromMetallicTextureAlpha ? "true" : "false");
  codeString += StringTools::printf("%s.useRoughnessFromMetallicTextureGreen = %s;\r\n",
                                    _codeVariableName.c_str(),
                                    useRoughnessFromMetallicTextureGreen ? "true" : "false");

  return codeString;
}

json ReflectivityBlock::serialize() const
{
  return nullptr;
}

void ReflectivityBlock::_deserialize(const json& /*serializationObject*/, Scene* /*scene*/,
                                     const std::string& /*rootUrl*/)
{
}

} // end of namespace BABYLON
