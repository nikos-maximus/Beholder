#ifndef BH_RESOURCE_HPP
#define BH_RESOURCE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include "bhHash.hpp"

extern const std::string INVALID_RESOURCE_PATH_STR;

template<typename Resource_t>
class bhResourceCache
{
public:
  struct Item_t
  {
    Item_t(const std::string& _path)
      : path(_path)
      , resource(std::make_unique<Resource_t>())
    {}

    std::unique_ptr<Resource_t> resource;
    std::string path;
  };
  
  inline Resource_t* Get(bhHash_t hash)
  {
    auto item = items.find(hash);
    if (item == items.end()) return nullptr; // Does not exist
    return item->second.resource.get();
  }

  inline Resource_t* Get(const std::string& path)
  {
    return Get(bhHash(path.c_str()));
  }

  Resource_t* New(const std::string& path)
  {
    bhHash_t hash = bhHash(path.c_str());
    auto item = items.find(hash);
    if (item != items.end()) return nullptr; // Function is called "New", so let's not return something that already exists (semantics!!!)
    auto result = items.emplace(hash, Item_t(path));
    if (result.second)
    {
      return result.first->second.resource.get();
    }
    return nullptr;
  }

  void ClearAll(void(*ResourceDestroyFunc)(std::unique_ptr<Resource_t>& resource))
  {
    for (auto& item : items)
    {
      ResourceDestroyFunc(item.second.resource);
    }
  }

protected:
private:
  std::unordered_map<bhHash_t, Item_t> items;
};

//void GenerateResourceHashes(std::unordered_map<bhHash_t, bhResourceInfo>& hashes, const char* dir, const char** extensions, const char* subDir = nullptr);

#endif //BH_RESOURCE_HPP
