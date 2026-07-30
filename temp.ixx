export module temp;

#include "macros.h"

import std;

using id_t = unsigned int;
using uint_t = unsigned int;
using byte_t = unsigned char;

using xstring = std::string;

MODULE_EXPORT enum basic_type_t : byte_t
{
	int8,
	uint8,
	int16,
	uint16,
	int32,
	uint32,
	int64,
	uint64,
	int128,
	uint128,
	float16,
	float32,
	float64,
	float128,
	//1 byte char
	char1,
	//2 byte char
	char2,
	//4 byte char
	char4,
	//8 byte char
	char8,
	//16 byte char
	char16,
	//32 byte char
	char32,
	//64 byte char
	char64,
	//128 byte char
	char128,

	//indirect int index, variadic string
	string,
	//indirect int index, variadic byte array
	bytearray,
};

uint_t get_basic_type_size(basic_type_t e)
{
	static const int layout[] = {
		//    int8, uint8, int16, uint16, int32, uint32, int64, uint64, int128, uint128
			  1,    1,     2,     2,      4,     4,      8,     8,      16,     16,
			  //    float16, float32, float64, float128,
					2,       4,       8,       16,
					//    char1, char2, char4, char8, char16, char64, char128
						  1,     2,     4,     8,     16,     64,     128,
						  //    string, bytearray
								4,     4,
	};
	return layout[e];
}

MODULE_EXPORT const xstring& get_basic_type_name(basic_type_t t)
{
	static const xstring names[] =
	{
		"int8",
		"uint8",
		"int16",
		"uint16",
		"int32",
		"uint32",
		"int64",
		"uint64",
		"int128",
		"uint128",
		"float16",
		"float32",
		"float64",
		"float128",
		"char1",
		"char2",
		"char4",
		"char8",
		"char16",
		"char32",
		"char64",
		"char128",

		"string",
		"bytearray",
	};

	return names[t];
}