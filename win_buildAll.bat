@echo off
set /A compileWin32=1
set /A compilex64=1

FOR /D %%G IN (*) do ( 
	IF EXIST "%%G/Builds/VisualStudio2017/%%G.sln" (
		echo "Solution file found for: %%G%
		IF "%compileWin32%" == "1" (
			echo "Compiling for Win32..."
			msbuild "%%G/Builds/VisualStudio2017/%%G.sln" /p:Configuration="Release 32bit",Platform=Win32
			COPY /Y "%%G\Builds\VisualStudio2017\Win32\Release 32Bit\VST\%%G_Win32.dll" "_compiledPlugins\win32"
		)

		IF "%compilex64%" == "1" (
			echo "Compiling for x64..."
			msbuild "%%G/Builds/VisualStudio2017/%%G.sln" /p:Configuration="Release 64bit",Platform=x64
			COPY /Y "%%G\Builds\VisualStudio2017\x64\Release 64Bit\VST\%%G_x64.dll" "_compiledPlugins\x64"
		)

	)
)

