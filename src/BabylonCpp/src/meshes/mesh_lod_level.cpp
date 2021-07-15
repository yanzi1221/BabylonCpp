#include <babylon/meshes/mesh_lod_level.h>

#include <babylon/babylon_stl_util.h>
#include <babylon/meshes/mesh.h>

namespace BABYLON {

MeshLODLevel::MeshLODLevel(float iDistanceOrScreenCoverage, const MeshPtr& iMesh)
    : distanceOrScreenCoverage{iDistanceOrScreenCoverage}, mesh{iMesh}
{
}

MeshLODLevel::MeshLODLevel(const MeshLODLevel& other) = default;

MeshLODLevel::MeshLODLevel(MeshLODLevel&& other) = default;

MeshLODLevel& MeshLODLevel::operator=(const MeshLODLevel& other) = default;

MeshLODLevel& MeshLODLevel::operator=(MeshLODLevel&& other) = default;

MeshLODLevel::~MeshLODLevel() = default;

bool MeshLODLevel::operator==(const MeshLODLevel& other) const
{
  return stl_util::almost_equal(distanceOrScreenCoverage, other.distanceOrScreenCoverage)
         && (mesh == other.mesh);
}

bool MeshLODLevel::operator!=(const MeshLODLevel& other) const
{
  return !(operator==(other));
}

} // end of namespace BABYLON
