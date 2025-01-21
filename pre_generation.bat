@echo off
echo Starting pre-generation script

if exist .\CM7\Core\Src\main.cpp (
    echo Core M7 - Switching main to C ...
    rename .\CM7\Core\Src\main.cpp main.c
) else (
    echo Core M7 - main.cpp not found.
)

if exist .\CM4\Core\Src\main.cpp (
    echo Core M4 - Switching main to C ...
    rename .\CM4\Core\Src\main.cpp main.c
) else (
    echo Core M4 - main.cpp not found.
)

if exist .\CM7\Core\Src\freertos.cpp (
    echo Core M7 - Switching FreeRTOS to C ...
    rename .\CM7\Core\Src\freertos.cpp freertos.c
) else (
    echo Core M7 - freertos.cpp not found.
)
if exist .\CM4\Core\Src\freertos.cpp (
    echo Core M4 - Switching FreeRTOS to C ...
    rename .\CM4\Core\Src\freertos.cpp freertos.c
) else (
    echo Core M4 - freertos.cpp not found.
)

echo Ending pre-generation script