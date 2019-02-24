#include "Hydra/Input/Windows/WindowsPlatformInput.h"

#include <Windows.h>

namespace Hydra
{
	uint32 WindowsPlatformInput::GetKeyMap(uint32* KeyCodes, String* KeyNames, uint32 MaxMappings)
	{
#define ADDKEYMAP(KeyCode, KeyName)		if (NumMappings<MaxMappings) { KeyCodes[NumMappings]=KeyCode; KeyNames[NumMappings]=KeyName; ++NumMappings; };

		uint32 NumMappings = 0;

		if (KeyCodes && KeyNames && (MaxMappings > 0))
		{
			ADDKEYMAP(VK_LBUTTON, String("LeftMouseButton"));
			ADDKEYMAP(VK_RBUTTON, String("RightMouseButton"));
			ADDKEYMAP(VK_MBUTTON, String("MiddleMouseButton"));

			ADDKEYMAP(VK_XBUTTON1, String("ThumbMouseButton"));
			ADDKEYMAP(VK_XBUTTON2, String("ThumbMouseButton2"));

			ADDKEYMAP(VK_BACK, String("BackSpace"));
			ADDKEYMAP(VK_TAB, String("Tab"));
			ADDKEYMAP(VK_RETURN, String("Enter"));
			ADDKEYMAP(VK_PAUSE, String("Pause"));

			ADDKEYMAP(VK_CAPITAL, String("CapsLock"));
			ADDKEYMAP(VK_ESCAPE, String("Escape"));
			ADDKEYMAP(VK_SPACE, String("SpaceBar"));
			ADDKEYMAP(VK_PRIOR, String("PageUp"));
			ADDKEYMAP(VK_NEXT, String("PageDown"));
			ADDKEYMAP(VK_END, String("End"));
			ADDKEYMAP(VK_HOME, String("Home"));

			ADDKEYMAP(VK_LEFT, String("Left"));
			ADDKEYMAP(VK_UP, String("Up"));
			ADDKEYMAP(VK_RIGHT, String("Right"));
			ADDKEYMAP(VK_DOWN, String("Down"));

			ADDKEYMAP(VK_INSERT, String("Insert"));
			ADDKEYMAP(VK_DELETE, String("Delete"));

			ADDKEYMAP(VK_NUMPAD0, String("NumPadZero"));
			ADDKEYMAP(VK_NUMPAD1, String("NumPadOne"));
			ADDKEYMAP(VK_NUMPAD2, String("NumPadTwo"));
			ADDKEYMAP(VK_NUMPAD3, String("NumPadThree"));
			ADDKEYMAP(VK_NUMPAD4, String("NumPadFour"));
			ADDKEYMAP(VK_NUMPAD5, String("NumPadFive"));
			ADDKEYMAP(VK_NUMPAD6, String("NumPadSix"));
			ADDKEYMAP(VK_NUMPAD7, String("NumPadSeven"));
			ADDKEYMAP(VK_NUMPAD8, String("NumPadEight"));
			ADDKEYMAP(VK_NUMPAD9, String("NumPadNine"));

			ADDKEYMAP(VK_MULTIPLY, String("Multiply"));
			ADDKEYMAP(VK_ADD, String("Add"));
			ADDKEYMAP(VK_SUBTRACT, String("Subtract"));
			ADDKEYMAP(VK_DECIMAL, String("Decimal"));
			ADDKEYMAP(VK_DIVIDE, String("Divide"));

			ADDKEYMAP(VK_F1, String("F1"));
			ADDKEYMAP(VK_F2, String("F2"));
			ADDKEYMAP(VK_F3, String("F3"));
			ADDKEYMAP(VK_F4, String("F4"));
			ADDKEYMAP(VK_F5, String("F5"));
			ADDKEYMAP(VK_F6, String("F6"));
			ADDKEYMAP(VK_F7, String("F7"));
			ADDKEYMAP(VK_F8, String("F8"));
			ADDKEYMAP(VK_F9, String("F9"));
			ADDKEYMAP(VK_F10, String("F10"));
			ADDKEYMAP(VK_F11, String("F11"));
			ADDKEYMAP(VK_F12, String("F12"));

			ADDKEYMAP(VK_NUMLOCK, String("NumLock"));

			ADDKEYMAP(VK_SCROLL, String("ScrollLock"));

			ADDKEYMAP(VK_LSHIFT, String("LeftShift"));
			ADDKEYMAP(VK_RSHIFT, String("RightShift"));
			ADDKEYMAP(VK_LCONTROL, String("LeftControl"));
			ADDKEYMAP(VK_RCONTROL, String("RightControl"));
			ADDKEYMAP(VK_LMENU, String("LeftAlt"));
			ADDKEYMAP(VK_RMENU, String("RightAlt"));
			ADDKEYMAP(VK_LWIN, String("LeftCommand"));
			ADDKEYMAP(VK_RWIN, String("RightCommand"));

			Map<uint32, uint32> ScanToVKMap;
#define MAP_OEM_VK_TO_SCAN(KeyCode) { const uint32 CharCode = MapVirtualKey(KeyCode,2); if (CharCode != 0) { ScanToVKMap[CharCode] = KeyCode; } }
			MAP_OEM_VK_TO_SCAN(VK_OEM_1);
			MAP_OEM_VK_TO_SCAN(VK_OEM_2);
			MAP_OEM_VK_TO_SCAN(VK_OEM_3);
			MAP_OEM_VK_TO_SCAN(VK_OEM_4);
			MAP_OEM_VK_TO_SCAN(VK_OEM_5);
			MAP_OEM_VK_TO_SCAN(VK_OEM_6);
			MAP_OEM_VK_TO_SCAN(VK_OEM_7);
			MAP_OEM_VK_TO_SCAN(VK_OEM_8);
			MAP_OEM_VK_TO_SCAN(VK_OEM_PLUS);
			MAP_OEM_VK_TO_SCAN(VK_OEM_COMMA);
			MAP_OEM_VK_TO_SCAN(VK_OEM_MINUS);
			MAP_OEM_VK_TO_SCAN(VK_OEM_PERIOD);
			MAP_OEM_VK_TO_SCAN(VK_OEM_102);
#undef  MAP_OEM_VK_TO_SCAN

			static const uint32 MAX_KEY_MAPPINGS(256);
			uint32 CharCodes[MAX_KEY_MAPPINGS];
			String CharKeyNames[MAX_KEY_MAPPINGS];
			const int32 CharMappings = GetCharKeyMap(CharCodes, CharKeyNames, MAX_KEY_MAPPINGS);

			for (int32 MappingIndex = 0; MappingIndex < CharMappings; ++MappingIndex)
			{
				if (ScanToVKMap.find(CharCodes[MappingIndex]) != ScanToVKMap.end())
				{
					ScanToVKMap.erase(CharCodes[MappingIndex]);
				}
				else
				{
					ADDKEYMAP(CharCodes[MappingIndex], CharKeyNames[MappingIndex]);
				}
			}

			ITER(ScanToVKMap, It)
			{
				ADDKEYMAP(It->second, Chr(It->first));
			}
		}

		assert(NumMappings < MaxMappings);
		return NumMappings;

#undef ADDKEYMAP
	}

	uint32 WindowsPlatformInput::GetCharKeyMap(uint32* KeyCodes, String* KeyNames, uint32 MaxMappings)
	{
		return GenericPlatformInput::GetStandardPrintableKeyMap(KeyCodes, KeyNames, MaxMappings, true, false);
	}
}