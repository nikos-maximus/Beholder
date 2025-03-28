#ifndef BH_UTIL_H
#define BH_UTIL_H

struct FileReadData
{
	long length;
	void* data;
};

#ifdef __cplusplus
extern "C"
{
#endif

//char* bhUtil_CreatePath(const char* dir, const char* fileName); // "dir" is expected to end in "/"
//void bhUtil_FreePath(char** path);
//int bhUtil_CheckFileExists(const char* path); // 0 on success
//int bhUtil_CheckDirectoryExists(const char* path); // 0 on success

struct FileReadData bhUtil_ReadFile(const char* filePath, int binary);
void bhUtil_FreeFileData(struct FileReadData* fd);

void* bhUtil_AlignedAlloc(size_t size, size_t alignment);
void bhUtil_AlignedFree(void* data);

//uint8_t CountBits(uint32_t val);

#ifdef __cplusplus
}
#endif

#endif //BH_UTIL_H
