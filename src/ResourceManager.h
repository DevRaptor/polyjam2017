#pragma once

#include <map>
#include <memory>
#include <string>

class Mesh;

class ResourceManager
{
public:
	ResourceManager()
	{
		LoadParameters();
	}
	~ResourceManager() { }

	bool GetBoolParameter(const std::string& name);
	int GetIntParameter(const std::string& name);
	float GetFloatParameter(const std::string& name);
	std::string GetStringParameter(const std::string& name);

	std::shared_ptr<Mesh> GetMesh(const std::string& name);

private:
	std::map<std::string, std::string> parameters;

	/*
	In games with multiple objects, should store std::weak_ptr<Mesh> to
	delete unused meshes
	*/
	std::map<std::string, std::shared_ptr<Mesh>> meshes;

	void LoadParameters();
	std::shared_ptr<Mesh> LoadMesh(const std::string& name);
};
