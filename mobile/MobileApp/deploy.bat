@echo off
echo ========================================
echo    RoboNet MobileApp Deploy Tool
echo ========================================
echo.

set BUILD_DIR=D:\Projects\RoboNet_Project\mobile\MobileApp\build\Qt_6_11_1_for_Android_arm64_v8a_Debug
set APK=%BUILD_DIR%\android-build\build\outputs\apk\debug\android-build-debug.apk
set ADB=C:\Users\Mazi\AppData\Local\Android\Sdk\platform-tools\adb.exe
set PACKAGE=org.qtproject.example.appMobileApp

echo [1/4] Running CMake configuration...
"C:\Program Files\CMake\bin\cmake.exe" ^
  -S "D:\Projects\RoboNet_Project\mobile\MobileApp" ^
  -B "%BUILD_DIR%" ^
  -DANDROID_ABI=arm64-v8a ^
  -DANDROID_NDK="C:/Users/Mazi/AppData/Local/Android/Sdk/ndk/27.2.12479018" ^
  -DANDROID_PLATFORM=android-28 ^
  -DANDROID_SDK_ROOT="C:/Users/Mazi/AppData/Local/Android/Sdk" ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_TOOLCHAIN_FILE="C:/Users/Mazi/AppData/Local/Android/Sdk/ndk/27.2.12479018/build/cmake/android.toolchain.cmake" ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/android_arm64_v8a" ^
  -DCMAKE_FIND_ROOT_PATH="C:/Qt/6.11.1/android_arm64_v8a" ^
  -DQT_HOST_PATH="C:/Qt/6.11.1/mingw_64" ^
  -DANDROID_USE_LEGACY_TOOLCHAIN_FILE=OFF ^
  -DCMAKE_GENERATOR=Ninja ^
  -DCMAKE_MAKE_PROGRAM="C:/mingw64/bin/ninja.exe"
if %errorlevel% neq 0 (
    echo CMAKE FAILED! Check errors above.
    pause
    exit /b 1
)
echo CMake OK!
echo.

echo [2/4] Building C++ and packaging APK...
"C:\Program Files\CMake\bin\cmake.exe" --build "%BUILD_DIR%" --target all
if %errorlevel% neq 0 (
    echo BUILD FAILED! Check errors above.
    pause
    exit /b 1
)
echo Build + APK OK!
echo.

echo [3/4] Installing APK on device...
"%ADB%" shell am force-stop %PACKAGE%
timeout /t 1 /nobreak > nul
"%ADB%" install -r "%APK%"
if %errorlevel% neq 0 (
    echo INSTALL FAILED! Check errors above.
    pause
    exit /b 1
)
echo Install OK!
echo.

echo [4/4] Launching app...
"%ADB%" shell monkey -p %PACKAGE% -c android.intent.category.LAUNCHER 1
echo.

echo ========================================
echo    Deploy Complete! Check your phone.
echo ========================================
pause
