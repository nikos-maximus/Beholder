 #ifndef BH_RESOURCE_HPP
#define BH_RESOURCE_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <assert.h>
#include <dirent.h>
#include "bhHash.hpp"
#include "bhUtil.hpp"
#include "Platform/bhPlatform.hpp"

////////////////////////////////////////////////////////////////////////////////
template<class Resource_T>
class bhResource
{
public:
    bhResource() = default;

    bhResource(const char* _name)
        : name(_name)
    {
        hash = bhHash(_name);
        resource.reset(new Resource_T());
    }

    enum Status
    {
        RESOURCE_NA,
        RESOURCE_PENDING,
        RESOURCE_READY
    };

    //void IncRef() { ++refCount; }
    //void DecRef()
    //{
    //    assert(refCount > 0);
    //    --refCount;
    //}

    const std::string& GetName() const { return name; }
    bhHash_t GetHash() const { return hash; }
    std::unique_ptr<Resource_T>& GetResource() { return resource; }
    Status GetStatus() const { return status; }
    void ResetStatus() { status = RESOURCE_NA; }
    void SetPending() { status = RESOURCE_PENDING; }
    void SetReady() { status = RESOURCE_READY; }

protected:
private:
    

    std::unique_ptr<Resource_T> resource;
    std::string name;
    //uint32_t refCount{ 0 };
    Status status{ Status::RESOURCE_NA };
    bhHash_t hash;
};

////////////////////////////////////////////////////////////////////////////////
template<class Resource_T>
void bhResource_AddDirectory(std::unordered_map<bhHash_t, bhResource<Resource_T>>& outRes, const char* dir, const char** extensions, const char* subDir = nullptr)
{
    static char currDir[BH_PATH_BUF_LEN];

    sprintf_s(currDir, BH_PATH_BUF_LEN, "%s%s", dir, subDir ? subDir : "");
    DIR* baseDir = opendir(currDir);
    if (baseDir)
    {
        while (dirent* entry = readdir(baseDir))
        {
            switch (entry->d_type)
            {
                case DT_REG: // A file
                {
                    static char fileName[BH_PATH_BUF_LEN];
                    if (!bhUtil::IsFileType(entry->d_name, extensions)) continue;
                    sprintf_s(fileName, BH_PATH_BUF_LEN, "%s%s", subDir ? subDir : "", entry->d_name);
                    outRes.emplace(bhHash(fileName), bhResource<Resource_T>(fileName));
                    continue;
                }
                case DT_DIR: // A directory
                {
                    if (strcmp(entry->d_name, ".") == 0) continue;
                    if (strcmp(entry->d_name, "..") == 0) continue;

                    static char nextSubDir[BH_PATH_BUF_LEN];
                    sprintf_s(nextSubDir, BH_PATH_BUF_LEN, "%s%s%s%s", subDir ? subDir : "", subDir ? bhPlatform::GetDirSeparator() : "", entry->d_name, bhPlatform::GetDirSeparator());
                    bhResource_AddDirectory(outRes, dir, extensions, nextSubDir);
                    break;
                }
                case DT_LNK: // Etc.
                default:
                {
                    continue;
                }
            }
        }
        closedir(baseDir);
    }
}

#endif //BH_RESOURCE_HPP
