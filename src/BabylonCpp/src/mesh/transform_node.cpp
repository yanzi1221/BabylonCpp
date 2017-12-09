#include <babylon/mesh/transform_node.h>

#include <babylon/babylon_stl_util.h>
#include <babylon/bones/bone.h>
#include <babylon/cameras/camera.h>
#include <babylon/core/json.h>
#include <babylon/engine/scene.h>
#include <babylon/math/tmp.h>
#include <babylon/mesh/abstract_mesh.h>

namespace BABYLON {

unique_ptr_t<Vector3> TransformNode::_lookAtVectorCache
  = ::std::make_unique<Vector3>(0.f, 0.f, 0.f);
unique_ptr_t<Quaternion> TransformNode::_rotationAxisCache
  = ::std::make_unique<Quaternion>();

TransformNode::TransformNode(const string_t& name, Scene* scene, bool isPure)
    : Node{name, scene}
    , billboardMode{AbstractMesh::BILLBOARDMODE_NONE}
    , scalingDeterminant{1.f}
    , infiniteDistance{false}
    , position{Vector3::Zero()}
    , _poseMatrix{::std::make_unique<Matrix>(Matrix::Zero())}
    , _worldMatrix{::std::make_unique<Matrix>(Matrix::Zero())}
    , _scaling{Vector3::One()}
    , _isDirty{false}
    , _isWorldMatrixFrozen{false}
    , _rotation{Vector3::Zero()}
    , _rotationQuaternion{nullptr}
    , _transformToBoneReferal{nullptr}
    , _localWorld{Matrix::Zero()}
    , _absolutePosition{Vector3::Zero()}
    , _pivotMatrix{Matrix::Identity()}
    , _pivotMatrixInverse{}
    , _postMultiplyPivotMatrix{false}
    , _nonUniformScaling{false}
{
  if (isPure) {
    getScene()->addTransformNode(this);
  }
}

TransformNode::~TransformNode()
{
}

Vector3& TransformNode::rotation()
{
  return _rotation;
}

const Vector3& TransformNode::rotation() const
{
  return _rotation;
}

void TransformNode::setRotation(const Vector3& newRotation)
{
  _rotation = newRotation;
}

Vector3& TransformNode::scaling()
{
  return _scaling;
}

const Vector3& TransformNode::scaling() const
{
  return _scaling;
}

void TransformNode::setScaling(const Vector3& newScaling)
{
  _scaling = newScaling;
}

unique_ptr_t<Quaternion>& TransformNode::rotationQuaternion()
{
  return _rotationQuaternion;
}

const unique_ptr_t<Quaternion>& TransformNode::rotationQuaternion() const
{
  return _rotationQuaternion;
}

void TransformNode::setRotationQuaternion(
  const Nullable<Quaternion>& quaternion)
{
  _rotationQuaternion
    = quaternion ? ::std::make_unique<Quaternion>(*quaternion) : nullptr;
  // reset the rotation vector.
  if (quaternion && !stl_util::almost_equal(rotation().length(), 0.f)) {
    rotation().copyFromFloats(0.f, 0.f, 0.f);
  }
}

Matrix* TransformNode::getWorldMatrix()
{
  if (_currentRenderId != getScene()->getRenderId()) {
    computeWorldMatrix();
  }
  return _worldMatrix.get();
}

Matrix& TransformNode::worldMatrixFromCache()
{
  return *_worldMatrix.get();
}

const Matrix& TransformNode::worldMatrixFromCache() const
{
  return *_worldMatrix.get();
}

TransformNode& TransformNode::updatePoseMatrix(const Matrix& matrix)
{
  _poseMatrix->copyFrom(matrix);
  return *this;
}

Matrix& TransformNode::getPoseMatrix()
{
  return *_poseMatrix;
}
const Matrix& TransformNode::getPoseMatrix() const
{
  return *_poseMatrix;
}

bool TransformNode::_isSynchronized()
{
  if (_isDirty) {
    return false;
  }

  if (billboardMode != _cache.billboardMode
      || billboardMode != AbstractMesh::BILLBOARDMODE_NONE) {
    return false;
  }

  if (_cache.pivotMatrixUpdated) {
    return false;
  }

  if (infiniteDistance) {
    return false;
  }

  if (!_cache.position.equals(position))
    return false;

  if (rotationQuaternion()) {
    if (!_cache.rotationQuaternion.equals(*rotationQuaternion()))
      return false;
  }

  if (!_cache.rotation.equals(rotation())) {
    return false;
  }

  if (!_cache.scaling.equals(scaling())) {
    return false;
  }

  return true;
}

void TransformNode::_initCache()
{
  Node::_initCache();

  _cache.localMatrixUpdated = false;
  _cache.position           = Vector3::Zero();
  _cache.scaling            = Vector3::Zero();
  _cache.rotation           = Vector3::Zero();
  _cache.rotationQuaternion = Quaternion(0.f, 0.f, 0.f, 0.f);
  _cache.billboardMode      = AbstractMesh::BILLBOARDMODE_NONE;
}

TransformNode& TransformNode::_markAsDirty(const string_t& property)
{
  if (property == "rotation") {
    setRotationQuaternion(nullptr);
  }
  _currentRenderId = numeric_limits_t<int>::max();
  _isDirty         = true;
  return *this;
}

Vector3& TransformNode::absolutePosition()
{
  return _absolutePosition;
}

const Vector3& TransformNode::absolutePosition() const
{
  return _absolutePosition;
}

TransformNode& TransformNode::setPivotMatrix(Matrix& matrix,
                                             bool postMultiplyPivotMatrix)
{
  _pivotMatrix              = matrix;
  _cache.pivotMatrixUpdated = true;
  _postMultiplyPivotMatrix  = postMultiplyPivotMatrix;

  if (_postMultiplyPivotMatrix) {
    _pivotMatrixInverse = Matrix::Invert(matrix);
  }
  return *this;
}

Matrix& TransformNode::getPivotMatrix()
{
  return _pivotMatrix;
}

const Matrix& TransformNode::getPivotMatrix() const
{
  return _pivotMatrix;
}

TransformNode& TransformNode::freezeWorldMatrix()
{
  _isWorldMatrixFrozen
    = false; // no guarantee world is not already frozen, switch off temporarily
  computeWorldMatrix(true);
  _isWorldMatrixFrozen = true;
  return *this;
}

TransformNode& TransformNode::unfreezeWorldMatrix()
{
  _isWorldMatrixFrozen = false;
  computeWorldMatrix(true);
  return *this;
}

bool TransformNode::isWorldMatrixFrozen() const
{
  return _isWorldMatrixFrozen;
}

Vector3& TransformNode::getAbsolutePosition()
{
  computeWorldMatrix();
  return _absolutePosition;
}

TransformNode&
TransformNode::setAbsolutePosition(const Nullable<Vector3>& absolutePosition)
{
  if (!absolutePosition) {
    return *this;
  }
  auto absolutePositionX = (*absolutePosition).x;
  auto absolutePositionY = (*absolutePosition).y;
  auto absolutePositionZ = (*absolutePosition).z;

  if (parent()) {
    auto invertParentWorldMatrix = *parent()->getWorldMatrix();
    invertParentWorldMatrix.invert();
    Vector3 worldPosition{absolutePositionX, absolutePositionY,
                          absolutePositionZ};
    position
      = Vector3::TransformCoordinates(worldPosition, invertParentWorldMatrix);
  }
  else {
    position.x = absolutePositionX;
    position.y = absolutePositionY;
    position.z = absolutePositionZ;
  }
  return *this;
}

TransformNode& TransformNode::setPositionWithLocalVector(const Vector3& vector3)
{
  computeWorldMatrix();
  position = Vector3::TransformNormal(vector3, _localWorld);
  return *this;
}

Vector3 TransformNode::getPositionExpressedInLocalSpace()
{
  computeWorldMatrix();
  auto invLocalWorldMatrix = _localWorld;
  invLocalWorldMatrix.invert();

  return Vector3::TransformNormal(position, invLocalWorldMatrix);
}

TransformNode& TransformNode::locallyTranslate(const Vector3& vector3)
{
  computeWorldMatrix(true);
  position = Vector3::TransformCoordinates(vector3, _localWorld);
  return *this;
}

TransformNode& TransformNode::lookAt(const Vector3& targetPoint, float yawCor,
                                     float pitchCor, float rollCor, Space space)
{
  auto dv  = AbstractMesh::_lookAtVectorCache;
  auto pos = (space == Space::LOCAL) ? position : getAbsolutePosition();
  targetPoint.subtractToRef(pos, dv);
  auto yaw   = -::std::atan2(dv.z, dv.x) - Math::PI_2;
  auto len   = ::std::sqrt(dv.x * dv.x + dv.z * dv.z);
  auto pitch = ::std::atan2(dv.y, len);
  if (rotationQuaternion()) {
    Quaternion::RotationYawPitchRollToRef(yaw + yawCor, pitch + pitchCor,
                                          rollCor, *rotationQuaternion());
  }
  else {
    rotation().x = pitch + pitchCor;
    rotation().y = yaw + yawCor;
    rotation().z = rollCor;
  }
  return *this;
}

Vector3 TransformNode::getDirection(const Vector3& localAxis)
{
  auto result = Vector3::Zero();

  getDirectionToRef(localAxis, result);

  return result;
}

TransformNode& TransformNode::getDirectionToRef(const Vector3& localAxis,
                                                Vector3& result)
{
  Vector3::TransformNormalToRef(localAxis, *getWorldMatrix(), result);
  return *this;
}

TransformNode& TransformNode::setPivotPoint(Vector3& point, Space space)
{
  if (getScene()->getRenderId() == 0) {
    computeWorldMatrix(true);
  }

  auto wm = *getWorldMatrix();

  if (space == Space::WORLD) {
    auto& tmat = Tmp::MatrixArray[0];
    wm.invertToRef(tmat);
    point = Vector3::TransformCoordinates(point, tmat);
  }

  Vector3::TransformCoordinatesToRef(point, wm, position);
  _pivotMatrix.m[12]        = -point.x;
  _pivotMatrix.m[13]        = -point.y;
  _pivotMatrix.m[14]        = -point.z;
  _cache.pivotMatrixUpdated = true;
  return *this;
}

Vector3 TransformNode::getPivotPoint()
{
  auto point = Vector3::Zero();
  getPivotPointToRef(point);
  return point;
}

TransformNode& TransformNode::getPivotPointToRef(Vector3& result)
{
  result.x = -_pivotMatrix.m[12];
  result.y = -_pivotMatrix.m[13];
  result.z = -_pivotMatrix.m[14];
  return *this;
}

Vector3 TransformNode::getAbsolutePivotPoint()
{
  auto point = Vector3::Zero();
  getAbsolutePivotPointToRef(point);
  return point;
}

TransformNode& TransformNode::getAbsolutePivotPointToRef(Vector3& result)
{
  result.x = _pivotMatrix.m[12];
  result.y = _pivotMatrix.m[13];
  result.z = _pivotMatrix.m[14];
  getPivotPointToRef(result);
  Vector3::TransformCoordinatesToRef(result, *getWorldMatrix(), result);
  return *this;
}

TransformNode& TransformNode::setParent(TransformNode* node)
{
  if (node == nullptr) {
    auto& rotation = Tmp::QuaternionArray[0];
    auto& position = Tmp::Vector3Array[0];
    auto& scale    = Tmp::Vector3Array[1];

    if (parent()) {
      auto parentTransformNode = static_cast<TransformNode*>(parent());
      if (parentTransformNode) {
        parentTransformNode->computeWorldMatrix(true);
      }
    }
    computeWorldMatrix(true);
    getWorldMatrix()->decompose(scale, rotation, position);

    if (rotationQuaternion()) {
      (*rotationQuaternion()).copyFrom(rotation);
    }
    else {
      rotation.toEulerAnglesToRef(this->rotation());
    }

    this->scaling().x = scale.x;
    this->scaling().y = scale.y;
    this->scaling().z = scale.z;

    this->position.x = position.x;
    this->position.y = position.y;
    this->position.z = position.z;
  }
  else {
    auto& rotation        = Tmp::QuaternionArray[0];
    auto& position        = Tmp::Vector3Array[0];
    auto& scale           = Tmp::Vector3Array[1];
    auto& diffMatrix      = Tmp::MatrixArray[0];
    auto& invParentMatrix = Tmp::MatrixArray[1];

    computeWorldMatrix(true);
    node->computeWorldMatrix(true);

    node->getWorldMatrix()->invertToRef(invParentMatrix);
    getWorldMatrix()->multiplyToRef(invParentMatrix, diffMatrix);
    diffMatrix.decompose(scale, rotation, position);

    if (rotationQuaternion()) {
      (*rotationQuaternion()).copyFrom(rotation);
    }
    else {
      rotation.toEulerAnglesToRef(this->rotation());
    }

    this->position.x = position.x;
    this->position.y = position.y;
    this->position.z = position.z;

    this->scaling().x = scale.x;
    this->scaling().y = scale.y;
    this->scaling().z = scale.z;
  }

  Node::setParent(node);
  return *this;
}

bool TransformNode::nonUniformScaling() const
{
  return _nonUniformScaling;
}

bool TransformNode::_updateNonUniformScalingState(bool value)
{
  if (_nonUniformScaling == value) {
    return false;
  }

  _nonUniformScaling = true;
  return true;
}

TransformNode& TransformNode::attachToBone(Bone* bone,
                                           TransformNode* affectedTransformNode)
{
  _transformToBoneReferal = affectedTransformNode;
  Node::setParent(bone);

  if (bone->getWorldMatrix()->determinant() < 0.f) {
    scalingDeterminant *= -1.f;
  }
  return *this;
}

TransformNode& TransformNode::detachFromBone()
{
  if (!parent()) {
    return *this;
  }

  if (parent()->getWorldMatrix()->determinant() < 0.f) {
    scalingDeterminant *= -1.f;
  }
  _transformToBoneReferal = nullptr;
  setParent(nullptr);
  return *this;
}

TransformNode& TransformNode::rotate(Vector3& axis, float amount, Space space)
{
  axis.normalize();
  if (!rotationQuaternion()) {
    setRotationQuaternion(Quaternion::RotationYawPitchRoll(
      rotation().y, rotation().x, rotation().z));
    setRotation(Vector3::Zero());
  }
  Quaternion rotationQuaternion;
  if (space == Space::LOCAL) {
    rotationQuaternion = Quaternion::RotationAxisToRef(
      axis, amount, AbstractMesh::_rotationAxisCache);
    this->rotationQuaternion()->multiplyToRef(rotationQuaternion,
                                              *this->rotationQuaternion());
  }
  else {
    if (parent()) {
      auto invertParentWorldMatrix = *parent()->getWorldMatrix();
      invertParentWorldMatrix.invert();
      axis = Vector3::TransformNormal(axis, invertParentWorldMatrix);
    }
    rotationQuaternion = Quaternion::RotationAxisToRef(
      axis, amount, AbstractMesh::_rotationAxisCache);
    rotationQuaternion.multiplyToRef(*this->rotationQuaternion(),
                                     *this->rotationQuaternion());
  }
  return *this;
}

TransformNode& TransformNode::rotateAround(const Vector3& point, Vector3& axis,
                                           float amount)
{
  axis.normalize();
  if (!rotationQuaternion()) {
    setRotationQuaternion(Quaternion::RotationYawPitchRoll(
      rotation().y, rotation().x, rotation().z));
    rotation().copyFromFloats(0, 0, 0);
  }
  point.subtractToRef(position, Tmp::Vector3Array[0]);
  Matrix::TranslationToRef(Tmp::Vector3Array[0].x, Tmp::Vector3Array[0].y,
                           Tmp::Vector3Array[0].z, Tmp::MatrixArray[0]);
  Tmp::MatrixArray[0].invertToRef(Tmp::MatrixArray[2]);
  Matrix::RotationAxisToRef(axis, amount, Tmp::MatrixArray[1]);
  Tmp::MatrixArray[2].multiplyToRef(Tmp::MatrixArray[1], Tmp::MatrixArray[2]);
  Tmp::MatrixArray[2].multiplyToRef(Tmp::MatrixArray[0], Tmp::MatrixArray[2]);

  Tmp::MatrixArray[2].decompose(Tmp::Vector3Array[0], Tmp::QuaternionArray[0],
                                Tmp::Vector3Array[1]);

  position.addInPlace(Tmp::Vector3Array[1]);
  Tmp::QuaternionArray[0].multiplyToRef(*rotationQuaternion(),
                                        *rotationQuaternion());

  return *this;
}

TransformNode& TransformNode::translate(const Vector3& axis, float distance,
                                        Space space)
{
  auto displacementVector = axis.scale(distance);
  if (space == Space::LOCAL) {
    auto tempV3 = getPositionExpressedInLocalSpace().add(displacementVector);
    setPositionWithLocalVector(tempV3);
  }
  else {
    setAbsolutePosition(getAbsolutePosition().add(displacementVector));
  }
  return *this;
}

TransformNode& TransformNode::addRotation(float x, float y, float z)
{
  Quaternion rotationQuaternionTmp;
  if (rotationQuaternion()) {
    rotationQuaternionTmp = *rotationQuaternion();
  }
  else {
    rotationQuaternionTmp = Tmp::QuaternionArray[1];
    Quaternion::RotationYawPitchRollToRef(rotation().y, rotation().x,
                                          rotation().z, rotationQuaternionTmp);
  }
  auto& accumulation = Tmp::QuaternionArray[0];
  Quaternion::RotationYawPitchRollToRef(y, x, z, accumulation);
  rotationQuaternionTmp.multiplyInPlace(accumulation);
  if (!rotationQuaternion()) {
    rotationQuaternionTmp.toEulerAnglesToRef(rotation());
  }
  return *this;
}

Matrix& TransformNode::computeWorldMatrix(bool force)
{
  if (_isWorldMatrixFrozen) {
    return *_worldMatrix;
  }

  if (!force && isSynchronized(true)) {
    return *_worldMatrix;
  }

  _cache.position.copyFrom(position);
  _cache.scaling.copyFrom(scaling());
  _cache.pivotMatrixUpdated = false;
  _cache.billboardMode      = billboardMode;
  _currentRenderId          = getScene()->getRenderId();
  _isDirty                  = false;

  // Scaling
  Matrix::ScalingToRef(scaling().x * scalingDeterminant,
                       scaling().y * scalingDeterminant,
                       scaling().z * scalingDeterminant, Tmp::MatrixArray[1]);

  // Rotation

  // rotate, if quaternion is set and rotation was used
  if (rotationQuaternion()) {
    auto len = rotation().length();
    if (len != 0.f) {
      (*rotationQuaternion())
        .multiplyInPlace(Quaternion::RotationYawPitchRoll(
          rotation().y, rotation().x, rotation().z));
      rotation().copyFromFloats(0.f, 0.f, 0.f);
    }
  }

  if (rotationQuaternion()) {
    (*rotationQuaternion()).toRotationMatrix(Tmp::MatrixArray[0]);
    _cache.rotationQuaternion.copyFrom(*rotationQuaternion());
  }
  else {
    Matrix::RotationYawPitchRollToRef(rotation().y, rotation().x, rotation().z,
                                      Tmp::MatrixArray[0]);
    _cache.rotation.copyFrom(rotation());
  }

  // Translation
  auto& camera = getScene()->activeCamera;

  if (infiniteDistance && !parent() && camera) {

    auto cameraWorldMatrix = *camera->getWorldMatrix();

    Vector3 cameraGlobalPosition(cameraWorldMatrix.m[12],
                                 cameraWorldMatrix.m[13],
                                 cameraWorldMatrix.m[14]);

    Matrix::TranslationToRef(
      position.x + cameraGlobalPosition.x, position.y + cameraGlobalPosition.y,
      position.z + cameraGlobalPosition.z, Tmp::MatrixArray[2]);
  }
  else {
    Matrix::TranslationToRef(position.x, position.y, position.z,
                             Tmp::MatrixArray[2]);
  }

  // Composing transformations
  _pivotMatrix.multiplyToRef(Tmp::MatrixArray[1], Tmp::MatrixArray[4]);
  Tmp::MatrixArray[4].multiplyToRef(Tmp::MatrixArray[0], Tmp::MatrixArray[5]);

  // Billboarding (testing PG:http://www.babylonjs-playground.com/#UJEIL#13)
  if (billboardMode != AbstractMesh::BILLBOARDMODE_NONE && camera) {
    if ((billboardMode & AbstractMesh::BILLBOARDMODE_ALL)
        != AbstractMesh::BILLBOARDMODE_ALL) {
      // Need to decompose each rotation here
      auto& currentPosition = Tmp::Vector3Array[3];

      if (parent() && parent()->getWorldMatrix()) {
        if (_transformToBoneReferal) {
          parent()->getWorldMatrix()->multiplyToRef(
            *_transformToBoneReferal->getWorldMatrix(), Tmp::MatrixArray[6]);
          Vector3::TransformCoordinatesToRef(position, Tmp::MatrixArray[6],
                                             currentPosition);
        }
        else {
          Vector3::TransformCoordinatesToRef(
            position, *parent()->getWorldMatrix(), currentPosition);
        }
      }
      else {
        currentPosition.copyFrom(position);
      }

      currentPosition.subtractInPlace(camera->globalPosition());

      auto finalEuler = Tmp::Vector3Array[4].copyFromFloats(0.f, 0.f, 0.f);
      if ((billboardMode & AbstractMesh::BILLBOARDMODE_X)
          == AbstractMesh::BILLBOARDMODE_X) {
        finalEuler.x = ::std::atan2(-currentPosition.y, currentPosition.z);
      }

      if ((billboardMode & AbstractMesh::BILLBOARDMODE_Y)
          == AbstractMesh::BILLBOARDMODE_Y) {
        finalEuler.y = ::std::atan2(currentPosition.x, currentPosition.z);
      }

      if ((billboardMode & AbstractMesh::BILLBOARDMODE_Z)
          == AbstractMesh::BILLBOARDMODE_Z) {
        finalEuler.z = ::std::atan2(currentPosition.y, currentPosition.x);
      }

      Matrix::RotationYawPitchRollToRef(finalEuler.y, finalEuler.x,
                                        finalEuler.z, Tmp::MatrixArray[0]);
    }
    else {
      Tmp::MatrixArray[1].copyFrom(camera->getViewMatrix());

      Tmp::MatrixArray[1].setTranslationFromFloats(0, 0, 0);
      Tmp::MatrixArray[1].invertToRef(Tmp::MatrixArray[0]);
    }

    Tmp::MatrixArray[1].copyFrom(Tmp::MatrixArray[5]);
    Tmp::MatrixArray[1].multiplyToRef(Tmp::MatrixArray[0], Tmp::MatrixArray[5]);
  }

  // Local world
  Tmp::MatrixArray[5].multiplyToRef(Tmp::MatrixArray[2], _localWorld);

  // Parent
  if (parent() && parent()->getWorldMatrix()) {
    if (billboardMode != AbstractMesh::BILLBOARDMODE_NONE) {
      if (_transformToBoneReferal) {
        parent()->getWorldMatrix()->multiplyToRef(
          *_transformToBoneReferal->getWorldMatrix(), Tmp::MatrixArray[6]);
        Tmp::MatrixArray[5].copyFrom(Tmp::MatrixArray[6]);
      }
      else {
        Tmp::MatrixArray[5].copyFrom(*parent()->getWorldMatrix());
      }

      _localWorld.getTranslationToRef(Tmp::Vector3Array[5]);
      Vector3::TransformCoordinatesToRef(
        Tmp::Vector3Array[5], Tmp::MatrixArray[5], Tmp::Vector3Array[5]);
      _worldMatrix->copyFrom(_localWorld);
      _worldMatrix->setTranslation(Tmp::Vector3Array[5]);
    }
    else {
      if (_transformToBoneReferal) {
        _localWorld.multiplyToRef(*parent()->getWorldMatrix(),
                                  Tmp::MatrixArray[6]);
        Tmp::MatrixArray[6].multiplyToRef(
          *_transformToBoneReferal->getWorldMatrix(), *_worldMatrix);
      }
      else {
        _localWorld.multiplyToRef(*parent()->getWorldMatrix(), *_worldMatrix);
      }
    }
    _markSyncedWithParent();
  }
  else {
    _worldMatrix->copyFrom(_localWorld);
  }

  // Post multiply inverse of pivotMatrix
  if (_postMultiplyPivotMatrix) {
    _worldMatrix->multiplyToRef(_pivotMatrixInverse, *_worldMatrix);
  }

  // Normal matrix
  if (scaling().isNonUniform()) {
    _updateNonUniformScalingState(true);
  }
  else if (parent()) {
    auto parentTransformNode = static_cast<TransformNode*>(parent());
    if (parentTransformNode) {
      _updateNonUniformScalingState(parentTransformNode->_nonUniformScaling);
    }
  }
  else {
    _updateNonUniformScalingState(false);
  }

  _afterComputeWorldMatrix();

  // Absolute position
  _absolutePosition.copyFromFloats(_worldMatrix->m[12], _worldMatrix->m[13],
                                   _worldMatrix->m[14]);

  // Callbacks
  onAfterWorldMatrixUpdateObservable.notifyObservers(this);

  if (!_poseMatrix) {
    _poseMatrix = ::std::make_unique<Matrix>(Matrix::Invert(*_worldMatrix));
  }

  return *_worldMatrix;
}

void TransformNode::_afterComputeWorldMatrix()
{
}

TransformNode& TransformNode::registerAfterWorldMatrixUpdate(
  const ::std::function<void(TransformNode* mesh, EventState& es)>& func)
{
  onAfterWorldMatrixUpdateObservable.add(func);
  return *this;
}

TransformNode& TransformNode::unregisterAfterWorldMatrixUpdate(
  const ::std::function<void(TransformNode* mesh, EventState& es)>& func)
{
  onAfterWorldMatrixUpdateObservable.removeCallback(func);
  return *this;
}

TransformNode* TransformNode::clone(const string_t& /*name*/,
                                    Node* /*newParent*/,
                                    bool /*doNotCloneChildren*/)
{
  return nullptr;
}

Json::object
TransformNode::serialize(Json::object& /*currentSerializationObject*/)
{
  return Json::object();
}

TransformNode* TransformNode::Parse(const Json::value& /*parsedTransformNode*/,
                                    Scene* /*scene*/,
                                    const string_t& /*rootUrl*/)
{
  return nullptr;
}

void TransformNode::dispose(bool doNotRecurse)
{
  // Animations
  getScene()->stopAnimation(this);

  // Remove from scene
  getScene()->removeTransformNode(this);

  if (!doNotRecurse) {
    // Children
    auto objects = getDescendants(true);
    for (auto& object : objects) {
      object->dispose();
    }
  }
  else {
    auto childMeshes = getChildMeshes(true);
    for (auto& child : childMeshes) {
      child->setParent(nullptr);
      child->computeWorldMatrix(true);
    }
  }

  onAfterWorldMatrixUpdateObservable.clear();

  Node::dispose();
}

} // end of namespace BABYLON
