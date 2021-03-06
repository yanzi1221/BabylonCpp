#include <babylon/babylon_common.h>
#include <babylon/cameras/arc_rotate_camera.h>
#include <babylon/engines/scene.h>
#include <babylon/interfaces/irenderable_scene.h>
#include <babylon/lights/hemispheric_light.h>
#include <babylon/samples/babylon_register_sample.h>
#include <babylon/sprites/sprite_manager.h>

namespace BABYLON {
namespace Samples {

/**
 * @brief Wave Of Sprites Scene. Example demonstrating how to Load and display sprites.
 * @see https://www.babylonjs-playground.com/#C8ZSHY#1
 * @see https://doc.babylonjs.com/babylon101/sprites
 */
struct WaveOfSpritesScene : public IRenderableScene {

  WaveOfSpritesScene(ICanvas* iCanvas) : IRenderableScene(iCanvas)
  {
  }

  ~WaveOfSpritesScene() override = default;

  const char* getName() override
  {
    return "Wave Of Sprites Scene";
  }

  void initializeScene(ICanvas* canvas, Scene* scene) override
  {
    // The number of sprites
    auto count  = 100ul;
    auto countf = static_cast<float>(count);

    // Change the scene clear color
    scene->clearColor = Color3::Black();

    // Create a camera
    auto camera = ArcRotateCamera::New("Camera", 5.6f, 1.4f, 80.f,
                                       Vector3(countf / 2.f, 0.f, countf / 2.f), scene);
    camera->attachControl(canvas, true);

    // Create a light
    auto light       = HemisphericLight::New("hemi", Vector3(0.f, 1.f, 0.f), scene);
    light->intensity = 0.98f;

    // Creates wave of sprites
    auto spriteManager
      = SpriteManager::New("spriteManager", "/textures/blue_dot.png", count * count, 1, scene);

    const auto makeSprite = [&spriteManager](float x, float z) -> void {
      auto f               = (std::sin(x / 10.f) + std::sin(z / 10.f));
      auto instance        = Sprite::New("sprite", spriteManager);
      instance->position.x = x * 2.f;
      instance->position.z = z * 2.f;
      instance->position.y = f * 8.f;
      instance->size       = 0.2f;
      instance->color      = Color4(0.f, f + 1.2f, f + 2.f, 1.f);
    };

    for (auto x = 0ul; x < count; ++x) {
      for (auto z = 0ul; z < count; ++z) {
        makeSprite((float)x, (float)z);
      }
    }
  }

}; // end of struct WaveOfSpritesScene

BABYLON_REGISTER_SAMPLE("Special FX", WaveOfSpritesScene)

} // end of namespace Samples
} // end of namespace BABYLON
