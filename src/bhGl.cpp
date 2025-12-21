#include <stdio.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include "bhDefines.hpp"
#include "bhGl.hpp"

namespace bhGl
{
	static constexpr GLubyte BH_IMGUI_FLAG_READY = BH_BIT(0);
	static constexpr GLubyte BH_IMGUI_FLAG_VISIBLE = BH_BIT(1);

	static GLubyte g_imguiFlags = 0;

	bool Init()
	{
		if (gl3wInit()) // 0 on success
		{
			return false;
		}
		if (!gl3wIsSupported(BH_GL_VERSION_MAJOR, BH_GL_VERSION_MINOR)) // 1 on success
		{
			return false;
		}

		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClearDepth(1.0);
		glClearStencil(0);

		return true;
	}

	void BeginImGuiFrame();

	void BeginFrame()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		BeginImGuiFrame();
	}

	void EndImGuiFrame();

	void EndFrame()
	{
		EndImGuiFrame();
		glFlush();
	}

	bool InitImGui(SDL_Window* window, void* glContext)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		if (ImGui_ImplSDL3_InitForOpenGL(window, glContext))
		{
			static const size_t VERSION_SRING_LEN = 16;
			char versionString[VERSION_SRING_LEN];
			sprintf_s(versionString, VERSION_SRING_LEN, "#version %d%d0", BH_GL_VERSION_MAJOR, BH_GL_VERSION_MINOR);
			if (ImGui_ImplOpenGL3_Init(versionString))
			{
				ImGui::StyleColorsDark();
				g_imguiFlags = BH_IMGUI_FLAG_READY;
				return true;
			}
		}
		return false;
	}

	void DestroyImGui()
	{
		g_imguiFlags = 0;
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}

	void BeginImGuiFrame()
	{
		if (g_imguiFlags == (BH_IMGUI_FLAG_READY | BH_IMGUI_FLAG_VISIBLE))
		{
			ImGui_ImplSDL3_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			// DEBUG
			bool showdemo = true;
			ImGui::ShowDemoWindow(&showdemo);
		}
	}

	void EndImGuiFrame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ShowImGui(bool show)
	{
		if (show)
		{
			g_imguiFlags |= BH_IMGUI_FLAG_VISIBLE;
		}
		else
		{
			g_imguiFlags &= ~BH_IMGUI_FLAG_VISIBLE;
		}
	}
}
