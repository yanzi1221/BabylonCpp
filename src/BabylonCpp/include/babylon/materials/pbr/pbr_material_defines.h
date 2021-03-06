#ifndef BABYLON_MATERIALS_PBR_PBR_MATERIAL_DEFINES_H
#define BABYLON_MATERIALS_PBR_PBR_MATERIAL_DEFINES_H

#include <babylon/babylon_api.h>
#include <babylon/materials/iimage_processing_configuration_defines.h>
#include <babylon/materials/imaterial_detail_map_defines.h>

namespace BABYLON {

/**
 * @brief Manages the defines for the PBR Material.
 */
struct BABYLON_SHARED_EXPORT PBRMaterialDefines : public IImageProcessingConfigurationDefines,
                                                  public IMaterialDetailMapDefines {

  /**
   * @brief Initializes the PBR Material defines.
   */
  PBRMaterialDefines();
  ~PBRMaterialDefines() override; // = default

  /**
   * @brief Resets the PBR Material defines.
   */
  void reset() override;

  /**
   * @brief Converts the material define values to a string.
   * @returns - String of material define information.
   */
  [[nodiscard]] std::string toString() const override;

}; // end of struct PBRMaterialDefines

} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_PBR_PBR_MATERIAL_DEFINES_H
