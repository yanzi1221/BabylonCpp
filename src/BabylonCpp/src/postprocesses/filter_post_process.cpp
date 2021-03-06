#include <babylon/postprocesses/filter_post_process.h>

#include <babylon/materials/effect.h>

namespace BABYLON {

FilterPostProcess::FilterPostProcess(const std::string& iName, const Matrix& _kernelMatrix,
                                     const std::variant<float, PostProcessOptions>& options,
                                     const CameraPtr& camera,
                                     const std::optional<unsigned int>& samplingMode,
                                     Engine* engine, bool reusable)
    : PostProcess{iName,  "filter",     {"kernelMatrix"}, {},      options,
                  camera, samplingMode, engine,           reusable}
{
  kernelMatrix = _kernelMatrix;

  onApply = [this](Effect* effect, EventState&) -> void {
    effect->setMatrix("kernelMatrix", kernelMatrix);
  };
}

FilterPostProcess::~FilterPostProcess() = default;

std::string FilterPostProcess::getClassName() const
{
  return "FilterPostProcess";
}

FilterPostProcessPtr FilterPostProcess::_Parse(const json& /*parsedPostProcess*/,
                                               const CameraPtr& /*targetCamera*/, Scene* /*scene*/,
                                               const std::string& /*rootUrl*/)
{
  return nullptr;
}

} // end of namespace BABYLON
