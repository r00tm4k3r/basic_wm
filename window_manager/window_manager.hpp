extern "C"
{
    #include <X11/Xlib.h>
}
#include <memory>

class WindowManager 
{
    public:
        static ::std::unique_ptr<WindowManager> Create();
        
        ~WindowManager();
    
        void Run();

    private:
        WindowManager(Display* d);
  
        const Window _root;
        static bool _wmDetected;
        Display* _display;

        
        static int OnXError(Display* display, XErrorEvent* e);
        static int OnWMDetected(Display* display, XErrorEvent* e);


        // Event handlers.
        void OnCreateNotify(const XCreateWindowEvent& e);
        void OnDestroyNotify(const XDestroyWindowEvent& e);
        void OnReparentNotify(const XReparentEvent& e);
        void OnMapNotify(const XMapEvent& e);
        void OnUnmapNotify(const XUnmapEvent& e);
        void OnConfigureNotify(const XConfigureEvent& e);
        void OnMapRequest(const XMapRequestEvent& e);
        void OnConfigureRequest(const XConfigureRequestEvent& e);
        void OnButtonPress(const XButtonEvent& e);
        void OnButtonRelease(const XButtonEvent& e);
        void OnMotionNotify(const XMotionEvent& e);
        void OnKeyPress(const XKeyEvent& e);
        void OnKeyRelease(const XKeyEvent& e);

};
