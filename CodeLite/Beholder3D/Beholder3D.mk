##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Beholder3D
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/home/nicholas/Projects/Beholder/CodeLite
ProjectPath            :=/home/nicholas/Projects/Beholder/CodeLite/Beholder3D
IntermediateDirectory  :=../build-$(WorkspaceConfiguration)/Beholder3D
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=nicholas
Date                   :=22/10/26
CodeLitePath           :=/home/nicholas/.codelite
MakeDirCommand         :=mkdir -p
LinkerName             :=/bin/clang++
SharedObjectLinkerName :=/bin/clang++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputDirectory        :=/home/nicholas/Projects/Beholder/CodeLite/build-$(WorkspaceConfiguration)/bin
OutputFile             :=../build-$(WorkspaceConfiguration)/bin/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :=$(IntermediateDirectory)/ObjectsList.txt
PCHCompileFlags        :=
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)$(HOME)/lib/glm $(IncludeSwitch)$(HOME)/lib/imgui $(IncludeSwitch)$(HOME)/lib/SDL/include $(IncludeSwitch)$(HOME)/lib/volk $(IncludeSwitch)$(HOME)/lib/VulkanMemoryAllocator/include $(IncludeSwitch)$(HOME)/lib/VulkanSDK/1.4.350.1/x86_64/include $(IncludeSwitch)../../include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)SDL3 
ArLibs                 :=  "SDL3" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overridden using an environment variable
##
AR       := /usr/bin/ar rcu
CXX      := /bin/clang++
CC       := /bin/clang
CXXFLAGS :=  -gdwarf-2 -fstandalone-debug -O0 -Wall $(Preprocessors)
CFLAGS   :=  -gdwarf-2 -fstandalone-debug -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhCamera.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhVk.cpp$(ObjectSuffix) \
	



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: MakeIntermediateDirs $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(IntermediateDirectory)"
	@$(MakeDirCommand) "$(OutputDirectory)"

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(IntermediateDirectory)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix): ../../src/bhLog.cpp $(IntermediateDirectory)/up_up_src_bhLog.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhLog.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhLog.cpp$(DependSuffix): ../../src/bhLog.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhLog.cpp$(DependSuffix) -MM ../../src/bhLog.cpp

$(IntermediateDirectory)/up_up_src_bhLog.cpp$(PreprocessSuffix): ../../src/bhLog.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhLog.cpp$(PreprocessSuffix) ../../src/bhLog.cpp

$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix): ../../src/bhConfig.cpp $(IntermediateDirectory)/up_up_src_bhConfig.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhConfig.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(DependSuffix): ../../src/bhConfig.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(DependSuffix) -MM ../../src/bhConfig.cpp

$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(PreprocessSuffix): ../../src/bhConfig.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhConfig.cpp$(PreprocessSuffix) ../../src/bhConfig.cpp

$(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(ObjectSuffix): ../../src/bhMain3D.cpp $(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhMain3D.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(DependSuffix): ../../src/bhMain3D.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(DependSuffix) -MM ../../src/bhMain3D.cpp

$(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(PreprocessSuffix): ../../src/bhMain3D.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhMain3D.cpp$(PreprocessSuffix) ../../src/bhMain3D.cpp

$(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(ObjectSuffix): ../../src/bhVolkImpl.cpp $(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhVolkImpl.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(DependSuffix): ../../src/bhVolkImpl.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(DependSuffix) -MM ../../src/bhVolkImpl.cpp

$(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(PreprocessSuffix): ../../src/bhVolkImpl.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhVolkImpl.cpp$(PreprocessSuffix) ../../src/bhVolkImpl.cpp

$(IntermediateDirectory)/up_up_src_bhCamera.cpp$(ObjectSuffix): ../../src/bhCamera.cpp $(IntermediateDirectory)/up_up_src_bhCamera.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhCamera.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhCamera.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhCamera.cpp$(DependSuffix): ../../src/bhCamera.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhCamera.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhCamera.cpp$(DependSuffix) -MM ../../src/bhCamera.cpp

$(IntermediateDirectory)/up_up_src_bhCamera.cpp$(PreprocessSuffix): ../../src/bhCamera.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhCamera.cpp$(PreprocessSuffix) ../../src/bhCamera.cpp

$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix): ../../src/bhUtil.cpp $(IntermediateDirectory)/up_up_src_bhUtil.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhUtil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(DependSuffix): ../../src/bhUtil.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(DependSuffix) -MM ../../src/bhUtil.cpp

$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(PreprocessSuffix): ../../src/bhUtil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhUtil.cpp$(PreprocessSuffix) ../../src/bhUtil.cpp

$(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(ObjectSuffix): ../../src/bhVkPipeline.cpp $(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhVkPipeline.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(DependSuffix): ../../src/bhVkPipeline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(DependSuffix) -MM ../../src/bhVkPipeline.cpp

$(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(PreprocessSuffix): ../../src/bhVkPipeline.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhVkPipeline.cpp$(PreprocessSuffix) ../../src/bhVkPipeline.cpp

$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix): ../../src/bhPlatform.cpp $(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhPlatform.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(DependSuffix): ../../src/bhPlatform.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(DependSuffix) -MM ../../src/bhPlatform.cpp

$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(PreprocessSuffix): ../../src/bhPlatform.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(PreprocessSuffix) ../../src/bhPlatform.cpp

$(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(ObjectSuffix): ../../src/bhVMAImpl.cpp $(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhVMAImpl.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(DependSuffix): ../../src/bhVMAImpl.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(DependSuffix) -MM ../../src/bhVMAImpl.cpp

$(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(PreprocessSuffix): ../../src/bhVMAImpl.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhVMAImpl.cpp$(PreprocessSuffix) ../../src/bhVMAImpl.cpp

$(IntermediateDirectory)/up_up_src_bhVk.cpp$(ObjectSuffix): ../../src/bhVk.cpp $(IntermediateDirectory)/up_up_src_bhVk.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhVk.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhVk.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhVk.cpp$(DependSuffix): ../../src/bhVk.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhVk.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhVk.cpp$(DependSuffix) -MM ../../src/bhVk.cpp

$(IntermediateDirectory)/up_up_src_bhVk.cpp$(PreprocessSuffix): ../../src/bhVk.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhVk.cpp$(PreprocessSuffix) ../../src/bhVk.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


