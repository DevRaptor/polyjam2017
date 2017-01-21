#pragma once

#include <string>
#include <vector>
#include <map>

#include<SDL_mixer.h>

#include "utility/Log.h"

enum class MusicCommandType
{
	FADEIN,
	FADEOUT
};

struct MusicCommand
{
	MusicCommandType type;
	int fadetime;
	std::string nextname; // only for fade in effect
	int id;
	bool looped = false;
	bool defaultambient = false; // if true, its not automatically erasing from container
	//if manually MusicCommandType::FADEOUT, then erasing to command with defaultambient == true
	MusicCommand(MusicCommandType ptype, int fade, std::string name, int commandid,
				 bool plooped = false, bool pdefault = false)
		: type(ptype), fadetime(fade), nextname(name), id(commandid),
		  looped(plooped), defaultambient(pdefault)
	{ }
};

class Audio
{
		bool activated;

		std::map<std::string, Mix_Music*> musics;
		std::map<std::string, Mix_Chunk*> sounds;
		int soundsvolume;

		std::vector<MusicCommand> commands;

		std::string idmusic;
		int idcommand = 0;

		int actualcommand = -1; //id of actual command, if in top of stack is different id, must execute

		Mix_Music* GetIdMusic(const std::string& name);
		Mix_Chunk* GetIdSound(const std::string& name);

	public:
		Audio();
		~Audio();


		void AddMusic(const std::string& name, const char* path);

		void AddSound(const std::string& name, const std::string& path);


		void SetVolumeMusic(int volume)
		{
			Mix_VolumeMusic(volume); //0..128
		}

		//for single chunk
		void SetVolumeChunk(const std::string& name, int volume)
		{
			Mix_VolumeChunk(GetIdSound(name), volume);
		}

		//for all sounds
		void SetVolumeSounds(int volume)
		{
			soundsvolume = volume;
		}
		int SoundsSize();
		int MusicsSize();

		void PlayMusic(const std::string& name, int fadetime = 1000,
					   bool defaultambient = false, bool looped = false);
		void StopMusic(int fadetime, bool entirestate = false); //erasing musics to defaultambient

		int PlaySound(const std::string& name);
		void StopSound(int channel);

		void Update();

		void ExecuteCommand(); //execute command from top of stack
};
