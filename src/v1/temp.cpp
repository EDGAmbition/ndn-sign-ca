/*
#include <ndn-cxx/util/time.hpp>

std::string timeFormat(ndn::time::system_clock::TimePoint time);

int main(int argc, char** argv)
{
  ndn::time::system_clock::TimePoint now = ndn::time::system_clock::now();
  std::string startTimeFormat = timeFormat(now);
  //std::cout << now << std::endl;
  std::cout << startTimeFormat << std::endl;
  
  uint64_t timestampInMilliseconds = toUnixTimestamp(now).count();
  //std::cout << timestampInMilliseconds << std::endl;  
  timestampInMilliseconds += 604800000;
  ndn::time::system_clock::TimePoint endTime = 
  	ndn::time::fromUnixTimestamp(boost::chrono::milliseconds(timestampInMilliseconds));
  std::string endTimeFormat = timeFormat(endTime);
  std::cout << endTimeFormat << std::endl;
}

std::string timeFormat(ndn::time::system_clock::TimePoint time){
	std::string time_str = toIsoString(time);
	std::string time_str1 = time_str.substr(0,8);
  std::string time_str2 = time_str.substr(9,6);
  time_str = time_str1 + time_str2;
  return time_str;
}
*/
#include<iostream>
#include<fstream>
#include<functional>
#include<string>
#include<sstream>
#include <stdlib.h>
#include<string.h>
std::string readFile(const char *pFileName)
{
	std::ifstream inFile(pFileName);
	std::string contents("");
	if (inFile.is_open())
	{
		std::stringstream buffer;
		buffer << inFile.rdbuf();
		contents.append(buffer.str());	
	}
 
	inFile.close();
 
	return contents;
}
void 
runCmd(std::string cmd){
  char *cmd_c = new char[cmd.length() + 1];
  strcpy(cmd_c,cmd.c_str());
  system(cmd_c);
  delete [] cmd_c;
}

int main(int argc, char ** argv){
/*
	std::string str = argv[1];
	std::size_t str_hash = std::hash<std::string>{}(str);
	std::cout << std::to_string(str_hash) << std::endl;
*/
/*
	std::string location = "./key/test.cert";
  std::string contents = readFile(location);
  std::cout << contents;
	return 0;
*/
	runCmd("pwd");
	return 0;
}








