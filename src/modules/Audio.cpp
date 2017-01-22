#include "Audio.h"

Audio::Audio()
{
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
	int result = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096); // or 48000
	Logger::Log("Audio driver: ",SDL_GetCurrentAudioDriver(),"\n");
	Logger::Log("Allocate channels: ",Mix_AllocateChannels(-1),"\n");
	soundsvolume = 100;

	idmusic = "";

	if(result == -1)
	{
		activated = false;
		Logger::Log("Cannot init audio module!\n"), std::cout<<"audio failed\n";
	}
	else
		activated = true;
}

Audio::~Audio()
{
	for(auto it = musics.begin(); it != musics.end(); ++it)
		Mix_FreeMusic((*it).second);

	for(auto it = sounds.begin(); it != sounds.end(); ++it)
		Mix_FreeChunk((*it).second);

	Mix_HaltMusic();
	Mix_CloseAudio();
	Mix_Quit();
}

Mix_Music* Audio::GetIdMusic(const std::string& name)
{
	if(musics.find(name) != musics.end())
	{
		return musics.find(name)->second;
	}
	else
	{
		Logger::Log("Error, music is not loaded\n");
		return NULL;
	}
}

void Audio::AddMusic(const std::string& name, const char* path)
{
	if(activated)
	{
		if(musics.find(name) == musics.end())
		{
			musics[name] = Mix_LoadMUS(path);
			if(musics[name] == NULL)
				Logger::Log("Error, music is not loaded. Error code: ",Mix_GetError(),"\n");
		}
		else
			Logger::Log("Music ", name, " was loaded earlier\n");
	}
}

Mix_Chunk* Audio::GetIdSound(const std::string& name)
{
	if(sounds.find(name)!= sounds.end())
	{
		return sounds.find(name)->second;
	}
	else
	{
		return NULL;
	}
}

void Audio::AddSound(const std::string& name, const std::string& path)
{
	if(activated)
	{
		sounds[name] =Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(sounds[name], soundsvolume);//Game::options->volumeSound);
		//need test validate of loading file
	}
}

int Audio::SoundsSize()
{
	return sounds.size();
}

int Audio::MusicsSize()
{
	return musics.size();
}


void Audio::PlayMusic(const std::string& name, int fadetime, bool defaultambient, bool looped)
{
	if(name != idmusic)
	{
		MusicCommand command(MusicCommandType::FADEIN, fadetime, name, idcommand,
							 looped,	defaultambient);
		idcommand++;
		commands.push_back(command);

		if(Mix_PlayingMusic())
		{
			//must fade out to eliminate break sound effect
			MusicCommand fadeout(MusicCommandType::FADEOUT, 1500, "", idcommand);
			idcommand++;
			commands.push_back(fadeout);
		}
	}

	idmusic = name;
}

void Audio::StopMusic(int fadeout, bool entirestate)
{
	Mix_FadeOutMusic(fadeout);
	if(Mix_PlayingMusic()) //manual fade out, must also pop last command
	{
		while( !commands.empty() && !commands.back().defaultambient)
			commands.pop_back();

		if(entirestate && !commands.empty()) //we must erase one default ambient
		{
			commands.pop_back();
		}
	}

	MusicCommand command(MusicCommandType::FADEOUT, fadeout, "", idcommand);
	idcommand++;
	commands.push_back(command);
}


int Audio::PlaySound(const std::string& name)
{
	return Mix_PlayChannel(-1, GetIdSound(name), 0);
}

void Audio::StopSound(int channel)
{
	if(channel != -1) // if -1, then interact with all channels
	{
		//Mix_HaltChannel(channel);
		Mix_FadeOutChannel(channel, 50);
	}
}

void Audio::Update()
{

	if(activated && !commands.empty())
	{
		if(actualcommand != commands.back().id)
		{
			ExecuteCommand();
		}

		if( !Mix_PlayingMusic()) //its silence, must pop command
		{
			commands.pop_back();

			ExecuteCommand();
		}
	}

}

void Audio::ExecuteCommand()
{
	MusicCommand& command = commands.back();

	if(command.type == MusicCommandType::FADEIN)
	{
		int value;
		if(command.looped == true)
			value = -1;
		else
			value = 1;

		auto it = musics.find(command.nextname);

		if(it != musics.end())
			Mix_FadeInMusic(it->second, value, command.fadetime);
		else
			Logger::Log("Music ", command.nextname, " not found\n");
	}
	else //fade out
	{
		Mix_FadeOutMusic(command.fadetime);
	}

	actualcommand = command.id;
	idmusic = command.nextname;
}

