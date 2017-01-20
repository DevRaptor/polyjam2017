#include "Log.h"

std::fstream Logger::log_file;

void Logger::InitLog()
{
	log_file.open("log.txt", std::ios::app | std::ios::out);
	Log("Init log\n");
}

void Logger::CloseLog()
{
	Log("Close log\n");
	log_file.close();
}