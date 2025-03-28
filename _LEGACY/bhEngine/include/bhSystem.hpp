#ifndef BH_SYSTEM_HPP
#define BH_SYSTEM_HPP

#define BH_APP_NAME "bhEngine"

//class bhMeshCache;
struct SDL_Window;
struct bhConfig;

namespace bhSystem
{
	//SDL_Window* MainWindow();
	//bhMeshCache* MeshCache();

	const bhConfig* Config();
	bool Init(char* argv[]);
	void Destroy();
	const char* EngineName();
	uint32_t EngineVersion();
	inline const char* ApplicationName() { return BH_APP_NAME; }
	uint32_t ApplicationVersion();
	
	bool WindowSize(int& w, int& h);
	float WindowASpect();

	void BeginFrame();
	void EndFrame();
	void BeginImGuiFrame();
	void EndImGuiFrame();

	void SetMouseMode_Cursor();
	void SetMouseMode_Look();
};

#endif //BH_SYSTEM_HPP
