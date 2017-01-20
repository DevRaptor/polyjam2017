#include "ResourceManager.h"

#include <iostream>
#include <fstream>

#include "rendering/Mesh.h"
#include "utility/Log.h"

void ResourceManager::LoadParameters()
{
	std::fstream textfile;
	textfile.open("data/parameters.txt", std::ios::in);

	std::string name, value;

	if (textfile.good())
	{
		getline(textfile, name); //header

		int i = 0;
		while (textfile.good())
		{
			getline(textfile, name, '=');
			getline(textfile, value, '\n');

			parameters[name] = value;
			i++;
		}

		Logger::Log("Loaded ", i, " parameters\n");
	}
	else
	{
		Logger::Log("Cannot open parameters.txt file!\n");
	}
}

std::shared_ptr<Mesh> ResourceManager::LoadMesh(const std::string& name)
{
	Logger::Log("Load ", name, " mesh\n");
	auto mesh = std::make_shared<Mesh>();
	meshes[name] = mesh;

	return mesh;
}

bool ResourceManager::GetBoolParameter(const std::string& name)
{
	auto it = parameters.find(name);

	if (it != parameters.end())
	{
		if (it->second == "true" || it->second == "1")
			return true;
		else if (it->second == "false" || it->second == "0")
			return false;

		Logger::Log("Cannot convert value to bool; parameter name: ", name, "!\n");
	}
	else
		Logger::Log("Cannot find parameter with name: ", name, "!\n");

	return false;
}

int ResourceManager::GetIntParameter(const std::string& name)
{
	auto it = parameters.find(name);

	if (it != parameters.end())
		return std::stoi(it->second);
	else
		Logger::Log("Cannot find parameter with name: ", name, "!\n");

	return 0;
}

float ResourceManager::GetFloatParameter(const std::string& name)
{
	auto it = parameters.find(name);

	if (it != parameters.end())
		return std::stof(it->second);
	else
		Logger::Log("Cannot find parameter with name: ", name, "!\n");

	return 0.0f;
}

std::string ResourceManager::GetStringParameter(const std::string& name)
{
	auto it = parameters.find(name);

	if (it != parameters.end())
		return it->second;
	else
		Logger::Log("Cannot find parameter with name: ", name, "!\n");

	return std::string();
}

std::shared_ptr<Mesh> ResourceManager::GetMesh(const std::string& name)
{
	auto it = meshes.find(name);

	if (it != meshes.end())
		return it->second;
	else
		return LoadMesh(name);
}
