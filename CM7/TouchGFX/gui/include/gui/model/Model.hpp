#pragma once

#define SCREEN_HISTORY_LENGTH 4

#define APP static_cast<FrontendApplication*>(touchgfx::Application::getInstance())
#define MODEL_REDIRECT Model::getInstance()->handleRedirection()
#define MODEL_DIALOG_SET Model::getInstance()->handleDialogSet()
#define MODEL_SET_LANGUAGE(id) Model::getInstance()->handleLanguageSet(id)

class ModelListener;

/**
 * @brief GUI model class.
 */
class Model
{

public:

    Model();
    Model(const Model&) = delete;
    Model(const Model&&) = delete;

    // Gets the GUI model singleton instance.
    static Model* getInstance();

    // Binds the listener to the model.
    void bind(ModelListener* listener) { modelListener = listener; }

    // Handles screen refresh ticks.
    void tick();

protected:
    static inline Model* instance = 0;
    ModelListener* modelListener;

};
