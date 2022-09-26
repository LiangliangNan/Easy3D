#ifndef EASY3D_WXWIDGETS_APPLICATION_H
#define EASY3D_WXWIDGETS_APPLICATION_H

#include "wx/app.h"

class Easy3DApp : public wxApp
{
public:
    virtual bool OnInit() wxOVERRIDE;
};

#endif // #ifndef EASY3D_WXWIDGETS_APPLICATION_H
