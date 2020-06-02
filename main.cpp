
#include "UI/window.h"

#include <iostream>


int main(int argc, char* argv[])
{
    EditorWindow window(1024, 512, 0);

    return window.Show();
}
