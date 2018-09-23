#ifndef BABYLON_MESH_SIMPLIFICATION_REFERENCE_H
#define BABYLON_MESH_SIMPLIFICATION_REFERENCE_H

#include <babylon/babylon_api.h>

namespace BABYLON {

/**
 * @brief
 */
class BABYLON_SHARED_EXPORT Reference {

public:
  Reference(int vertexId, int triangleId);
  ~Reference();

public:
  int vertexId;
  int triangleId;

}; // end of class Reference

} // end of namespace BABYLON

#endif // end of BABYLON_MESH_SIMPLIFICATION_REFERENCE_H
