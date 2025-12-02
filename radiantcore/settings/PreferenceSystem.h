#pragma once

#include "ipreferencesystem.h"
#include "PreferencePage.h"

namespace settings
{

/// Implementation of the IPreferenceSystem module
class PreferenceSystem: public IPreferenceSystem
{
    PreferencePagePtr _rootPage;
    PreferencePage& getRootPage();

public:
    // Looks up a page for the given path and returns it to the client
    IPreferencePage& getPage(const std::string& path) override;
    void foreachPage(const std::function<void(IPreferencePage&)>& functor) override;

    // RegisterableModule implementation
    std::string getName() const override;

};

} // namespace
