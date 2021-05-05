#include "window_manager.hpp"
#include <glog/logging.h>

using ::std::unique_ptr;
unique_ptr<WindowManager> WindowManager::Create()
{
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr)
    {
        LOG(ERROR) << "Failed to open X display " << XDisplayName(nullptr);
        return nullptr;
    }
    return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* d) :
    _display(CHECK_NOTNULL(d)),
    _root(DefaultRootWindow(_display))
{
}

WindowManager::~WindowManager()
{
    XCloseDisplay(_display);
}

void WindowManager::Run()
{
    //Initialization

    _wmDetected = false;
    XSetErrorHandler(&WindowManager::OnWMDetected);
    XSelectInput(_display, _root, SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(_display, false);
    if(_wmDetected)
    {
        LOG(ERROR) << "Detected another window manager on display" << XDisplayString(_display);
        return;
    }
    XSetErrorHandler(&WindowManager::OnXError);

    //Main event loop

    for(;;)
    {
        XEvent e;
        XNextEvent(_display, &e);
        //LOG(INFO) << "Received event: " << ToString(e);


        //dispatch event
        switch(e.type)
        {
            case KeyPress:
                OnKeyPress(e.xkey);
                break;
            case KeyRelease:
                OnKeyRelease(e.xkey);
                break;
            case CreateNotify:
                OnCreateNotify(e.xcreatewindow);
                break;
            case DestroyNotify:
                OnDestroyNotify(e.xdestroywindow);
                break;
            case ReparentNotify:
                OnReparentNotify(e.xreparent);
                break;
            case MapNotify:
                OnMapNotify(e.xmap);
                break;
            case UnmapNotify:
                OnUnmapNotify(e.xunmap);
                break;
            case ConfigureNotify:
                OnConfigureNotify(e.xconfigure);
                break;
            case MapRequest:
                OnMapRequest(e.xmaprequest);
                break;
            case ConfigureRequest:
                OnConfigureRequest(e.xconfigurerequest);
                break;
            case ButtonPress:
                OnButtonPress(e.xbutton);
                break;
            case ButtonRelease:
                OnButtonRelease(e.xbutton);
                break;
            case MotionNotify:
                // Skip any already pending motion events.
                while (XCheckTypedWindowEvent(
                    _display, e.xmotion.window, MotionNotify, &e)) {}
                OnMotionNotify(e.xmotion);
                break;
            default:
                LOG(WARNING) << "Incorrect event";

        }    
    }
}

int WindowManager::OnWMDetected(Display* display, XErrorEvent* e)
{
    CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
    _wmDetected = true;
    return 0;
}

int WindowManager::OnXError(Display* display, XErrorEvent* e)
{

    return 0;
}