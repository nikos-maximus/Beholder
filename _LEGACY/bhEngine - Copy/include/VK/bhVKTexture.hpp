#ifndef BH_VK_TEXTURE_HPP
#define BH_VK_TEXTURE_HPP

#include "VK/bhVKIncludes.hpp"
#include "texture/bhTexture.hpp"

class bhVKTexture : public bhTexture
{
public:
  struct Properties
  {
    VkImage image{ VK_NULL_HANDLE };
    VkDeviceMemory memory{ VK_NULL_HANDLE };
    VkFormat fmt{ VK_FORMAT_UNDEFINED };
    VkExtent3D ext{};
    uint32_t mipLevels{ 0 };
  };

  bhVKTexture() = default;
  bhVKTexture(const Properties& p);

  void SetProperties(const Properties& props);

  //bool Load(const char* fileName);
  //const bhImage* GetImage() const { return image.get(); }
  //void SetTexture(const DeviceTexture& _dt) { texture = _dt; }
  //const DeviceTexture& GetTexture() const { return texture; }
  //DeviceTexture& GetTexture() { return texture; }
  
  __forceinline bool IsValid() override final
  {
    return (image != VK_NULL_HANDLE) && (memory != VK_NULL_HANDLE);
  }

  void GenerateMipmaps(VkCommandBuffer commandBuffer, uint32_t numMipLevels);
  inline void SetImage(VkImage inImg) { image = inImg; }
  inline const VkImage Image() const { return image; }
  inline const VkDeviceMemory Memory() const { return memory; }
  inline const VkFormat Format() const { return fmt; }
  inline uint32_t MipLevels() const { return mipLevels; }

protected:
private:
  VkImage image{ VK_NULL_HANDLE };
  VkDeviceMemory memory{ VK_NULL_HANDLE };
  VkFormat fmt{ VK_FORMAT_UNDEFINED };
  uint32_t mipLevels{ 0 };
};

#endif //BH_VK_TEXTURE_HPP
