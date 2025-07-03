#include "bhEditor.hpp"
#include "bhPlatform.hpp"

int main(int argc, char* argv[])
{
  // TODO:
  if (argc > 1)
  {
    bhPlatform::SetDataDir(argv[1]);
  }
  return bhEditor::Run();
}
