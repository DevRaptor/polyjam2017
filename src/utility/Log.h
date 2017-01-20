#pragma once

#include <fstream>
#include <iostream>

class Logger
{
public:
	static std::fstream log_file;

	template<typename T>
	static void Log(T arg)
	{
		log_file << arg;
		log_file.flush();
	}

	template<typename T1, typename ...Ts>
	static void Log(T1 arg, Ts... args)
	{
		Log(arg);
		Log(args...);
	}

	static void InitLog();
	static void CloseLog();
};