#include "FileUtility.h"

void LoadFileToBuffer(const std::string& file_name, std::string& buffer)
{
	std::ifstream file(file_name);
	if (!file.is_open())
	{
		Logger::Log("Cannot open shader file: ", file_name, "\n");
		return;
	}

	file.seekg(0, std::ios::end);
	buffer.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	buffer.assign((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	file.close();
}