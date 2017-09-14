#ifndef BABYLON_ANIMATIONS_ANIMATABLE_H
#define BABYLON_ANIMATIONS_ANIMATABLE_H

#include <babylon/babylon_global.h>

namespace BABYLON {

class BABYLON_SHARED_EXPORT Animatable {

public:
  Animatable(Scene* scene, IAnimatable* target, int fromFrame = 0,
             int toFrame = 100, bool loopAnimation = false,
             float speedRatio                              = 1.f,
             const ::std::function<void()>& onAnimationEnd = nullptr,
             const vector_t<Animation*>& animations = vector_t<Animation*>());
  ~Animatable();

  /** Methods **/
  vector_t<Animation*>& getAnimations();
  void appendAnimations(IAnimatable* target,
                        const vector_t<Animation*>& animations);
  Animation* getAnimationByTargetProperty(const string_t& property) const;
  void reset();
  void enableBlending(float blendingSpeed);
  void disableBlending();
  void goToFrame(int frame);
  void pause();
  void restart();
  void stop(const string_t& animationName = "");
  bool _animate(const millisecond_t& delay);

public:
  IAnimatable* target;
  bool animationStarted;
  int fromFrame;
  int toFrame;
  bool loopAnimation;
  float speedRatio;
  ::std::function<void()> onAnimationEnd;

private:
  millisecond_t _localDelayOffset;
  millisecond_t _pausedDelay;
  vector_t<Animation*> _animations;
  bool _paused;
  Scene* _scene;

}; // end of class Animatable

} // end of namespace BABYLON

#endif // end of BABYLON_ANIMATIONS_ANIMATABLE_H
