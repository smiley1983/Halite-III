CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

cl.exe /FeMyBot.exe /std:c++14 /O2 /MT /EHsc /D_USE_MATH_DEFINES .\hlt\hlt_in.cpp .\hlt\location.cpp .\hlt\map.cpp .\MyBot.cpp

.\halite.exe -d "240 160" ".\MyBot.exe" ".\MyBot.exe"
