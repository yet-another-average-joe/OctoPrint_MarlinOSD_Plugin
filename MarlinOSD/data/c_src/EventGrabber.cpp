
/*
	EventGrabber.cpp

	https://jiafei427.wordpress.com/2017/03/13/linux-reading-the-mouse-events-datas-from-devinputmouse0/
	https://stackoverflow.com/questions/56758138/where-to-find-the-max-resolution-for-a-touchscreen-monitor-in-linux

	TODO :	also eats POWER BUTTON event !
			can't shutdown from "dtoverlay shutdwon input button" while HIDs are disabled !
			-> process POWER BUTTON event
*/

#include "EventGrabber.h"
#include "Settings.h"
#include "Errors.h"

#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>

//#include <stdlib.h>
//#include <iostream>

using namespace std;

bool CEventGrabber::isGrabbing = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HID devices enumeration

vector<std::string> CEventGrabber::globVector(const string& pattern)
{
	glob_t glob_result;
	glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
	vector<string> names;

	for (unsigned int i = 0; i < glob_result.gl_pathc; ++i)
		names.push_back(string(glob_result.gl_pathv[i]));

	globfree(&glob_result);

	return names;
}

// input file names
vector<std::string> CEventGrabber::file_names;

// input file ids
vector<int> CEventGrabber::file_desc;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// disables (grabs) HIDs

void CEventGrabber::disableHIDs()
{
	if (isGrabbing)
		return;

	file_names = globVector("/dev/input/event*");

	//if (file_names.size() == 0)
	//	exitError(ERROR_NO_EVENT_DEVICES, __FILE__, __LINE__);

	for (size_t i = 0; i < file_names.size(); i++)
	{
		int fd = open(file_names.at(i).c_str(), O_RDONLY | O_NONBLOCK);

		if (fd == -1)
			exitError(ERROR_FAILED_OPEN_EVENT_DEVICE, __FILE__, __LINE__);

		if (ioctl(fd, EVIOCGRAB, 1) == -1)
			exitError(ERROR_FAILED_GRAB_EVENTS, __FILE__, __LINE__);

		file_desc.push_back(fd); // enqueue
	}

	isGrabbing = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// re-enables HIDs

void CEventGrabber::enableHIDs()
{
	if (!isGrabbing)
		return;

	for (size_t i = 0; i < file_names.size(); i++)
	{
		if (ioctl(file_desc.at(i), EVIOCGRAB, 0) == -1)
			exitError(ERROR_FAILED_UNGRAB_EVENTS, __FILE__, __LINE__);

		if (close(file_desc.at(i)) == -1)
			exitError(ERROR_FAILED_CLOSE_EVENT_DEVICE, __FILE__, __LINE__);
	}

	file_names.clear();
	file_desc.clear();
	isGrabbing = false;
}


