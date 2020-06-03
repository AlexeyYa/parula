
#include "UI/window.h"

#include <iostream>


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    EditorWindow window(1024, 512, 0);

    return window.Show();
}
