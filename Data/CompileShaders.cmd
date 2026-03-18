@echo off
pushd .\Shader_src

for %%f in (*) do (
	rem echo %%i
	slangc.exe -target spirv %%f -o ..\Shaders\%%~nf.out
)

popd
@echo on
