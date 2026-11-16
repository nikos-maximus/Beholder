#include <SDL3/SDL_init.h>
#include "bhDefines.hpp"

class bhAppState
{
protected:
  enum ImguiFlags
  {
    READY,
    VISIBLE
  };

public:
  SDL_PropertiesID props { SDL_MAX_UINT32 };
  SDL_Window* mainWindow { nullptr };
  
  void SetImguiReady(bool ready)
  {
    imguiFlags = ready ? (imguiFlags | BH_BIT(ImguiFlags::READY)) : (imguiFlags & ~BH_BIT(ImguiFlags::READY));
  }
  
  bool IsImguiReady()
  {
    return (imguiFlags & BH_BIT(ImguiFlags::READY)) != 0;
  }
  
  void SetImguiVisible(bool visible)
  {
    imguiFlags = visible ? (imguiFlags | BH_BIT(ImguiFlags::VISIBLE)) : (imguiFlags & ~BH_BIT(ImguiFlags::VISIBLE));
  }
  
  bool IsImguiVisible()
  {
    return IsImguiReady() && ((imguiFlags & BH_BIT(ImguiFlags::VISIBLE)) != 0);
  }
  
protected:
private:
  uint8_t imguiFlags { 0 };
};
