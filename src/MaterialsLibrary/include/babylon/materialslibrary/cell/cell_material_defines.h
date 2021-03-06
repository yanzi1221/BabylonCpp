#ifndef BABYLON_MATERIALS_LIBRARY_CELL_CELL_MATERIAL_DEFINES_H
#define BABYLON_MATERIALS_LIBRARY_CELL_CELL_MATERIAL_DEFINES_H

#include <babylon/babylon_api.h>
#include <babylon/materials/material_defines.h>

namespace BABYLON {
namespace MaterialsLibrary {

struct BABYLON_SHARED_EXPORT CellMaterialDefines : public MaterialDefines {

  CellMaterialDefines();
  ~CellMaterialDefines() override; // = default

}; // end of struct CellMaterialDefines

} // end of namespace MaterialsLibrary
} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_LIBRARY_CELL_CELL_MATERIAL_DEFINES_H
