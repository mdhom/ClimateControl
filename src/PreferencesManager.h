#ifndef PREFERENCESMANAGER_H
#define PREFERENCESMANAGER_H

#include <Preferences.h>

class PreferencesManager {

private:
    bool _isEditing;
    Preferences preferences;
    void endIf(bool wasOpen);
    
    uint PublishInterval = 0;

public:
    PreferencesManager();
    bool beginEdit();
    void endEdit();
    
    void SetPublishInterval(uint interval);
    uint GetPublishInterval();
};
#endif