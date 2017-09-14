#include <babylon/postprocess/highlights_post_process.h>

namespace BABYLON {

HighlightsPostProcess::HighlightsPostProcess(const string_t& iName, float ratio,
                                             Camera* camera,
                                             unsigned int samplingMode,
                                             Engine* engine, bool reusable,
                                             unsigned int textureType)
    : PostProcess{iName,    "highlights", {},           {},
                  ratio,    camera,       samplingMode, engine,
                  reusable, nullptr,      textureType}
{
}

HighlightsPostProcess::~HighlightsPostProcess()
{
}

} // end of namespace BABYLON
