#pragma once
#include <string>
class Panel
{
public:
    virtual ~Panel()=default;
    virtual std::string& GetTitle()=0;
    virtual void SetVisible(bool visible)=0;
    virtual void OnImGuiUpdate()=0;
};