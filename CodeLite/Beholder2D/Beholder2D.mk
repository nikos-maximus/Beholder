##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Beholder2D
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/home/nicholas/Projects/Beholder/CodeLite
ProjectPath            :=/home/nicholas/Projects/Beholder/CodeLite/Beholder2D
IntermediateDirectory  :=../build-$(WorkspaceConfiguration)/Beholder2D
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=nicholas
Date                   :=19/10/26
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
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)$(HOME)/lib/SDL/include $(IncludeSwitch)$(HOME)/lib/volk $(IncludeSwitch)$(HOME)/lib/VulkanMemoryAllocator/include $(IncludeSwitch)$(HOME)/lib/VulkanSDK/1.4.350.1/x86_64/include $(IncludeSwitch)../../include 
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
Objects0=$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix): ../../src/bhConfig.cpp $(IntermediateDirectory)/up_up_src_bhConfig.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhConfig.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(DependSuffix): ../../src/bhConfig.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(DependSuffix) -MM ../../src/bhConfig.cpp

$(IntermediateDirectory)/up_up_src_bhConfig.cpp$(PreprocessSuffix): ../../src/bhConfig.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhConfig.cpp$(PreprocessSuffix) ../../src/bhConfig.cpp

$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix): ../../src/bhPlatform.cpp $(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhPlatform.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(DependSuffix): ../../src/bhPlatform.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(DependSuffix) -MM ../../src/bhPlatform.cpp

$(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(PreprocessSuffix): ../../src/bhPlatform.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhPlatform.cpp$(PreprocessSuffix) ../../src/bhPlatform.cpp

$(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(ObjectSuffix): ../../src/bhMain2D.cpp $(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhMain2D.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(DependSuffix): ../../src/bhMain2D.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(DependSuffix) -MM ../../src/bhMain2D.cpp

$(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(PreprocessSuffix): ../../src/bhMain2D.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhMain2D.cpp$(PreprocessSuffix) ../../src/bhMain2D.cpp

$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix): ../../src/bhUtil.cpp $(IntermediateDirectory)/up_up_src_bhUtil.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhUtil.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(DependSuffix): ../../src/bhUtil.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(DependSuffix) -MM ../../src/bhUtil.cpp

$(IntermediateDirectory)/up_up_src_bhUtil.cpp$(PreprocessSuffix): ../../src/bhUtil.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhUtil.cpp$(PreprocessSuffix) ../../src/bhUtil.cpp

$(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix): ../../src/bhLog.cpp $(IntermediateDirectory)/up_up_src_bhLog.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhLog.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhLog.cpp$(DependSuffix): ../../src/bhLog.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhLog.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhLog.cpp$(DependSuffix) -MM ../../src/bhLog.cpp

$(IntermediateDirectory)/up_up_src_bhLog.cpp$(PreprocessSuffix): ../../src/bhLog.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhLog.cpp$(PreprocessSuffix) ../../src/bhLog.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


