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

////template< size_t n > // This type returns a number through function lookup.
////struct cn // The function returns cn<n>.
////{
////	char data[n + 1];
////}; // The caller uses (sizeof fn() - 1).
////
////template< typename id, size_t n, size_t acc >
////cn< acc > seen(id, cn< n >, cn< acc >); // Default fallback case.
////
/////* Evaluate the counter by finding the last defined overload.
////   Each function, when defined, alters the lookup sequence for lower-order
////   functions. */
////#define counter_read( id ) \
////( sizeof seen( id(), cn< 1 >(), cn< \
////( sizeof seen( id(), cn< 2 >(), cn< \
////( sizeof seen( id(), cn< 4 >(), cn< \
////( sizeof seen( id(), cn< 8 >(), cn< \
////( sizeof seen( id(), cn< 16 >(), cn< \
////( sizeof seen( id(), cn< 32 >(), cn< 0 \
/////* Add more as desired; trimmed for Stack Overflow code block. */ \
////                      >() ).data - 1 ) \
////                      >() ).data - 1 ) \
////                      >() ).data - 1 ) \
////                      >() ).data - 1 ) \
////                      >() ).data - 1 ) \
////                      >() ).data - 1 )
////
////   /* Define a single new function with place-value equal to the bit flipped to 1
////	  by the increment operation.
////	  This is the lowest-magnitude function yet undefined in the current context
////	  of defined higher-magnitude functions. */
////#define counter_inc( id ) \
////cn< counter_read( id ) + 1 > \
////seen( id, cn< ( counter_read( id ) + 1 ) & ~ counter_read( id ) >, \
////          cn< ( counter_read( id ) + 1 ) & counter_read( id ) > )




//#define COUNTER_READ_CRUMB( TAG, RANK, ACC ) counter_crumb( TAG(), constant_index< RANK >(), constant_index< ACC >() )
//#define COUNTER_READ( TAG ) COUNTER_READ_CRUMB( TAG, 1, COUNTER_READ_CRUMB( TAG, 2, COUNTER_READ_CRUMB( TAG, 4, COUNTER_READ_CRUMB( TAG, 8, \
//    COUNTER_READ_CRUMB( TAG, 16, COUNTER_READ_CRUMB( TAG, 32, COUNTER_READ_CRUMB( TAG, 64, COUNTER_READ_CRUMB( TAG, 128, 0 ) ) ) ) ) ) ) )
//
//#define COUNTER_INC( TAG ) \
//constexpr \
//constant_index< COUNTER_READ( TAG ) + 1 > \
//counter_crumb( TAG, constant_index< ( COUNTER_READ( TAG ) + 1 ) & ~ COUNTER_READ( TAG ) >, \
//                                                constant_index< ( COUNTER_READ( TAG ) + 1 ) & COUNTER_READ( TAG ) > ) { return {}; }
//
//#define COUNTER_LINK_NAMESPACE( NS ) using NS::counter_crumb;
//
//
//
//template< std::size_t n >
//struct constant_index : std::integral_constant< std::size_t, n > {};
//
//template< typename id, std::size_t rank, std::size_t acc >
//constexpr constant_index< acc > counter_crumb(id, constant_index< rank >, constant_index< acc >) { return {}; } // found by ADL via constant_index
//
//struct my_cnt {};
//
//int const a = counter_crumb(my_cnt(), constant_index< 1 >(), constant_index< 
//					counter_crumb(my_cnt(), constant_index< 2 >(), constant_index< 
//						counter_crumb(my_cnt(), constant_index< 4 >(), constant_index< 
//							counter_crumb(my_cnt(), constant_index< 8 >(), constant_index< 
//								counter_crumb(my_cnt(), constant_index< 16 >(), constant_index< 
//									counter_crumb(my_cnt(), constant_index< 32 >(), constant_index< 
//										counter_crumb(my_cnt(), constant_index< 64 >(), constant_index< 
//											counter_crumb(my_cnt(), constant_index< 128 >(), constant_index< 0 >()) >()) >()) >()) >()) >()) >()) >());
//
//constexpr constant_index< 
//	counter_crumb(my_cnt(), constant_index< 1 >(), constant_index< 
//		counter_crumb(my_cnt(), constant_index< 2 >(), constant_index< 
//			counter_crumb(my_cnt(), constant_index< 4 >(), constant_index< 
//				counter_crumb(my_cnt(), constant_index< 8 >(), constant_index< 
//					counter_crumb(my_cnt(), constant_index< 16 >(), constant_index< 
//						counter_crumb(my_cnt(), constant_index< 32 >(), constant_index< 
//							counter_crumb(my_cnt(), constant_index< 64 >(), constant_index< 
//								counter_crumb(my_cnt(), constant_index< 128 >(), constant_index< 0 >()) >()) >()) >()) >()) >()) >()) >()) + 1 > 
//	counter_crumb(  my_cnt, 
//					constant_index< (counter_crumb(my_cnt(), constant_index< 1 >(), constant_index< 
//										counter_crumb(my_cnt(), constant_index< 2 >(), constant_index< 
//											counter_crumb(my_cnt(), constant_index< 4 >(), constant_index< 
//												counter_crumb(my_cnt(), constant_index< 8 >(), constant_index< 
//													counter_crumb(my_cnt(), constant_index< 16 >(), constant_index< 
//														counter_crumb(my_cnt(), constant_index< 32 >(), constant_index< 
//															counter_crumb(my_cnt(), constant_index< 64 >(), constant_index< 
//																counter_crumb(my_cnt(), constant_index< 128 >(), constant_index< 0 >()) >()) >()) >()) >()) >()) >()) >()) + 1) 
//										& ~counter_crumb(my_cnt(), constant_index< 1 >(), constant_index< 
//											counter_crumb(my_cnt(), constant_index< 2 >(), constant_index< 
//												counter_crumb(my_cnt(), constant_index< 4 >(), constant_index< 
//													counter_crumb(my_cnt(), constant_index< 8 >(), constant_index< 
//														counter_crumb(my_cnt(), constant_index< 16 >(), constant_index< 
//															counter_crumb(my_cnt(), constant_index< 32 >(), constant_index< 
//																counter_crumb(my_cnt(), constant_index< 64 >(), constant_index< 
//																	counter_crumb(my_cnt(), constant_index< 128 >(), constant_index< 0 >()) >()) >()) >()) >()) >()) >()) >()) >, 
//					constant_index< (counter_crumb(my_cnt(), constant_index< 1 >(), constant_index< 
//										counter_crumb(my_cnt(), constant_index< 2 >(), constant_index< 
//											counter_crumb(my_cnt(), constant_index< 4 >(), constant_index< 
//												counter_crumb(my_cnt(), constant_index< 8 >(), constant_index< 
//													counter_crumb(my_cnt(), constant_index< 16 >(), constant_index< 
//														counter_crumb(my_cnt(), constant_index< 32 >(), constant_index< 
//															counter_crumb(my_cnt(), constant_index< 64 >(), constant_index< 
//																counter_crumb(my_cnt(), constant_index< 128 >(), constant_index< 0 >()) >()) >()) >()) >()) >()) >()) >()) + 1)
//											& counter_crumb(my_cnt(), constant_index< 1 >(), constant_index< 
//												counter_crumb(my_cnt(), constant_index< 2 >(), constant_index< 
//													counter_crumb(my_cnt(), constant_index< 4 >(), constant_index< 
//														counter_crumb(my_cnt(), constant_index< 8 >(), constant_index< 
//															counter_crumb(my_cnt(), constant_index< 16 >(), constant_index< 
//																counter_crumb(my_cnt(), constant_index< 32 >(), constant_index< 
//																	counter_crumb(my_cnt(), constant_index< 64 >(), constant_index< 
//																		counter_crumb(my_cnt(), constant_index< 128 >(), constant_index< 0 >()) >()) >()) >()) >()) >()) >()) >()) >) {
//		return {};
//};
//COUNTER_INC(my_cnt);
//COUNTER_INC(my_cnt);
//COUNTER_INC(my_cnt);
//COUNTER_INC(my_cnt);
//
//int const b = COUNTER_READ(my_cnt);
//
//COUNTER_INC(my_cnt);