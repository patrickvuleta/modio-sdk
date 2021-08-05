#pragma once
#include <string>
#include <codecvt>
#include <locale>
#include "utf8.h"
//in the unlikely event codecvt ever actually gets deprecated, you can replace this with the utfcpp library at https://github.com/nemtrif/utfcpp

inline const std::wstring UTF8ToWideChar(std::string UTF8String)
{
	return std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t >{}.from_bytes(UTF8String);
}

inline const std::string WideCharToUTF8(std::wstring WideString)
{
	std::string Result;
	utf8::utf16to8(WideString.begin(), WideString.end(), back_inserter(Result));
	return Result;
}