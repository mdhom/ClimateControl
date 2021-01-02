#include "PreferencesManager.h"

PreferencesManager::PreferencesManager(){
    _isEditing = false;
}

bool PreferencesManager::beginEdit()
{
    if (_isEditing)
    {
        return true;
    }

    preferences.begin("config", false);
    _isEditing = true;

    return false;
}

void PreferencesManager::endEdit()
{
    preferences.end();
    _isEditing = false;
}

void PreferencesManager::endIf(bool wasOpen)
{
    if (!wasOpen)
    {
        this->endEdit();
    }
}

void PreferencesManager::SetPublishInterval(uint interval) {
    if (interval <= 0) {
        Serial.println("Invalid interval given");
        return;
    }

    bool alreadyEditing = this->beginEdit();

    this->preferences.putUInt("PublishInterval", interval);

    this->endIf(alreadyEditing);

    PublishInterval = interval;
}

uint PreferencesManager::GetPublishInterval()
{
    if (PublishInterval == 0) {
        bool alreadyEditing = this->beginEdit();

        PublishInterval = this->preferences.getUInt("PublishInterval", 10000);

        this->endIf(alreadyEditing);
    }

    return PublishInterval;
}