#ifndef BABYLON_MATERIALS_TEXTURES_MULTI_RENDER_TARGET_H
#define BABYLON_MATERIALS_TEXTURES_MULTI_RENDER_TARGET_H

#include <babylon/babylon_api.h>
#include <babylon/materials/textures/imulti_render_target_options.h>
#include <babylon/materials/textures/render_target_texture.h>

namespace BABYLON {

/**
 * @brief A multi render target, like a render target provides the ability to render to a texture.
 * Unlike the render target, it can render to several draw buffers in one draw.
 * This is specially interesting in deferred rendering or for any effects requiring more than
 * just one color from a single pass.
 */
class BABYLON_SHARED_EXPORT MultiRenderTarget : public RenderTargetTexture {

public:
  /**
   * @brief Instantiate a new multi render target texture.
   * A multi render target, like a render target provides the ability to render
   * to a texture. Unlike the render target, it can render to several draw
   * buffers in one draw. This is specially interesting in deferred rendering or
   * for any effects requiring more than just one color from a single pass.
   * @param name Define the name of the texture
   * @param size Define the size of the buffers to render to
   * @param count Define the number of target we are rendering into
   * @param scene Define the scene the texture belongs to
   * @param options Define the options used to create the multi render target
   */
  MultiRenderTarget(const std::string& name, const std::variant<int, RenderTargetSize, float>& size,
                    size_t count, Scene* scene,
                    const std::optional<IMultiRenderTargetOptions>& options = std::nullopt);
  ~MultiRenderTarget() override; // = default

  /**
   * @brief Hidden
   */
  void _rebuild(bool forceFullRebuild = false) override;

  /**
   * @brief Replaces a texture within the MRT.
   * @param texture The new texture to insert in the MRT
   * @param index The index of the texture to replace
   */
  void replaceTexture(const TexturePtr& texture, unsigned int index);

  /**
   * @brief Resize all the textures in the multi render target.
   * Be careful as it will recreate all the data in the new texture.
   * @param size Define the new size
   */
  void resize(Size size);

  /**
   * @brief Changes the number of render targets in this MRT.
   * Be careful as it will recreate all the data in the new texture.
   * @param count new texture count
   * @param options Specifies texture types and sampling modes for new textures
   */
  virtual void updateCount(size_t count,
                           const std::optional<IMultiRenderTargetOptions>& options = std::nullopt);

  /**
   * @brief Dispose the render targets and their associated resources.
   */
  void dispose() override;

  /**
   * @brief Release all the underlying texture used as draw buffers.
   */
  void releaseInternalTextures();

protected:
  /**
   * @brief Get if draw buffers are currently supported by the used hardware and browser.
   */
  bool get_isSupported() const;

  /**
   * @brief Get the list of textures generated by the multi render target.
   */
  std::vector<TexturePtr>& get_textures();

  /**
   * @brief Gets the number of textures in this MRT. This number can be different from
   * `_textures.length` in case a depth texture is generated.
   */
  size_t get_count() const;

  /**
   * @brief Get the depth texture generated by the multi render target if
   * options.generateDepthTexture has been set.
   */
  TexturePtr& get_depthTexture();

  /**
   * @brief Set the wrapping mode on U of all the textures we are rendering to.
   * Can be any of the Texture. (CLAMP_ADDRESSMODE, MIRROR_ADDRESSMODE or WRAP_ADDRESSMODE).
   */
  void set_wrapU(unsigned int wrap) override;

  /**
   * @brief Set the wrapping mode on V of all the textures we are rendering to.
   * Can be any of the Texture. (CLAMP_ADDRESSMODE, MIRROR_ADDRESSMODE or WRAP_ADDRESSMODE).
   */
  void set_wrapV(unsigned int wrap) override;

  /**
   * @brief Get the number of samples used if MSAA is enabled.
   */
  unsigned int get_samples() const override;

  /**
   * @brief Set the number of samples used if MSAA is enabled.
   */
  void set_samples(unsigned int value) override;

  void unbindFrameBuffer(Engine* engine, unsigned int faceIndex);

private:
  void _initTypes(size_t count, std::vector<unsigned int>& types,
                  std::vector<unsigned int>& samplingModes,
                  const std::optional<IMultiRenderTargetOptions>& options = std::nullopt);
  void _createInternalTextures();
  void _createTextures();

public:
  /**
   * Get if draw buffers are currently supported by the used hardware and
   * browser.
   */
  ReadOnlyProperty<MultiRenderTarget, bool> isSupported;

  /**
   * Get the list of textures generated by the multi render target.
   */
  ReadOnlyProperty<MultiRenderTarget, std::vector<TexturePtr>> textures;

  /**
   * Gets the number of textures in this MRT. This number can be different from `_textures.length`
   * in case a depth texture is generated.
   */
  ReadOnlyProperty<MultiRenderTarget, size_t> count;

  /**
   * Get the depth texture generated by the multi render target if options.generateDepthTexture
   * has been set
   */
  ReadOnlyProperty<MultiRenderTarget, TexturePtr> depthTexture;

private:
  std::vector<InternalTexturePtr> _internalTextures;
  std::vector<TexturePtr> _textures;
  IMultiRenderTargetOptions _multiRenderTargetOptions;
  size_t _count;
  bool _drawOnlyOnFirstAttachmentByDefault;
  TexturePtr _nullTexture;

}; // end of class MultiRenderTarget

} // end of namespace BABYLON

#endif // end of BABYLON_MATERIALS_TEXTURES_MULTI_RENDER_TARGET_H
