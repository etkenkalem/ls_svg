@echo off

set ProjectDir="P:\\ls\\svg"

pushd %ProjectDir%\

set Common=-TP -MT -FC -Zi -nologo -wd4700 -wd4312 -wd4311 -wd4530 -Od

rem Compile App:
cl /TP /std:c++14 /Fesvg ^
    %Common% ^
    %ProjectDir%\main.cpp ^
    /link -SUBSYSTEM:console
popd
