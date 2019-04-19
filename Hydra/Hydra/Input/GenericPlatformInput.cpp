#include "Hydra/Input/GenericPlatformInput.h"

uint32 GenericPlatformInput::GetStandardPrintableKeyMap(uint32 * KeyCodes, String * KeyNames, uint32 MaxMappings, bool bMapUppercaseKeys, bool bMapLowercaseKeys)
{
	uint32 NumMappings = 0;

#define ADDKEYMAP(KeyCode, KeyName)		if (NumMappings<MaxMappings) { KeyCodes[NumMappings]=KeyCode; KeyNames[NumMappings]=KeyName; ++NumMappings; };

	ADDKEYMAP('0', String("Zero"));
	ADDKEYMAP('1', String("One"));
	ADDKEYMAP('2', String("Two"));
	ADDKEYMAP('3', String("Three"));
	ADDKEYMAP('4', String("Four"));
	ADDKEYMAP('5', String("Five"));
	ADDKEYMAP('6', String("Six"));
	ADDKEYMAP('7', String("Seven"));
	ADDKEYMAP('8', String("Eight"));
	ADDKEYMAP('9', String("Nine"));

	// we map both upper and lower
	if (bMapUppercaseKeys)
	{
		ADDKEYMAP('A', String("A"));
		ADDKEYMAP('B', String("B"));
		ADDKEYMAP('C', String("C"));
		ADDKEYMAP('D', String("D"));
		ADDKEYMAP('E', String("E"));
		ADDKEYMAP('F', String("F"));
		ADDKEYMAP('G', String("G"));
		ADDKEYMAP('H', String("H"));
		ADDKEYMAP('I', String("I"));
		ADDKEYMAP('J', String("J"));
		ADDKEYMAP('K', String("K"));
		ADDKEYMAP('L', String("L"));
		ADDKEYMAP('M', String("M"));
		ADDKEYMAP('N', String("N"));
		ADDKEYMAP('O', String("O"));
		ADDKEYMAP('P', String("P"));
		ADDKEYMAP('Q', String("Q"));
		ADDKEYMAP('R', String("R"));
		ADDKEYMAP('S', String("S"));
		ADDKEYMAP('T', String("T"));
		ADDKEYMAP('U', String("U"));
		ADDKEYMAP('V', String("V"));
		ADDKEYMAP('W', String("W"));
		ADDKEYMAP('X', String("X"));
		ADDKEYMAP('Y', String("Y"));
		ADDKEYMAP('Z', String("Z"));
	}

	if (bMapLowercaseKeys)
	{
		ADDKEYMAP('a', String("A"));
		ADDKEYMAP('b', String("B"));
		ADDKEYMAP('c', String("C"));
		ADDKEYMAP('d', String("D"));
		ADDKEYMAP('e', String("E"));
		ADDKEYMAP('f', String("F"));
		ADDKEYMAP('g', String("G"));
		ADDKEYMAP('h', String("H"));
		ADDKEYMAP('i', String("I"));
		ADDKEYMAP('j', String("J"));
		ADDKEYMAP('k', String("K"));
		ADDKEYMAP('l', String("L"));
		ADDKEYMAP('m', String("M"));
		ADDKEYMAP('n', String("N"));
		ADDKEYMAP('o', String("O"));
		ADDKEYMAP('p', String("P"));
		ADDKEYMAP('q', String("Q"));
		ADDKEYMAP('r', String("R"));
		ADDKEYMAP('s', String("S"));
		ADDKEYMAP('t', String("T"));
		ADDKEYMAP('u', String("U"));
		ADDKEYMAP('v', String("V"));
		ADDKEYMAP('w', String("W"));
		ADDKEYMAP('x', String("X"));
		ADDKEYMAP('y', String("Y"));
		ADDKEYMAP('z', String("Z"));
	}

	ADDKEYMAP(';', String("Semicolon"));
	ADDKEYMAP('=', String("Equals"));
	ADDKEYMAP(',', String("Comma"));
	ADDKEYMAP('-', String("Hyphen"));
	ADDKEYMAP('.', String("Period"));
	ADDKEYMAP('/', String("Slash"));
	ADDKEYMAP('`', String("Tilde"));
	ADDKEYMAP('[', String("LeftBracket"));
	ADDKEYMAP('\\', String("Backslash"));
	ADDKEYMAP(']', String("RightBracket"));
	ADDKEYMAP('\'', String("Apostrophe"));
	ADDKEYMAP(' ', String("SpaceBar"));

	// AZERTY Keys
	ADDKEYMAP('&', String("Ampersand"));
	ADDKEYMAP('*', String("Asterix"));
	ADDKEYMAP('^', String("Caret"));
	ADDKEYMAP(':', String("Colon"));
	ADDKEYMAP('$', String("Dollar"));
	ADDKEYMAP('!', String("Exclamation"));
	ADDKEYMAP('(', String("LeftParantheses"));
	ADDKEYMAP(')', String("RightParantheses"));
	ADDKEYMAP('"', String("Quote"));
	ADDKEYMAP('_', String("Underscore"));
	ADDKEYMAP(224, String("A_AccentGrave"));
	ADDKEYMAP(231, String("C_Cedille"));
	ADDKEYMAP(233, String("E_AccentAigu"));
	ADDKEYMAP(232, String("E_AccentGrave"));
	ADDKEYMAP(167, String("Section"));

#undef ADDKEYMAP

	return NumMappings;
}