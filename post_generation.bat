@echo off
echo Starting pre-generation script

if exist .\CM7\Core\Src\main.c (
    echo Core M7 - Switching main to C++ ...
    rename .\CM7\Core\Src\main.c main.cpp
) else (
    echo Core M7 - main.c not found.
)

if exist .\CM4\Core\Src\main.c (
    echo Core M4 - Switching main to C++ ...
    rename .\CM4\Core\Src\main.c main.cpp
) else (
    echo Core M4 - main.c not found.
)

if exist .\CM7\Core\Src\freertos.c (
    echo Core M7 - Switching FreeRTOS to C++ ...
    rename .\CM7\Core\Src\freertos.c freertos.cpp
) else (
    echo Core M7 - freertos.c not found.
)

if exist .\CM4\Core\Src\freertos.c (
    echo Core M4 - Switching FreeRTOS to C++ ...
    rename .\CM4\Core\Src\freertos.c freertos.cpp
) else (
    echo Core M4 - freertos.c not found.
)

echo Ending pre-generation script