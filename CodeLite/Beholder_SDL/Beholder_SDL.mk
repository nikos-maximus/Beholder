##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Beholder_SDL
ConfigurationName      :=Debug
WorkspaceConfiguration :=Debug
WorkspacePath          :=/home/nicholas/Projects/Beholder/CodeLite
ProjectPath            :=/home/nicholas/Projects/Beholder/CodeLite/Beholder_SDL
IntermediateDirectory  :=../build-$(WorkspaceConfiguration)/Beholder_SDL
OutDir                 :=$(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=nicholas
Date                   :=29/09/26
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
Libs                   := 
ArLibs                 :=  
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
Objects0=$(IntermediateDirectory)/up_up_src_bhMain.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/up_up_src_bhMain.cpp$(ObjectSuffix): ../../src/bhMain.cpp $(IntermediateDirectory)/up_up_src_bhMain.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/nicholas/Projects/Beholder/src/bhMain.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_up_src_bhMain.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_up_src_bhMain.cpp$(DependSuffix): ../../src/bhMain.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_up_src_bhMain.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_up_src_bhMain.cpp$(DependSuffix) -MM ../../src/bhMain.cpp

$(IntermediateDirectory)/up_up_src_bhMain.cpp$(PreprocessSuffix): ../../src/bhMain.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_up_src_bhMain.cpp$(PreprocessSuffix) ../../src/bhMain.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(IntermediateDirectory)


