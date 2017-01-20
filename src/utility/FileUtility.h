#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "utility/Log.h"

void LoadFileToBuffer(const std::string& file_name, std::string& buffer);