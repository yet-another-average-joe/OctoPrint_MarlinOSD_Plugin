/*

	EventManager.h

*/

#pragma once


#include <string>
#include <vector>
#include <glob.h>


class CEventGrabber // static class
{
	// enumarates event devices ("/dev/input/event*)
	static std::vector<std::string> globVector(const std::string& pattern);

	// input file names
	static std::vector<std::string> file_names;
	
	// input file IDs
	static std::vector<int> file_desc;

	// status
	static bool isGrabbing;
	
public:

	static void disableHIDs();	// grabbs HID events
	static void enableHIDs();	// ungrabs HID events
};


