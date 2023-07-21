#include "gui/common/FrontendApplication.hpp" // this must go first, because bit flags operators included in Model.hpp will break simulator.
#include <gui/model/Model.hpp>
#include "gui/model/ModelListener.hpp"
#include "texts/TextKeysAndLanguages.hpp"
#include "touchgfx/Texts.hpp"
#if DEVICE
#include "main.h"
#include "Thread.hpp"
#endif

Model::Model() : modelListener(0) { instance = this; }
Model* Model::getInstance() { return instance; }

void Model::tick()
{
#if DEVICE
    Thread::tick(Thread::Frame);
#endif
}
