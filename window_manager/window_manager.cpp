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

void WindowManager::OnCreateNotify(const XCreateWindowEvent& e)
{

}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e)
{
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;


    if (_clients.count(e.window))
    {
        const Window frame = _clients[e.window];
        XConfigureWindow(_display, frame, e.value_mask, &changes);
    }
    //LOG(INFO) << "Resize " << e.window << " to " << Size<int>(e.width, e.height);
}

void WindowManager::OnConfigureNotify (const XConfigureEvent& e) {}

void WindowManager::OnMapRequest(const XMapRequestEvent& e)
{
    Frame(e.window);
    XMapWindow(_display, e.window);
}

void WindowManager::Frame(Window w)
{
    // Visual frame properties
    const unsigned int BORDER_WIDTH = 3;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;

    //Retrive attributes of window to frame

    XWindowAttributes xWinAttr;

    CHECK(XGetWindowAttributes(_display, w, &xWinAttr));

    //see framing existing top-level windows

    //create frame
    const Window frame = XCreateSimpleWindow(
        _display,
        _root,
        xWinAttr.x,
        xWinAttr.y,
        xWinAttr.width,
        xWinAttr.height,
        BORDER_WIDTH,
        BORDER_COLOR,
        BG_COLOR);
    
    // select events on frame
    XSelectInput (
        _display,
        frame,
        SubstructureRedirectMask | SubstructureNotifyMask
    );

    // add save set for unforeseen circumstances
    XAddToSaveSet(_display, w);
    XReparentWindow(_display, w, frame, 0, 0);

    //map frame
    XMapWindow(_display, frame);

    //save frame handle
    _clients[w] = frame;

     XGrabButton(
      _display,
      Button1,
      Mod1Mask,
      w,
      false,
      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
      GrabModeAsync,
      GrabModeAsync,
      None,
      None);
  //   b. Resize windows with alt + right button.
  XGrabButton(
      _display,
      Button3,
      Mod1Mask,
      w,
      false,
      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
      GrabModeAsync,
      GrabModeAsync,
      None,
      None);
  //   c. Kill windows with alt + f4.
  XGrabKey(
      _display,
      XKeysymToKeycode(_display, XK_F4),
      Mod1Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);
  //   d. Switch windows with alt + tab.
  XGrabKey(
      _display,
      XKeysymToKeycode(_display, XK_Tab),
      Mod1Mask,
      w,
      false,
      GrabModeAsync,
      GrabModeAsync);

      LOG(INFO) << "Framed window " << w << " [" << frame << "]";
}


void WindowManager::OnReparentNotify (const XReparentEvent &e) {}

void WindowManager::OnMapNotify(const XMapEvent& e) {}