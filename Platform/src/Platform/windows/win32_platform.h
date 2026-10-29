#pragma once
#include <Base/Base.h>
#include <Base/Datastructures/String.h>

void win32_getCurrentDirectory(char* buffer, PH::sizeptr maxsize);
PH::bool32 win32_setCurrentDirectory(PH::Base::SubString directory);
PH::bool32 win32_createDirectory(PH::Base::SubString directory);