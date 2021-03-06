#ifndef BABYLON_ANIMATIONS_EASING_CUBIC_EASE_H
#define BABYLON_ANIMATIONS_EASING_CUBIC_EASE_H

#include <memory>

#include <babylon/animations/easing/easing_function.h>
#include <babylon/babylon_api.h>

namespace BABYLON {

class CubicEase;
using CubicEasePtr = std::shared_ptr<CubicEase>;

/**
 * @brief Easing function with a power of 3 shape (see link below).
 * @see https://easings.net/#easeInCubic
 * @see https://doc.babylonjs.com/babylon101/animations#easing-functions
 */
class BABYLON_SHARED_EXPORT CubicEase : public EasingFunction {

public:
  template <typename... Ts>
  static CubicEasePtr New(Ts&&... args)
  {
    return std::shared_ptr<CubicEase>(new CubicEase(std::forward<Ts>(args)...));
  }
  ~CubicEase() override; // = default

  /**
   * @brief Hidden
   */
  [[nodiscard]] float easeInCore(float gradient) const override;

protected:
  CubicEase() = default;

}; // end of class CubicEase

} // end of namespace BABYLON

#endif // end of BABYLON_ANIMATIONS_EASING_CUBIC_EASE_H
