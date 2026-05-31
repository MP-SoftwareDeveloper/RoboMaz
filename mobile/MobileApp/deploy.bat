@echo off
echo ========================================
echo    RoboNet MobileApp Deploy Tool
echo ========================================
echo.

echo [1/7] Running CMake configuration...
"C:\Program Files\CMake\bin\cmake.exe" -S "D:\Projects\RoboNet_Project\mobile\MobileApp" -B "D:\Projects\RoboNet_Project\mobile\MobileApp\build\Qt_6_11_1_for_Android_arm64_v8a_Debug" -DANDROID_ABI=arm64-v8a -DANDROID_NDK="C:/Users/Mazi/AppData/Local/Android/Sdk/ndk/27.2.12479018" -DANDROID_PLATFORM=android-28 -DANDROID_SDK_ROOT="C:/Users/Mazi/AppData/Local/Android/Sdk" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="C:/Users/Mazi/AppData/Local/Android/Sdk/ndk/27.2.12479018/build/cmake/android.toolchain.cmake" -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/android_arm64_v8a" -DCMAKE_FIND_ROOT_PATH="C:/Qt/6.11.1/android_arm64_v8a" -DQT_HOST_PATH="C:/Qt/6.11.1/mingw_64" -DANDROID_USE_LEGACY_TOOLCHAIN_FILE=OFF -DCMAKE_GENERATOR=Ninja -DCMAKE_MAKE_PROGRAM="C:/mingw64/bin/ninja.exe"
if %errorlevel% neq 0 (
    echo CMAKE FAILED! Check errors above.
    pause
    exit /b 1
)
echo CMake successful!
echo.

echo [2/7] Building C++ project...
"C:\Program Files\CMake\bin\cmake.exe" --build "D:\Projects\RoboNet_Project\mobile\MobileApp\build\Qt_6_11_1_for_Android_arm64_v8a_Debug" --target all
if %errorlevel% neq 0 (
    echo BUILD FAILED! Check errors above.
    pause
    exit /b 1
)
echo Build successful!
echo.

echo [3/7] Generating Android build folder...
"C:\Qt\6.11.1\mingw_64\bin\androiddeployqt.exe" --input "D:\Projects\RoboNet_Project\mobile\MobileApp\build\Qt_6_11_1_for_Android_arm64_v8a_Debug\android-appMobileApp-deployment-settings.json" --output "D:\Projects\RoboNet_Project\mobile\MobileApp\build\Qt_6_11_1_for_Android_arm64_v8a_Debug\android-build-appMobileApp" --android-platform android-36 --jdk "C:\Program Files\Java\jdk-23" --gradle
if %errorlevel% neq 0 (
    echo ANDROIDDEPLOYQT FAILED! Check errors above.
    pause
    exit /b 1
)
echo Android build folder ready!
echo.

echo [4/7] Building Android APK with Gradle...
cmd /c "cd /d D:\Projects\RoboNet_Project\mobile\MobileApp\build\Qt_6_11_1_for_Android_arm64_v8a_Debug\android-build-appMobileApp && gradlew.bat assembleDebug"
if %errorlevel% neq 0 (
    echo GRADLE BUILD FAILED! Check errors above.
    pause
    exit /b 1
)
echo APK built successfully!
echo.

echo [5/7] Stopping old app on phone...
C:\Users\Mazi\AppData\Local\Android\Sdk\platform-tools\adb.exe shell am force-stop org.qtproject.example.appMobileApp
timeout /t 2 /nobreak > nul
echo Done!
echo.

echo [6/7] Installing new APK...
C:\Users\Mazi\AppData\Local\Android\Sdk\platform-tools\adb.exe install -r "D:\Projects\RoboNet_Project\mobile\MobileApp\build\Qt_6_11_1_for_Android_arm64_v8a_Debug\android-build-appMobileApp\build\outputs\apk\debug\android-build-appMobileApp-debug.apk"
if %errorlevel% neq 0 (
    echo INSTALL FAILED! Check errors above.
    pause
    exit /b 1
)
echo Install successful!
echo.

echo [7/7] Launching app on phone...
C:\Users\Mazi\AppData\Local\Android\Sdk\platform-tools\adb.exe shell monkey -p org.qtproject.example.appMobileApp -c android.intent.category.LAUNCHER 1
echo.

echo ========================================
echo    Deploy Complete! Check your phone.
echo ========================================
pause