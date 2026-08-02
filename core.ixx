//#include "ankerl/unordered_dense.h"
export module tecs:core;

#include "configs_macro.h"

import std;
import typelist;
import configs;

import :misc;
import :maths;
import :threads;

using id_t = unsigned int;
using uint_t = unsigned int;
using byte_t = unsigned char;

using size_t = std::size_t;
using xstring = std::string;
//template<typename tkey, typename tvalue> using xmap = ankerl::unordered_dense::map<tkey, tvalue>;


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
	public:
		static entity_t null;
	public:
		entity_t() {}
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


	////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename... Args>
	struct archetype_storage_t
	{
		using this_type = archetype_storage_t<Args...>;
		static constexpr size_t this_id = misc::type_id_archetype<this_type>;
		
		static constexpr size_t COUNT_PER_CHUNK = 
			maths::round_power_2(configs::ARCHETYPE_CHUNK_LENGTH / (misc::data_size_of_t<Args...>::value + sizeof(entity_t)));

		enum { map_component_archetype = misc::map_component_archetype<this_id, mpl::typelist<Args...>>::value };

		template<typename T>
		struct column_t
		{
			using container_t = std::array<T, COUNT_PER_CHUNK>;
			container_t values;
			
			T& operator[](size_t index)
			{
				return values[index];
			}
			const T& operator[](size_t index) const
			{
				return values[index];
			}
			//container_t::iterator begin() noexcept {
			//	return values.begin();
			//}

			//container_t::const_iterator end() const noexcept {
			//	return values.end();
			//}
		};

		struct chunk_t : column_t<Args>...
		{
			column_t<entity_t> entities;

			template<typename T>
			auto& column()
			{
				return column_t<T>::values;
			}

			template<typename T>
			void set(T t)
			{
				column_t<T>::operator[](0) = t;
			}
		};

		struct sparse_set_t
		{

		};

	private:
		static chunk_t columns;

	public:
		static void name() { std::cout << misc::type_name_id<this_type>() << std::endl; }

		template<typename... Ts>
		static void add(Ts...args) 
		{
			
		}

		template<typename T>
		static void set(T t)
		{
			columns.set(t);
		}
		template<typename T>
		static T get()
		{
			const auto& b = columns.column<T>();
			return b[0];
		}
	};
	template<typename... Args> typename archetype_storage_t<Args...>::chunk_t archetype_storage_t<Args...>::columns;

	template<typename... Args>
	struct archetype_define_t
	{
	private:
		using unpack_list_t = mpl::no_duplicate<typename mpl::unpack<Args...>::type>::type;

		static constexpr auto apply()
		{
			constexpr auto storage = apply_impl(
				misc::constexpr_array_to_index_sequence < 
					misc::sort_index_sequence(unpack_list_t{}) > (
						std::make_index_sequence<mpl::length<unpack_list_t>::value>{}));

			return storage;
		}

		template<size_t... I>
		static constexpr auto apply_impl(std::index_sequence<I...>)
		{
			using type = archetype_storage_t<mpl::at<I, unpack_list_t>::type...>;
			return type{};
		}

	public:
		using type = decltype(apply());
	};

	export template<typename... Args>
	using archetype_t = archetype_define_t<Args...>::type;

	/// <summary>
	/// forward declare for archetype_defines and query.
	/// put it here so query don't need to import archetype_defines
	/// </summary>
	/// <typeparam name="ID"></typeparam>
	export template<std::size_t ID> struct archetype_of_id_t;
}

//namespace NAMESPACE_COMPONENT_ARCHETYPE
//{

//}



