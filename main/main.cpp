#include <cstdlib>
#include <glog/logging.h>
#include "window_manager.hpp"

using ::std::unique_ptr;

int main(int argc, char** argv)
{
    ::google::InitGoogleLogging(argv[0]);

    unique_ptr<WindowManager> windowManager = WindowManager::Create();

    if(!windowManager)
    {
        LOG(ERROR) << "Failed to initialize window manager.";
        return EXIT_FAILURE;
    }

    windowManager->Run();

    return EXIT_SUCCESS;
}