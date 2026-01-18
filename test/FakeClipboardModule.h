#pragma once

#include "iclipboard.h"

namespace test
{

class FakeClipboardModule :
    public radiant::IClipboard
{
private:
    std::string _contents;
    sigc::signal<void> _changedSignal;

public:
    std::string getString() override
    {
        return _contents;
    }

    void setString(const std::string& str) override
    {
        _contents = str;
        _changedSignal.emit();
    }

    sigc::signal<void>& signal_clipboardContentChanged() override
    {
        return _changedSignal;
    }

    std::string getName() const override
    {
        static std::string _name(MODULE_CLIPBOARD);
        return _name;
    }
};

}
