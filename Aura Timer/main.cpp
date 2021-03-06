#include "stdafx.h"
#include <chrono>

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <tchar.h>
#include <thread>
#include <windows.h>
#include <AURASDK/AURALightingSDK.h>

EnumerateMbControllerFunc EnumerateMbController;
SetMbModeFunc SetMbMode;
SetMbColorFunc SetMbColor;
GetMbColorFunc GetMbColor;
GetMbLedCountFunc GetMbLedCount;

// Set an LED to a given RGB color.
void setLED(byte* _color, int led, int r, int g, int b)
{
	_color[3*led] = r;
	_color[3*led+1] = b;
	_color[3*led+2] = g;
}

void AuraTimer(int r, int g, int b)
{
	// Use _T to convert to text when compiling with Unicode support. This avoid having to call LoadLibraryA/LoadLibraryW explicitely.
	HMODULE auradll = nullptr;
	auradll = LoadLibrary(_T("AURASDK/AURA_SDK.dll"));

	if (auradll == nullptr)
	{
		printf("Failed to load AURA_SDK.dll\n");
		return;
	}
	else {
		printf("Successfully loaded AURA_SDK.dll\n");
	}

	// Get entry points of the API functions.
	(FARPROC&)EnumerateMbController = GetProcAddress(auradll, "EnumerateMbController");
	(FARPROC&)SetMbMode = GetProcAddress(auradll, "SetMbMode");
	(FARPROC&)SetMbColor = GetProcAddress(auradll, "SetMbColor");
	(FARPROC&)GetMbColor = GetProcAddress(auradll, "GetMbColor");
	(FARPROC&)GetMbLedCount = GetProcAddress(auradll, "GetMbLedCount");
	
	// Obtain the number of controllers.
	DWORD _count = EnumerateMbController(NULL, 0);
	MbLightControl* _mbLightCtrl = new MbLightControl[_count];
	EnumerateMbController(_mbLightCtrl, _count);

	DWORD _ledCount = GetMbLedCount(_mbLightCtrl[0]);
	
	BYTE* _curColorB = new BYTE[_ledCount * 3];
	ZeroMemory(_curColorB, _ledCount * 3);
	//std::cout << "Current color: \n";
	GetMbColor(_mbLightCtrl[0], _curColorB, _ledCount * 3);
	//for (std::size_t i = 0; i < _ledCount * 3; i++)
	//{
	//	printf("%d", _curColorB[i]);
	//}
	//SetMbColor(_mbLightCtrl[0], _curColorB, temp);
	
	BYTE *_color = new BYTE[_ledCount * 3];
	ZeroMemory(_color, _ledCount * 3);
	//std::cout << "\nColor: \n";
	//for (std::size_t i = 0; i < _ledCount * 3; i++)
	//{
	//	printf("%d", _color[i]);
	//}
	
	// 5 LEDs in my case, I/O shroud and an aura header.
	printf("\nLED Count: %2d\n", _ledCount);
	for (std::size_t i = 0; i < _ledCount; i++)
	{
		setLED(_color, i, r, g, b);
	}
	SetMbColor(_mbLightCtrl[0], _color, _ledCount * 3);

	// Clean up.
	delete[] _color;
	delete[] _curColorB;
	delete[] _mbLightCtrl;

	FreeLibrary(auradll);
	return;
}

// Uncomment to prevent console from spawning if not controlled by other settings.
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
int main(int argc, char *argv[])
{
	// Check if we have a time and RGB values.
	if (argc == 5)
	{
		// Read in time and RGB as separate strings.
		std::string time = argv[1];
		std::string rS = argv[2];
		std::string gS = argv[3];
		std::string bS = argv[4];

		// Convert everything into integers.
		int hh = std::stoi(time.substr(0, 2));
		int mm = std::stoi(time.substr(3, 2));
		int ss = std::stoi(time.substr(6, 2));

		int r = std::stoi(rS);
		int g = std::stoi(gS);
		int b = std::stoi(bS);

		// Use auto type to replace either one of the following:
		// std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
		// std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		// Get the current time.
		auto now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		std::tm *now_tm = std::localtime(&now_time_t);
		//std::cout << "The current time is: " << std::ctime(&now_time_t);

		// Copy the current date and time in tm format and edit the relevant parts.
		std::tm *temp = std::localtime(&now_time_t);
		struct std::tm go_tm = *temp;
		go_tm.tm_hour = hh;
		go_tm.tm_min = mm;
		go_tm.tm_sec = ss;

		// Convert the go time into time_t format.
		std::time_t go_time_t = mktime(&go_tm);
		// Calculate go_time - now_time.
		double difference = difftime(go_time_t, now_time_t);
		std::cout << "Difference: " << difference << std::endl;
		if (difference > 0)
		{
			// Go time lies in the future.
			// Turn off lights.
			AuraTimer(0, 0, 0);
			std::chrono::system_clock::time_point go_timepoint = std::chrono::system_clock::from_time_t(go_time_t);
			std::cout << "Sleeping until " << std::ctime(&go_time_t);
			std::this_thread::sleep_until(go_timepoint);
			std::cout << "Waking up!!";
			AuraTimer(r, g, b);
		} else {
			// Go time lies in the past. Just set the color immediately.
			AuraTimer(r, g, b);
		}
		return 0;
	} else {
		std::cout << "Usage: Aura Timer.exe <hh:mm:ss> <R> <G> <B>\n";
		return 1;
	}
}