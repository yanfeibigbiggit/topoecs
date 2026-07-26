#include<expected>
#include "ankerl/unordered_dense.h"

export module core;

import std;
import typelist;

using id_t = unsigned int;
using uint_t = unsigned int;
using byte_t = unsigned char;

using xstring = std::string;
template<typename tkey, typename tvalue> using xmap = ankerl::unordered_dense::map<tkey, tvalue>;

namespace tecs
{
	namespace traits
	{
		//>>>>>>>> is_std_pair_v
		template <typename> struct is_std_pair : std::false_type {};

		template <typename T, typename U> struct is_std_pair<std::pair<T, U>> : std::true_type {};

		export template<typename T> constexpr bool is_std_pair_v = is_std_pair<T>::value;
		//<<<<<<<<

		//>>>>>>>> is_std_string_v
		template<typename> struct is_std_string_type : std::false_type {};

		template<> struct is_std_string_type<std::string> : std::true_type {};
		template<> struct is_std_string_type<const std::string> : std::true_type {};
		template<> struct is_std_string_type<std::string&> : std::true_type {};
		template<> struct is_std_string_type<const std::string&> : std::true_type {};
		template<> struct is_std_string_type<std::string&&> : std::true_type {};
		template<> struct is_std_string_type<const std::string&&> : std::true_type {};

		export template<typename T> constexpr bool is_std_string_v = is_std_string_type<T>::value;
		//<<<<<<<<

		//>>>>>>>> remove_qualifier_t
		template<typename T>
		struct remove_qualifier_type
		{
			using type = std::remove_cvref_t<std::remove_pointer_t<T>>;
		};
		export template<typename T> using remove_qualifier_t = remove_qualifier_type<T>::type;
		//<<<<<<<<

		//>>>>>>>> is_component_name_v
		template<typename CompNameType>
		struct is_component_name_type
		{
			using raw_t = remove_qualifier_t<CompNameType>;
			enum { value = std::is_same_v<raw_t, char> || is_std_string_v<CompNameType> };
		};

		export template<typename T> constexpr bool is_component_name_v = is_component_name_type<T>::value;
		//<<<<<<<<

		//>>>>>>>> is_all_same_v
		template<typename... Args>
		struct is_all_same_type;

		template<> struct is_all_same_type<> { enum { value = 1 }; };

		template<typename First> struct is_all_same_type<First> { enum { value = 1 }; };

		template<typename First, typename Second>
		struct is_all_same_type<First, Second>
		{
			using first_t = First;
			using next_t = Second;
			enum { value = std::is_same_v<First, Second> };
		};

		template<typename First, typename... Args>
		struct is_all_same_type<First, Args...>
		{
			using first_t = First;
			using next_t = is_all_same_type<Args...>::first_t;
			enum { value = std::is_same_v<first_t, next_t>&& is_all_same_type<Args...>::value };
		};

		export template<typename... Args> constexpr bool is_all_same_v = is_all_same_type<Args...>::value;
		//<<<<<<<<

		//>>>>>>>> is_all_component_name_v
		template<typename... Args> struct is_all_component_name_type;

		template<> struct is_all_component_name_type<> { enum { value = 1 }; };

		template<typename First>
		struct is_all_component_name_type<First>
		{
			using first_t = First;
			enum { value = traits::is_component_name_v<First> };
		};

		template<typename First, typename... Args>
		struct is_all_component_name_type<First, Args...>
		{
			using first_t = First;
			enum { value = traits::is_component_name_v<first_t> && is_all_component_name_type<Args...>::value };
		};

		export template<typename... Args> constexpr bool is_all_component_name_v = is_all_component_name_type<Args...>::value;
		//<<<<<<<<

		template<bool, typename TrueResult, typename FalseResult> struct select_if_else_t;

		template<typename TrueResult, typename FalseResult>
		struct select_if_else_t<true, TrueResult, FalseResult> { using type = TrueResult; };

		template<typename TrueResult, typename FalseResult>
		struct select_if_else_t<false, TrueResult, FalseResult> { using type = FalseResult; };


		template<typename One, typename Two>
		constexpr bool type_name_compare_compile_time()
		{
			return std::strcmp(typeid(One).name(), typeid(Two).name()) <= 0;
		}


	}

	namespace must
	{
		template<typename T> concept is_component_name_t = traits::is_component_name_v<T>;
	}

	namespace convert
	{
		template<typename StringType>
			requires must::is_component_name_t<StringType>
		const char* cast_component_name(StringType&& comp_name)
		{
			if constexpr (traits::is_std_string_v<StringType>)
			{
				return comp_name.c_str();
			}
			else
			{
				return comp_name;
			}
		}
	}

	class spinlock_t
	{
	public:
		std::atomic_flag flag;

		void lock(const std::memory_order order_ = std::memory_order::memory_order_seq_cst)
		{
			while (flag.test_and_set(order_));
		}
		void unlock()
		{
			flag.clear();
		}
	};

	namespace caller
	{
		//export template<typename... T> void type_id() {}
		//export using type_id_t = void(*)();

		template <typename T>
		constexpr const char* type_name_id()
		{
			const char* str;
#if defined(__clang__) || defined(__GNUC__)
			str = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			str = __FUNCSIG__;
#else
			str = char unknown compiler[-1];
#endif

			return str;
		}

		constexpr int compare_type_name_id(const char* s1, const char* s2)
		{
			//std::strlen and sizeof is not constexpr
			int skip = 0;
#if defined(_MSC_VER)
			skip = 47;
#endif
			for (int i = skip; ; i++)
			{
				auto c1 = s1[i]; auto c2 = s2[i];
				if (c1 == '\0' && c2 == '\0') { return 0; }
				if (c1 == c2) { continue; }
				if (c1 == '\0') { return 0; }
				if (c2 == '\0') { return 1; }
				return c1 - c2 < 0;
			}
			return 0;
		}

		template<size_t... I>
		constexpr auto index_sequence_to_constexpr_array(std::index_sequence<I...>)
		{
			return std::array<size_t, sizeof...(I)>{I...};
		}

		template<auto Array, size_t... I>
		constexpr auto constexpr_array_to_index_sequence(std::index_sequence<I...>)
		{
			return std::index_sequence<Array[I]...>{};
		}

		template<typename... Args>
		class type_order_sequence_t {};

		template<typename... Args>
		constexpr auto sort_index_sequence()
		{
			constexpr std::size_t N = sizeof...(Args);
			using index_name_pair = std::pair<std::size_t, const char*>;
			std::array<const char*, N> type_array{ type_name_id<Args>()... };
			std::array<index_name_pair, N> type_index_array;
			for (int i = 0; i < N; i++)
			{
				type_index_array[i].first = i;
				type_index_array[i].second = type_array[i];
			}

			for (int i = 0; i < N - 1; i++)
			{
				for (int j = 0; j < N - 1 - i; j++)
				{
					if (compare_type_name_id(type_index_array[j].second, type_index_array[j + 1].second))
					{
						auto temp = type_index_array[j + 1];
						type_index_array[j + 1] = type_index_array[j];
						type_index_array[j] = temp;
					}
				}
			}

			std::array<std::size_t, N> index_array;
			for (int i = 0; i < N; i++)
			{
				index_array[i] = type_index_array[i].first;
			}
			return index_array;
		}
		template<typename TList, size_t... I>
		constexpr auto make_order_type_sequence(std::index_sequence<I...> x)
		{
			using type = mpl::typelist<mpl::at<I, TList>::type...>;
			return type{};
		}

		export template<typename... Args>
			constexpr auto sort_type_sequence()
		{
			auto tid = make_order_type_sequence<mpl::typelist<Args...>>(
				constexpr_array_to_index_sequence< sort_index_sequence<Args...>()>(
					std::make_index_sequence<sizeof...(Args)>{}));

			return tid;
		}
	}
}

namespace tecs
{
	const id_t MASK_ID		= 0x00ffffff;
	const id_t MASK_VERSION	= 0xff000000;
	const id_t VERSION_ONE = 1 << 24;

	export const id_t ID_NULL = 0x00ffffff;

	inline uint_t get_id(id_t e) { return e & MASK_ID; }
	inline id_t set_id(id_t e, uint_t newid) { return e - get_id(e) + newid; }
	inline id_t get_version(id_t e) { return (e & MASK_VERSION) >> 24; }
	inline id_t set_version(id_t e, id_t v) { return get_id(e) + (v << 24); }
	inline id_t incr_version(id_t e) { return e + VERSION_ONE; }

	enum error_code_t { e_null_pointer, e_negative_integer, e_missing_component };
	

	export class entity_t
	{
	private:
		id_t id_v = 0;
		entity_t() {}
	public:
		static entity_t null;
	public:
		entity_t(uint_t id_, uint_t version_)
		{
			id_v = set_version(id_, version_);
		}

		uint_t id() { return get_id(id_v); }
		uint_t version() { return get_version(id_v); }
	};
	entity_t entity_t::null = { ID_NULL, 0 };

	class id_generator_t
	{
	private:
		id_t current;
		std::queue<id_t> free;

	public:
		id_generator_t()
		{
			current = 0;
		}

		id_t get()
		{
			id_t entity = 0;
			if (!free.empty())
			{
				entity = free.front();
				free.pop();
			}
			else
			{
				if (current >= ID_NULL)
				{
					//throw
					return ID_NULL;
				}
				entity = current;
				current += 1;
			}
			return entity;
		}

		void recycle(entity_t e)
		{
			id_t id_v = incr_version(e.id());
			free.push(id_v);
		}
	};

	class chunk_t
	{
	private:
		byte_t* bytes;
		uint_t chunk;
	public:
		chunk_t(uint_t element_size_, uint_t count_)
		{
			bytes = new byte_t[element_size_ * count_];
		}
		~chunk_t()
		{
			if (bytes != nullptr)
			{
				delete[] bytes;
				bytes = nullptr;
			}
		}

		inline void add(uint_t index_, byte_t* src_, uint_t element_size_)
		{
			std::copy(src_, src_ + element_size_, bytes + index_ * element_size_);
		}
	};

	class paged_array_t
	{
	public:
		uint_t bytes_per_element;
		uint_t elements_per_chunk;

		const uint_t CHUNK_BYTE_SIZE = 1 << 16;

		paged_array_t(uint_t bytes_per_element_, uint_t elements_per_chunk_)
		{
			bytes_per_element = bytes_per_element_;
			elements_per_chunk = elements_per_chunk_;
		}

		void add(uint_t id, byte_t* src, uint_t start_)
		{
			auto chunk_id = id & elements_per_chunk;
		}

	};

	template<typename Comp>
	class sparse_set_t
	{
		std::vector<short> sparse_array;
		std::vector<entity_t> dense_array;
	};


	export enum basic_type_t : byte_t
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

	export const xstring& get_basic_type_name(basic_type_t t)
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

	//////////////////////////////////////////////////////////////////////////////////////////////
	// component_t: description of one component
	//////////////////////////////////////////////////////////////////////////////////////////////
	export class component_t
	{
	private:
		class member_t
		{
		public:
			xstring name;
			basic_type_t type;
		};

	private:
		int _id;
		xstring _name;
		std::vector<member_t> _members;
		
	public:
		int id() { return _id; }
		const xstring& name() { return _name; }
		uint_t size() { return _members.size(); }

		component_t(const char* comp_name) { _name = comp_name; }

		template<typename... Args>
		void member(const char* member_name, basic_type_t basic_type, Args... args)
		{
			auto found = std::find_if(_members.begin(), _members.end(), [member_name](const member_t& item) { return item.name == member_name; });
			if (found != _members.end())
			{
				found->type = basic_type;
			}
			else
			{
				uint_t member_size = get_basic_type_size(basic_type);
				member_t item = { member_name, basic_type };
				_members.push_back(item);
			}

			member(args...);
		}

		xstring format_string()
		{
			xstring s;
			s.append("[component:").append(_name).append("]");
			for (auto item : _members)
			{
				s.append("(").append(item.name).append(",").append(get_basic_type_name(item.type)).append(")");
			}
			return s;
		}

	private:
		void member() {}

	private:
		static xmap<xstring, int> comp_name_map;
		static xmap<int, xstring> comp_id_map;
		static std::vector<component_t> comp_desc_array;

	public:
		template<typename... Args>
		static int define(const char* comp_name, const char* member_name, basic_type_t basic_type, Args... args)
		{
			static spinlock_t spin;
			spin.lock();
			int comp_id = get_id(comp_name);
			if (comp_id < 0)
			{
				component_t desc = component_t(comp_name);
				desc.member(member_name, basic_type, args...);

				comp_id = comp_desc_array.size();
				desc._id = comp_id;
				comp_desc_array.push_back(std::move(desc));

				comp_name_map[comp_name] = comp_id;
				comp_id_map[comp_id] = comp_name;
			}
			else
			{
				component_t& desc = comp_desc_array[comp_id];
				desc.member(member_name, basic_type, args...);
			}
			spin.unlock();
			return comp_id;
		}

		static bool has(const char* comp_name)
		{
			return comp_name_map.contains(comp_name);
		}

		static int get_id(const char* comp_name)
		{
			auto found = comp_name_map.find(comp_name);
			if (found == comp_name_map.end())
			{
				return -1;
			}
			return found->second;
		}

		static const component_t& get(int comp_id)
		{
			return comp_desc_array[comp_id];
		}

		static const component_t& get(const char* comp_name)
		{
			int comp_id = get_id(comp_name);
			return comp_desc_array[comp_id];
		}

		//format: [component:component_name](member_name_1, basic_type_t)(member_name_2, basic_type_t)...
		void parse_from(const char* format_)
		{

		}
	};
	xmap<xstring, int> component_t::comp_name_map;
	xmap<int, xstring> component_t::comp_id_map;
	std::vector<component_t> component_t::comp_desc_array;


	////////////////////////////////////////////////////////////////////////////////////////////////
	//// archetype_t: description of one archetype_t
	////////////////////////////////////////////////////////////////////////////////////////////////
	export class archetype_t
	{
	private:
		int id;
		std::vector<int> comp_id_array;

		template<const char* N>
		class type_of_name_t
		{

		};

		template<typename T>
		class name_of_type_t
		{
			static constexpr const char* name = typeid(T).name();
			static constexpr T* pointer = nullptr;
		};

	public:

		template<typename... CompNames>
		void add(const char* comp_name, CompNames... comps)
		{
			auto comp_id = component_t::get_id(comp_name);
			if (comp_id < 0)
			{
				return;
			}

			if (std::find(comp_id_array.begin(), comp_id_array.end(), comp_id) != comp_id_array.end())
			{
				return;
			}

			comp_id_array.push_back(comp_id);

			add(comps...);
		}
		void add()
		{
			std::sort(comp_id_array.begin(), comp_id_array.end());
		}		

		int has(const char* comp_name)
		{
			auto comp_id = component_t::get_id(comp_name);
			if(comp_id < 0)
			{
				return 0;
			}
			for (auto oneid : comp_id_array)
			{
				if(comp_id == oneid)
				{
					return 1;
				}
			}
			return 0;
		}

		xstring format_string()
		{
			xstring s;
			for (auto comp_id : comp_id_array)
			{
				auto desc = component_t::get(comp_id);
				s.append(desc.format_string()).append("\n");
			}
			return s;
		}

	public:
		template<typename... CompNames>
			requires traits::is_all_same_v<CompNames...>
		static int define(const char* comp, CompNames&&... comps)
		{
			//static_assert(traits::is_all_same_v<CompNames...>);

			uint_t tid = std::this_thread::get_id()._Get_underlying_id();
			
			auto comp_id = component_t::get_id(comp);
			std::cout << comp << ": " << comp_id << std::endl;
			
			define(std::forward<CompNames>(comps)...);
			return 0;
		}

		static void define(const char* comp_array[], int start, int count)
		{
			for (int i = 0; i < count; i++)
			{
				std::cout << comp_array[start + i] << std::endl;
			}
		}

		template<typename Iterator>
			requires std::bidirectional_iterator<Iterator> && (!must::is_component_name_t<Iterator>)//!must, or (const char*...)will enter here
		static void define(Iterator begin, Iterator end)
		{
			for (; begin != end; ++begin) 
			{
				if constexpr (traits::is_std_pair_v<std::decay_t<decltype(*begin)>>)
				{
					std::cout << begin->second << std::endl;
				}
				else
				{
					std::cout << *begin << std::endl;
				}
			}
		}

		template<typename... CompName>
		static int get(const char* comp_name, CompName... comps)
		{
			return 0;
		}

		static int get(const char* comp_array[], int count)
		{
			return 0;
		}

		private:
			static void define()
			{

			}
	};

	export class world_t
	{
	private:
		id_generator_t id_generator;
	public:
		entity_t entity()
		{
			auto id_v = id_generator.get();
			return entity_t::null;
		}
		bool is_alive() { return true; }
		void free()
		{

		}
		//template<typename comp_t> void add(id_t entity, comp_t comp)
		//{
		//	assert(sizeof(comp_t) == bytes_per_element);
		//	auto src = static_cast<byte_t*>(comp);
		//	add(entity, src, 0);
		//}
	};

}