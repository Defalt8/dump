#pragma once
#ifndef AXL_RESOURCE_JSON_HPP
#define AXL_RESOURCE_JSON_HPP

#include "../macros.hpp"
#include "../type_traits.hpp"
#include "../utilities.hpp"
#include "../memory.hpp"
#include "../limits.hpp"
#include "../math/functions.hpp"
#include "../pointer/Unique.hpp"
#include "../dsa/Array.hpp"
#include "../dsa/HashList.hpp"
#include "../dsa/HashMap.hpp"
#include "../dsa/String.hpp"
#include "../stream/Input.hpp"
#include "../stream/Output.hpp"
#include "../stream/Input.hpp"
#include "../stream/BufferedInput.hpp"
#include "../stream/BufferedOutput.hpp"

#ifndef AXL_JSON_ALLOCATOR
#define AXL_JSON_ALLOCATOR void 
#endif //AXL_JSON_ALLOCATOR

namespace axl {
namespace json {

struct Null;
struct Boolean;
struct Integer;
struct Number;
struct String;
struct Array;
struct Object;
struct Variant;
struct Entry;
struct PrintSettings;


using allocator_t   = AXL_JSON_ALLOCATOR; 
using null_t        = struct {}; 
using boolean_t     = bool;
using integer_t     = int64_t;
using number_t      = double;
using heap_string_t = axl::dsa::String<char_t>;
using string_t      = axl::dsa::String<char_t,allocator_t>;
using string_view_t = axl::dsa::StringView<char_t>;
using variant_t     = Variant;

static constexpr size_t hash_table_size_ = 256;
static constexpr auto   null             = null_t {};
static constexpr auto   inf              = (1e+300 * 1e+300);
static constexpr auto   nan              = (__builtin_nan("0"));
static constexpr auto   cstr_null        = "null"_axl;
static constexpr auto   cstr_true        = "true"_axl;
static constexpr auto   cstr_false       = "false"_axl;
static constexpr auto   cstr_null_string = ""_axl;
static thread_local allocator_t  *   allocator        = nullptr;
static thread_local bool             process_string   = true;
// \\uxxxx to unicode <= 65535 : default `true`
static thread_local bool             decode_utf8      = true;  
// \\Uxxxxxx to unicode <= 2097151 : default `true`
static thread_local bool             decode_utf8_ext  = true;  
// unicode <= 65535 to \\uxxxx : default `false`
static thread_local bool             encode_utf8      = false; 
// unicode > 65535 to \\Uxxxxxx : default `false`
static thread_local bool             encode_utf8_ext  = false; 


#ifdef AXL_JSON_USE_HASH_LIST_FOR_ARRAY
template <typename E> using array_t                  = axl::dsa::HashList<E,allocator_t,true,hash_table_size_,axl::dsa::EQ<E>>;
template <typename E> using array_iterator_t         = axl::dsa::HashListIterator<E,true,hash_table_size_,axl::dsa::EQ<E>>;
template <typename E> using const_array_iterator_t   = axl::dsa::ConstHashListIterator<E,true,hash_table_size_,axl::dsa::EQ<E>>;
#else
template <typename E> using array_t                  = axl::dsa::List<E,allocator_t>;
template <typename E> using array_iterator_t         = axl::dsa::ListIterator<E>;
template <typename E> using const_array_iterator_t   = axl::dsa::ConstListIterator<E>;
#endif // AXL_JSON_USE_HASH_LIST_FOR_ARRAY
template <typename E> using object_t                 = axl::dsa::HashMap<heap_string_t,E,allocator_t,hash_table_size_>;
template <typename E> using object_iterator_t        = axl::dsa::HashMapIterator<heap_string_t,E,hash_table_size_>;
template <typename E> using const_object_iterator_t  = axl::dsa::ConstHashMapIterator<heap_string_t,E,hash_table_size_>;
template <typename E> using object_entry_t           = axl::dsa::HashMapEntry<string_t,E>;
template <typename E> using unique_ptr_t             = axl::pointer::Unique<E,allocator_t>;

template <class A = void>
struct UniquePtrMaker
{
	template <typename T, typename... Args, axl::enable_if_t<(!axl::is::Constructible<T,A &,Args...>::value && axl::is::Constructible<T,Args...>::value),int> = 0>
	static inline unique_ptr_t<T>
	make_unique(Args &&... args) axl_except
	{
		axl_throw_if(!allocator, axl::null_pointer_exception("json::make_unique<>(): null allocator"));
		return axl::pointer::make_unique<T>(allocator, axl::forward<Args>(args)...);
	}

	template <typename T, typename... Args, axl::enable_if_t<axl::is::Constructible<T,A &,Args...>::value,int> = 0>
	static inline unique_ptr_t<T>
	make_unique(Args &&... args) axl_except
	{
		axl_throw_if(!allocator, axl::null_pointer_exception("json::make_unique<>(): null allocator"));
		return axl::pointer::make_unique<T>(allocator, *allocator, axl::forward<Args>(args)...);
	}
};

template <>
struct UniquePtrMaker<void>
{
	template <typename T, typename... Args, axl::enable_if_t<axl::is::Constructible<T,Args...>::value,int> = 0>
	static inline unique_ptr_t<T>
	make_unique(Args &&... args) axl_except
	{
		return axl::pointer::make_unique<T>(allocator, axl::forward<Args>(args)...);
	}
};


template <typename T, typename... Args>
static inline unique_ptr_t<T>
make_unique(Args &&... args) axl_except
{
	return UniquePtrMaker<allocator_t>::template make_unique<T>(axl::forward<Args>(args)...);
}


static constexpr char_t    hex_char(uint8_t value);
static constexpr uint8_t hex_value(char_t hex_char);

struct Null
{
	null_t m_value {};
	
	~Null() = default;
	Null() = default;
	Null(Null && rhs) = default;
	Null(Null const & rhs) = default;
	Null & operator=(Null && rhs) = default;
	Null & operator=(Null const & rhs) = default;
	
	Null(null_t const & value_) 
	{}
	
	null_t       & value()       axl_noexcept { return m_value; }
	null_t const & value() const axl_noexcept { return m_value; }
	
	operator null_t       & ()       axl_noexcept { return m_value; }
	operator null_t const & () const axl_noexcept { return m_value; }

};

struct Boolean
{
	boolean_t m_value {};
	
	~Boolean() {}
	Boolean() {}
	Boolean(Boolean && rhs) = default;
	Boolean(Boolean const & rhs) = default;
	Boolean & operator=(Boolean && rhs) = default;
	Boolean & operator=(Boolean const & rhs) = default;
	
	Boolean(boolean_t value_) 
		: m_value { value_ } 
	{}

	boolean_t       & value()       axl_noexcept { return m_value; }
	boolean_t const & value() const axl_noexcept { return m_value; }
	
	operator boolean_t       & ()       axl_noexcept { return m_value; }
	operator boolean_t const & () const axl_noexcept { return m_value; }

};

struct Integer
{
	integer_t m_value {};
	
	~Integer() = default;
	Integer() = default;
	Integer(Integer && rhs) = default;
	Integer(Integer const & rhs) = default;
	Integer & operator=(Integer && rhs) = default;
	Integer & operator=(Integer const & rhs) = default;
	
	Integer(integer_t value_) 
		: m_value { value_ } 
	{}
	
	template <typename T, axl::enable_if_t<axl::is::Integral<axl::remove_reference_and_cv_t<T>>::value,int> = 0>
	Integer(T && value_) 
		: m_value { static_cast<integer_t>(value_) } 
	{}

	integer_t       & value()       axl_noexcept { return m_value; }
	integer_t const & value() const axl_noexcept { return m_value; }

	operator integer_t       & ()       axl_noexcept { return m_value; }
	operator integer_t const & () const axl_noexcept { return m_value; }

};

struct Number
{
	number_t m_value {};
	
	~Number() = default;
	Number() = default;
	Number(Number && rhs) = default;
	Number(Number const & rhs) = default;
	Number & operator=(Number && rhs) = default;
	Number & operator=(Number const & rhs) = default;
	
	Number(number_t value_) 
		: m_value { value_ } 
	{}
	
	template <typename T, axl::enable_if_t<axl::is::FloatingPoint<axl::remove_reference_and_cv_t<T>>::value,int> = 0>
	Number(T && value_) 
		: m_value { static_cast<number_t>(value_) } 
	{}

	number_t       & value()       axl_noexcept { return m_value; }
	number_t const & value() const axl_noexcept { return m_value; }

	operator number_t       & ()       axl_noexcept { return m_value; }
	operator number_t const & () const axl_noexcept { return m_value; }

};

struct String
{
	unique_ptr_t<string_t> m_value {};
	
	~String() = default;
	String() = default;
	
	String & operator=(String && rhs) = default;

	String & 
	operator=(String const & rhs)
	{
		if(&rhs != this)
		{
			axl::destruct(this);
			axl::construct<String>(this, rhs);
		}
		return *this;
	}


	String(String && rhs) 
		: m_value { axl::move(rhs.m_value) } 
	{}
	
	String(String const & rhs) 
		: m_value { rhs.m_value ? make_unique<string_t>(*rhs.m_value) : unique_ptr_t<string_t>{} } 
	{}

	String(string_t value_) 
		: m_value { make_unique<string_t>(axl::move(value_)) } 
	{}

	template <typename... Args>
	String(Args &&... args) 
		: m_value { make_unique<string_t>(axl::forward<Args>(args)...) } 
	{}

	String(string_view_t const & value_) 
		: m_value { make_unique<string_t>(value_) } 
	{}

	String(char_t const * value_) 
		: m_value { make_unique<string_t>(value_) } 
	{}

	string_t       & value()       axl_except { return *m_value; }
	string_t const & value() const axl_except { return *m_value; }

	operator unique_ptr_t<string_t>       & ()       axl_noexcept { return m_value; }
	operator unique_ptr_t<string_t> const & () const axl_noexcept { return m_value; }

	bool operator!()       axl_noexcept { return !m_value; }
	bool operator!() const axl_noexcept { return !m_value; }
	explicit operator bool ()       axl_noexcept { return bool(m_value); }
	explicit operator bool () const axl_noexcept { return bool(m_value); }

};

struct Array
{
	unique_ptr_t<array_t<variant_t>> m_elements {};
	
	~Array() = default;
	Array() = default;
	Array(Array && rhs); 
	Array(Array const & rhs); 
	Array & operator=(Array && rhs) = default;
	
	Array & 
	operator=(Array const & rhs)
	{
		if(&rhs != this)
		{
			axl::destruct(this);
			axl::construct<Array>(this, rhs);
		}
		return *this;
	}
	
	template <size_t size_>
	Array(variant_t (&& elements_)[size_]); 

	template <typename... Args>
	array_iterator_t<variant_t> insert(Args &&... args);
	template <typename... Args>
	array_iterator_t<variant_t> rinsert(Args &&... args);

	array_t<variant_t> & elements() axl_except;
	array_t<variant_t> const & elements() const axl_except;

	variant_t       & operator[](size_t index)       axl_except;
	variant_t const & operator[](size_t index) const axl_except;
	
	operator unique_ptr_t<array_t<variant_t>> & () axl_noexcept;
	operator unique_ptr_t<array_t<variant_t>> const & () const axl_noexcept;

	bool operator!()       axl_noexcept;
	bool operator!() const axl_noexcept;
	explicit operator bool ()       axl_noexcept;
	explicit operator bool () const axl_noexcept;

};

struct Entry;

struct Object
{
	unique_ptr_t<object_t<variant_t>> m_entries {};
	
	~Object() = default;
	Object() axl_noexcept;
	Object(Object && rhs) axl_noexcept;
	Object(Object const & rhs);
	Object & operator=(Object && rhs) axl_noexcept = default;
	
	Object & 
	operator=(Object const & rhs)
	{
		if(&rhs != this)
		{
			axl::destruct(this);
			axl::construct<Object>(this, rhs);
		}
		return *this;
	}
	
	template <size_t size_>
	Object(Entry (&& entries_)[size_]);
	
	bool set(string_view_t const & key_, variant_t && value_) axl_except;
	bool set(string_view_t const & key_, variant_t const & value_) axl_except;
	bool set(String && key_, variant_t && value_) axl_except;
	bool set(String && key_, variant_t const & value_) axl_except;
	bool set(String const & key_, variant_t && value_) axl_except;
	bool set(String const & key_, variant_t const & value_) axl_except;
	bool remove(string_view_t const & key) axl_noexcept;

	template <typename T>
	bool get_boolean(string_view_t const & key, T & copy_to) const axl_noexcept;
	template <typename T>
	bool get_integer(string_view_t const & key, T & copy_to) const axl_noexcept;
	template <typename T>
	bool get_number (string_view_t const & key, T & copy_to) const axl_noexcept;
	template <typename T>
	bool get_string (string_view_t const & key, T & copy_to) const axl_noexcept;
	bool get_array  (string_view_t const & key, Array  & copy_to) const axl_noexcept;
	bool get_object (string_view_t const & key, Object & copy_to) const axl_noexcept;

	variant_t       & operator[](string_view_t const & key)       axl_except;
	variant_t const & operator[](string_view_t const & key) const axl_except;

	object_t<variant_t>       & entries()       axl_except;
	object_t<variant_t> const & entries() const axl_except;

	operator unique_ptr_t<object_t<variant_t>>       & () axl_noexcept;
	operator unique_ptr_t<object_t<variant_t>> const & () const axl_noexcept;
	
	bool operator!()       axl_noexcept;
	bool operator!() const axl_noexcept;
	explicit operator bool ()       axl_noexcept;
	explicit operator bool () const axl_noexcept;

};

struct Variant
{
	union 
	{
		Null    null;
		Boolean boolean;
		Integer integer;
		Number  number;
		String  string;
		Array   array;
		Object  object;
	};
	enum Index : uint8_t 
	{
		  invalid_i
		, null_i
		, boolean_i
		, integer_i
		, number_i
		, string_i
		, array_i
		, object_i
	};
	Index index { invalid_i };

	~Variant()
	{
		switch(index)
		{
			case null_i:    axl::destruct(&null);    break;
			case boolean_i: axl::destruct(&boolean); break;
			case integer_i: axl::destruct(&integer); break;
			case number_i:  axl::destruct(&number);  break;
			case string_i:  axl::destruct(&string);  break;
			case array_i:   axl::destruct(&array);   break;
			case object_i:  axl::destruct(&object);  break;
			default: 
			case invalid_i: break;
		}
		index = invalid_i;
	}

	Variant(Variant && rhs)
		: index { rhs.index }
	{
		switch(index)
		{
			case null_i:    axl::construct<Null>(&null); break;
			case boolean_i: axl::construct<Boolean>(&boolean, axl::move(rhs.boolean)); break;
			case integer_i: axl::construct<Integer>(&integer, axl::move(rhs.integer)); break;
			case number_i:  axl::construct<Number>(&number,  axl::move(rhs.number));  break;
			case string_i:  axl::construct<String>(&string,  axl::move(rhs.string));  break;
			case array_i:   axl::construct<Array>(&array,   axl::move(rhs.array));   break;
			case object_i:  axl::construct<Object>(&object,  axl::move(rhs.object));  break;
			default: 
			case invalid_i: break;
		}
	}
	
	Variant(Variant const & rhs)
		: index { rhs.index }
	{
		switch(index)
		{
			case null_i:    axl::construct<Null>(&null); break;
			case boolean_i: axl::construct<Boolean>(&boolean, rhs.boolean); break;
			case integer_i: axl::construct<Integer>(&integer, rhs.integer); break;
			case number_i:  axl::construct<Number>(&number,  rhs.number);   break;
			case string_i:  axl::construct<String>(&string,  rhs.string);   break;
			case array_i:   axl::construct<Array>(&array,   rhs.array);     break;
			case object_i:  axl::construct<Object>(&object,  rhs.object);   break;
			default: 
			case invalid_i: break;
		}
	}

	Variant & 
	operator=(Variant && rhs)
	{
		if(&rhs != this)
		{
			axl::destruct(this);
			axl::construct<Variant>(this, axl::move(rhs));
		}
		return *this;
	}

	Variant & 
	operator=(Variant const & rhs)
	{
		if(&rhs != this)
		{
			axl::destruct(this);
			axl::construct<Variant>(this, rhs);
		}
		return *this;
	}
	
	template <typename T>
	Variant & 
	operator=(T && rhs)
	{
		axl::destruct(this);
		axl::construct<Variant>(this, axl::forward<T>(rhs));
		return *this;
	}
	
	Variant()
	{}

	Variant(Null && value_)
		: null    {}
		, index   { null_i }
	{}

	Variant(Null const & value_)
		: null    {}
		, index   { null_i }
	{}

	Variant(null_t const & value_)
		: null    {}
		, index   { null_i }
	{}

	Variant(Boolean && value_)
		: boolean { axl::move(value_) }
		, index   { boolean_i }
	{}

	Variant(Boolean const & value_)
		: boolean { value_ }
		, index   { boolean_i }
	{}

	Variant(boolean_t value_)
		: boolean { value_ }
		, index   { boolean_i }
	{}

	Variant(Integer && value_)
		: integer { axl::move(value_) }
		, index   { integer_i }
	{}

	Variant(Integer const & value_)
		: integer { value_ }
		, index   { integer_i }
	{}

	template <typename T, axl::enable_if_t<axl::is::Integral<axl::remove_reference_and_cv_t<T>>::value,int> = 0>
	Variant(T && value_)
		: integer { static_cast<integer_t>(value_) }
		, index   { integer_i }
	{}

	Variant(Number && value_)
		: number  { axl::move(value_) }
		, index   { number_i }
	{}

	Variant(Number const & value_)
		: number  { value_ }
		, index   { number_i }
	{}

	template <typename T, axl::enable_if_t<axl::is::FloatingPoint<axl::remove_reference_and_cv_t<T>>::value,int> = 0>
	Variant(T && value_)
		: number  { static_cast<number_t>(value_) }
		, index   { number_i }
	{}

	Variant(String && value_)
		: string  { axl::move(value_) }
		, index   { string_i }
	{}
	
	Variant(String const & value_)
		: string  { value_ }
		, index   { string_i }
	{}
	
	Variant(string_t const & value_)
		: string  { value_ }
		, index   { string_i }
	{}
	
	Variant(string_view_t const & value_)
		: string  { value_ }
		, index   { string_i }
	{}
	
	Variant(char_t const * value_)
		: string  { value_ }
		, index   { string_i }
	{}
	
	Variant(Array && value_)
		: array   { axl::move(value_) }
		, index   { array_i }
	{}
	
	Variant(Array const & value_)
		: array   { value_ }
		, index   { array_i }
	{}

	template <size_t size_>
	Variant(variant_t (&& elements_)[size_]) 
		: array   { axl::move(elements_) }
		, index   { array_i } 
	{}

	Variant(Object && value_)
		: object  { axl::move(value_) }
		, index   { object_i }
	{}
	
	Variant(Object const & value_)
		: object  { value_ }
		, index   { object_i }
	{}

	template <size_t size_>
	Variant(Entry (&& entries_)[size_])
		: object { axl::move(entries_) }
		, index  { object_i }
	{}

	variant_t & 
	operator[](string_view_t const & key) axl_except
	{
		axl_throw_if(index != object_i, axl::runtime_error_exception("Variant::operator[](key): this variant is not an object"));
		return object.entries()[key];
	}

	variant_t const & 
	operator[](string_view_t const & key) const axl_except
	{
		axl_throw_if(index != object_i, axl::runtime_error_exception("Variant::operator[](key): this variant is not an object"));
		return object.entries()[key];
	}

	template <typename T>
	constexpr bool 
	operator==(T const & rhs) const axl_noexcept 
	{
		return this->operator==(Variant(rhs));
	}

	template <typename T>
	constexpr bool 
	operator!=(T const & rhs) const axl_noexcept 
	{
		return this->operator!=(Variant(rhs));
	}

	constexpr bool 
	operator==(Variant const & rhs) const axl_noexcept 
	{
		if(rhs.index != index)
			return false;
		switch(index)
		{
			case null_i:    return true;
			case boolean_i: return boolean.m_value  == rhs.boolean.m_value;
			case integer_i: return integer.m_value  == rhs.integer.m_value;
			case number_i:  return number.m_value   == rhs.number.m_value;
			case string_i:  return string.m_value   == rhs.string.m_value || ((string.m_value && rhs.string.m_value) && *(string.m_value.ptr()) == *(rhs.string.m_value.ptr()));
			case array_i:   return array.m_elements == rhs.array.m_elements;
			case object_i:  return object.m_entries == rhs.object.m_entries;
			default: 
			case invalid_i: return false;
		} 
	}

	constexpr bool 
	operator!=(Variant const & rhs) const axl_noexcept 
	{
		if(rhs.index != index)
			return true;
		switch(index)
		{
			case null_i:    return false;
			case boolean_i: return boolean.m_value  != rhs.boolean.m_value;
			case integer_i: return integer.m_value  != rhs.integer.m_value;
			case number_i:  return number.m_value   != rhs.number.m_value;
			case string_i:  return string.m_value   != rhs.string.m_value || ((string.m_value && rhs.string.m_value) && *(string.m_value.ptr()) != *(rhs.string.m_value.ptr())) || (bool(string.m_value) == !rhs.string.m_value);
			case array_i:   return array.m_elements != rhs.array.m_elements;
			case object_i:  return object.m_entries != rhs.object.m_entries;
			default: 
			case invalid_i: return true;
		} 
	}

	bool operator!()       axl_noexcept { return index == invalid_i; }
	bool operator!() const axl_noexcept { return index == invalid_i; }
	explicit operator bool ()       axl_noexcept { return index != invalid_i; }
	explicit operator bool () const axl_noexcept { return index != invalid_i; }

};

/// Array definition

Array::Array(Array && rhs) 
	: m_elements { axl::move(rhs.m_elements) }
{}

Array::Array(Array const & rhs) 
	: m_elements { rhs.m_elements ? make_unique<array_t<variant_t>>(*rhs.m_elements) : unique_ptr_t<array_t<variant_t>>{} }
{}

template <size_t size_>
Array::Array(variant_t (&& elements_)[size_]) 
	: m_elements { make_unique<array_t<variant_t>>(axl::move(elements_)) } 
{}

template <typename... Args>
array_iterator_t<variant_t> 
Array::insert(Args &&... args)
{
	axl_throw_if(!m_elements, axl::null_pointer_exception("json::Array::insert(): uninitialized elements"));
	auto & elements = *m_elements.ptr();
	return elements.emplace(axl::forward<Args>(args)...);
}

template <typename... Args>
array_iterator_t<variant_t> 
Array::rinsert(Args &&... args)
{
	axl_throw_if(!m_elements, axl::null_pointer_exception("json::Array::insert(): uninitialized elements"));
	auto & elements = *m_elements.ptr();
	return elements.remplace(axl::forward<Args>(args)...);
}

array_t<variant_t> & 
Array::elements() axl_except
{ return *m_elements; }

array_t<variant_t> const & 
Array::elements() const axl_except
{ return *m_elements; }

variant_t & 
Array::operator[](size_t index) axl_except
{
	axl_throw_if(!m_elements, axl::null_pointer_exception("json::Array::operator[]"));
	auto & elements_ = *m_elements.ptr();
	auto it = elements_.begin();
	axl_throw_if(index >= elements_.size(), axl::index_out_of_bounds_exception("json::Array::operator[]", index, elements_.size()));
	for(; index > 0 && it; ++it);
	return *it;
}

variant_t const & 
Array::operator[](size_t index) const axl_except
{
	axl_throw_if(!m_elements, axl::null_pointer_exception("json::Array::operator[]"));
	auto const & elements_ = *m_elements.ptr();
	auto it = elements_.begin();
	axl_throw_if(index >= elements_.size(), axl::index_out_of_bounds_exception("json::Array::operator[]", index, elements_.size()));
	for(; index > 0 && it; ++it);
	return *it;
}

Array::operator unique_ptr_t<array_t<variant_t>>       & ()       axl_noexcept { return m_elements; }
Array::operator unique_ptr_t<array_t<variant_t>> const & () const axl_noexcept { return m_elements; }

bool Array::operator!()       axl_noexcept { return !m_elements; }
bool Array::operator!() const axl_noexcept { return !m_elements; }
Array::operator bool ()       axl_noexcept { return bool(m_elements); }
Array::operator bool () const axl_noexcept { return bool(m_elements); }

/// Object definition

struct Entry
{
	heap_string_t key;
	variant_t    value;
	
	template <typename KT = heap_string_t, typename VT = variant_t>
	Entry(KT && key_, VT && value_)
		: key   (axl::forward<KT>(key_))
		, value (axl::forward<VT>(value_))
	{}

};

Object::Object() axl_noexcept 
	: m_entries {}
{}

Object::Object(Object && rhs) axl_noexcept 
	: m_entries { axl::move(rhs.m_entries) }
{}

Object::Object(Object const & rhs) 
	: m_entries { rhs.m_entries ? make_unique<object_t<variant_t>>(*rhs.m_entries) : unique_ptr_t<object_t<variant_t>>{} }
{}

template <size_t size_>
Object::Object(Entry (&& entries_)[size_])
	: m_entries { make_unique<object_t<variant_t>>(axl::move(entries_)) } 
{}

object_t<variant_t> & 
Object::entries() axl_except
{ return *m_entries; }

object_t<variant_t> const & 
Object::entries() const axl_except
{ return *m_entries; }

bool 
Object::set(string_view_t const & key_, variant_t && value_) axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::set(): uninitialized entries"));
	auto & entries_ = *m_entries.ptr();
	entries_.emplace(key_, axl::move(value_));
	return false;
}

bool 
Object::set(string_view_t const & key_, variant_t const & value_) axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::set(): uninitialized entries"));
	auto & entries_ = *m_entries.ptr();
	entries_.emplace(key_, value_);
	return false;
}

bool 
Object::set(String && key_, variant_t && value_) axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::set(): uninitialized entries"));
	auto & entries_ = *m_entries.ptr();
	entries_.emplace(axl::move(key_.value()), axl::move(value_));
	return false;
}

bool 
Object::set(String && key_, variant_t const & value_) axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::set(): uninitialized entries"));
	auto & entries_ = *m_entries.ptr();
	entries_.emplace(axl::move(key_.value()), value_);
	return false;
}

bool 
Object::set(String const & key_, variant_t && value_) axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::set(): uninitialized entries"));
	auto & entries_ = *m_entries.ptr();
	entries_.emplace(key_.value(), axl::move(value_));
	return false;
}

bool 
Object::set(String const & key_, variant_t const & value_) axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::set(): uninitialized entries"));
	auto & entries_ = *m_entries.ptr();
	entries_.emplace(key_.value(), value_);
	return false;
}


bool 
Object::remove(string_view_t const & key) axl_noexcept
{
	if(!m_entries)
		return false;
	return (*m_entries.ptr()).remove(key);
}

template <typename T>
bool
Object::get_boolean(string_view_t const & key, T & copy_to) const axl_noexcept
{
	if(m_entries)
	{
		auto const & entries = *m_entries.ptr();
		auto it = entries.position_of(key);
		if(it)
		{
			auto const & var = it.ptr()->value();
			switch(var.index)
			{
				case Variant::boolean_i: copy_to = static_cast<T>(var.boolean);  return true;
				case Variant::null_i: return true;
				default: break;
			}
		}
	}
	return false;
}

template <typename T>
bool
Object::get_integer(string_view_t const & key, T & copy_to) const axl_noexcept
{
	if(m_entries)
	{
		auto const & entries = *m_entries.ptr();
		auto it = entries.position_of(key);
		if(it)
		{
			auto const & var = it.ptr()->value();
			switch(var.index)
			{
				case Variant::integer_i: copy_to = static_cast<T>(var.integer);  return true;
				case Variant::null_i: return true;
				default: break;
			}
		}
	}
	return false;
}

template <typename T>
bool
Object::get_number(string_view_t const & key, T & copy_to) const axl_noexcept
{
	if(m_entries)
	{
		auto const & entries = *m_entries.ptr();
		auto it = entries.position_of(key);
		if(it)
		{
			auto const & var = it.ptr()->value();
			switch(var.index)
			{
				case Variant::number_i: copy_to = static_cast<T>(var.number);  return true;
				case Variant::null_i: return true;
				default: break;
			}
		}
	}
	return false;
}

template <typename T>
bool
Object::get_string(string_view_t const & key, T & copy_to) const axl_noexcept
{
	if(m_entries)
	{
		auto const & entries = *m_entries.ptr();
		auto it = entries.position_of(key);
		if(it)
		{
			auto const & var = it.ptr()->value();
			switch(var.index)
			{
				case Variant::string_i:  copy_to = static_cast<T>(var.string.value());  return true;
				case Variant::null_i: return true;
				default: break;
			}
		}
	}
	return false;
}

bool
Object::get_array(string_view_t const & key, Array  & copy_to) const axl_noexcept
{
	if(m_entries)
	{
		auto const & entries = *m_entries.ptr();
		auto it = entries.position_of(key);
		if(it)
		{
			auto const & var = it.ptr()->value();
			switch(var.index)
			{
				case Variant::array_i: copy_to = var.array;  return true;
				case Variant::null_i: return true;
				default: break;
			}
		}
	}
	return false;
}

bool
Object::get_object(string_view_t const & key, Object & copy_to) const axl_noexcept
{
	if(m_entries)
	{
		auto const & entries = *m_entries.ptr();
		auto it = entries.position_of(key);
		if(it)
		{
			auto const & var = it.ptr()->value();
			switch(var.index)
			{
				case Variant::object_i: copy_to = var.object;  return true;
				case Variant::null_i: return true;
				default: break;
			}
		}
	}
	return false;
}


variant_t & 
Object::operator[](string_view_t const & key) axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::operator[]"));
	return (*m_entries)[key];
}

variant_t const & 
Object::operator[](string_view_t const & key) const axl_except
{
	axl_throw_if(!m_entries, axl::null_pointer_exception("json::Object::operator[]"));
	return (*m_entries)[key];
}

Object::operator unique_ptr_t<object_t<variant_t>>       & ()       axl_noexcept { return m_entries; }
Object::operator unique_ptr_t<object_t<variant_t>> const & () const axl_noexcept { return m_entries; }

bool Object::operator!()       axl_noexcept { return !m_entries; }
bool Object::operator!() const axl_noexcept { return !m_entries; }
Object::operator bool ()       axl_noexcept { return bool(m_entries); }
Object::operator bool () const axl_noexcept { return bool(m_entries); }

struct PrintSettings
{
	int  depth     = 0;
	bool readable  = true;
	bool _indent   = true;
};

static inline axl::stream::Output & print(axl::stream::Output & ostream, Null const & rhs, PrintSettings const & ps = {});
static inline axl::stream::Output & print(axl::stream::Output & ostream, Boolean const & rhs, PrintSettings const & ps = {});
static inline axl::stream::Output & print(axl::stream::Output & ostream, Integer const & rhs, PrintSettings const & ps = {});
static inline axl::stream::Output & print(axl::stream::Output & ostream, Number const & rhs, PrintSettings const & ps = {});
static inline axl::stream::Output & print(axl::stream::Output & ostream, String const & rhs, PrintSettings const & ps = {});
static inline axl::stream::Output & print(axl::stream::Output & ostream, Array const & rhs, PrintSettings const & ps = {});
static        axl::stream::Output & print(axl::stream::Output & ostream, Object const & rhs, PrintSettings const & ps = {});
static        axl::stream::Output & print(axl::stream::Output & ostream, Variant const & rhs, PrintSettings const & ps = {});
static        axl::stream::Output & print(axl::stream::Output & ostream, object_t<variant_t>::element_t const & rhs, PrintSettings const & ps = {});

static inline axl::stream::Output &
print_newline(axl::stream::Output & ostream)
{
	return ostream << '\n';
}

static inline axl::stream::Output &
print_indent(axl::stream::Output & ostream, int n)
{
	return ostream << axl::stream::repeat('\t', n);
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, Null const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	return ostream << cstr_null;
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, Boolean const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	return ostream << (rhs.value() ? cstr_true : cstr_false);
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, Integer const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	return ostream << rhs.value();
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, Number const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	auto const & value = rhs.value();
	if(value != value)
		return ostream << "null";
	else if(value == inf)
		return ostream << "9e+999";
	else if(value == -inf)
		return ostream << "-9e+999";
	return ostream << value;
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, String const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	if(process_string)
	{
		ostream << '"';
		if(rhs)
		{
			auto const & str_ = rhs.value();
			size_t length_ = str_.length();
			char_t const * begin_ = str_.begin();
			for(size_t i = 0; i < length_; ++i)
			{
				char_t ch = begin_[i];
				switch(ch)
				{
					case '\\':
					{
						if(i < (length_ - 1) && ((!decode_utf8 && begin_[i+1] == 'u') || (!decode_utf8_ext && begin_[i+1] == 'U')))
						{
							char_t es[]{ '\\', begin_[++i] }; 
							ostream.write(&es[0], sizeof(char_t), 2);
						}
						else
						{
							constexpr char_t es[]{ '\\', '\\' }; 
							ostream.write(&es[0], sizeof(char_t), 2);
						}
						break;
					}
					case '"':
					{
						constexpr char_t es[]{ '\\', '"' }; 
						ostream.write(&es[0], sizeof(char_t), 2);
						break;
					}
					case '\n':
					{
						constexpr char_t es[]{ '\\', 'n' }; 
						ostream.write(&es[0], sizeof(char_t), 2);
						break;
					}
					case '\r':
					{
						constexpr char_t es[]{ '\\', 'r' }; 
						ostream.write(&es[0], sizeof(char_t), 2);
						break;
					}
					case '\t':
					{
						constexpr char_t es[]{ '\\', 't' }; 
						ostream.write(&es[0], sizeof(char_t), 2);
						break;
					}
					case '\b':
					{
						constexpr char_t es[]{ '\\', 'b' }; 
						ostream.write(&es[0], sizeof(char_t), 2);
						break;
					}
					case '\f':
					{
						constexpr char_t es[]{ '\\', 'f' }; 
						ostream.write(&es[0], sizeof(char_t), 2);
						break;
					}
					default:
					{
						if(encode_utf8 || encode_utf8_ext)
						{
							if((ch & 0xF8) == 0xF0 && encode_utf8_ext)
							{
								if(i < length_ - 3)
								{
									char_t bytes_[4] { ch, begin_[i+1], begin_[i+2], begin_[i+3] };
									i += 3;
									uint32_t unicode_ = 
										  ((bytes_[0] & 0x07) << 18)
										| ((bytes_[1] & 0x3F) << 12)
										| ((bytes_[2] & 0x3F) << 6)
										| ((bytes_[3] & 0x3F));
									char_t decoded[] {
										  '\\'
										, 'U'
										, hex_char((unicode_ >> 20) & 0xF)
										, hex_char((unicode_ >> 16) & 0xF)
										, hex_char((unicode_ >> 12) & 0xF)
										, hex_char((unicode_ >> 8) & 0xF)
										, hex_char((unicode_ >> 4) & 0xF)
										, hex_char((unicode_) & 0xF)
									}; 
									ostream.write(&decoded[0], sizeof(char_t), 8);
								}
								else
									ostream.write(&ch, sizeof(char_t), 1);
							}
							else if((ch & 0xF0) == 0xE0 && encode_utf8)
							{
								if(i < length_ - 2)
								{
									char_t bytes_[3] { ch, begin_[i+1], begin_[i+2] };
									i += 2;
									uint16_t unicode_ = 
										  ((bytes_[0] & 0x0F) << 12)
										| ((bytes_[1] & 0x3F) << 6)
										| ((bytes_[2] & 0x3F));
									char_t decoded[] {
										  '\\'
										, 'u'
										, hex_char((unicode_ >> 12) & 0xF)
										, hex_char((unicode_ >> 8) & 0xF)
										, hex_char((unicode_ >> 4) & 0xF)
										, hex_char((unicode_) & 0xF)
									}; 
									ostream.write(&decoded[0], sizeof(char_t), 6);
								}
								else
									ostream.write(&ch, sizeof(char_t), 1);
							}
							else if((ch & 0xE0) == 0xC0 && encode_utf8)
							{
								if(i < length_ - 1)
								{
									char_t bytes_[2] { ch, begin_[i] };
									++i;
									uint16_t unicode_ = 
										  ((bytes_[0] & 0x1F) << 6)
										| ((bytes_[1] & 0x3F));
									char_t decoded[] {
										  '\\'
										, 'u'
										, hex_char((unicode_ >> 12) & 0xF)
										, hex_char((unicode_ >> 8) & 0xF)
										, hex_char((unicode_ >> 4) & 0xF)
										, hex_char((unicode_) & 0xF)
									}; 
									ostream.write(&decoded[0], sizeof(char_t), 6);
								}
								else
									ostream.write(&ch, sizeof(char_t), 1);
							}
							else
								ostream.write(&ch, sizeof(char_t), 1);
						}
						else
							ostream.write(&ch, sizeof(char_t), 1);
						break;
					}
				}
			}
		}
		return ostream << '"';
	}
	else
	{
		ostream << '"';
		if(rhs)
			ostream << rhs.value();
		return ostream << '"';
	}
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, Array const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	ostream << '[';
	if(rhs)
	{
		auto const & elements = rhs.elements();
		if(elements.size() > 0)
		{
			bool single_line = false;
			if(elements.size() <= 7)
			{
				switch((*elements.begin()).index)
				{
					case Variant::null_i:
					case Variant::boolean_i:
					case Variant::integer_i:
					case Variant::number_i:
						single_line = true; 
						break;
					default:
						break;
				}
			}
			PrintSettings ps_ { ps.depth + 1, ps.readable, !single_line };
			for(auto it = elements.begin(); it; ++it)
			{
				if(ps.readable && !single_line)
					print_newline(ostream);
				print(ostream, *it.ptr(), ps_);
				if(it != elements.rbegin())
					ostream << ',';
			}
			if(ps.readable && !single_line)
			{
				print_newline(ostream);
				print_indent(ostream, ps.depth);
			}
		}
	}
	return ostream << ']'; 
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, object_t<variant_t>::element_t const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	ostream << '"' << rhs.key() << '"' << ':';
	if(ps.readable)
		ostream << ' ';
	return print(ostream, rhs.value(), { ps.depth, ps.readable, false });
}

static inline axl::stream::Output &
print(axl::stream::Output & ostream, Object const & rhs, PrintSettings const & ps)
{
	if(ps.readable && ps._indent)
		print_indent(ostream, ps.depth);
	ostream << '{';
	if(rhs)
	{
		auto const & entries = rhs.entries();
		if(entries.size() > 0)
		{
			if(ps.readable)
				print_newline(ostream);
			PrintSettings ps_ = { ps.depth + 1, ps.readable, true };
			for(auto it = entries.begin(); it; ++it)
			{
				print(ostream, *it.ptr(), ps_);
				if(it != entries.rbegin())
				{
					ostream << ',';
					if(ps.readable)
						print_newline(ostream);
				}
			}
			if(ps.readable)
			{
				print_newline(ostream);
				print_indent(ostream, ps.depth);
			}
		}
	}
	return ostream << '}'; 
}

static axl::stream::Output &
print(axl::stream::Output & ostream, Variant const & rhs, PrintSettings const & ps)
{
	switch(rhs.index)
	{
		case rhs.null_i:    return print(ostream, rhs.null, ps);
		case rhs.boolean_i: return print(ostream, rhs.boolean, ps);
		case rhs.integer_i: return print(ostream, rhs.integer, ps);
		case rhs.number_i:  return print(ostream, rhs.number, ps);
		case rhs.string_i:  return print(ostream, rhs.string, ps);
		case rhs.array_i:   return print(ostream, rhs.array, ps);
		case rhs.object_i:  return print(ostream, rhs.object, ps);
		default: break;
	}
	return ostream;
}


static inline char_t parse(axl::stream::Input & istream, Null & rhs, char_t ch_ = char_t());
static inline char_t parse(axl::stream::Input & istream, Boolean & rhs, char_t ch_ = char_t());
static inline char_t parse(axl::stream::Input & istream, Integer & rhs, char_t ch_ = char_t());
static inline char_t parse(axl::stream::Input & istream, Number & rhs, char_t ch_ = char_t());
static inline char_t parse(axl::stream::Input & istream, String & rhs, char_t ch_ = char_t());
static inline char_t parse(axl::stream::Input & istream, Array & rhs, char_t ch_ = char_t());
static        char_t parse(axl::stream::Input & istream, Object & rhs, char_t ch_ = char_t());
static        char_t parse(axl::stream::Input & istream, Variant & rhs, char_t ch_ = char_t());
static        char_t parse(axl::stream::Input & istream, object_t<variant_t>::element_t & rhs, char_t ch_ = char_t());
static        char_t parse_numeric(axl::stream::Input & istream, Variant & rhs, char_t ch_ = char_t());

static inline char_t
_skip_spaces(axl::stream::Input & istream)
{
	char_t ch;
	while((1 == istream.read(&ch, sizeof(char_t), 1)) && axl::is_char_white_space(ch))
	{
		axl_throw_if(istream.is_at_end(), axl::end_of_stream_exception("json::_skip_spaces"));
		axl_throw_alt_if(istream.is_at_end(), return ch);
	}
	return ch;
}

static inline char_t
_check_and_skip_spaces(axl::stream::Input & istream, char_t ch_)
{
	if(ch_ != char_t() && !axl::is_char_white_space(ch_))
		return ch_;
	char_t ch = ch_;
	while((1 == istream.read(&ch, sizeof(char_t), 1)) && axl::is_char_white_space(ch))
	{
		axl_throw_if(istream.is_at_end(), axl::end_of_stream_exception("json::_skip_spaces"));
		axl_throw_alt_if(istream.is_at_end(), return ch);
	}
	return ch;
}

static char_t
parse(axl::stream::Input & istream, Variant & rhs, char_t ch_)
{
	char_t ch = _check_and_skip_spaces(istream, ch_);
	switch(ch)
	{
		case 'n':
		{
			Null null_;
			ch = parse(istream, null_, ch);
			rhs = axl::move(null_);
			break;
		}
		case 't':
		case 'f':
		{
			Boolean boolean_;
			ch = parse(istream, boolean_, ch);
			rhs = axl::move(boolean_);
			break;
		}
		case '-':
		case '+':
		case '.':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			ch = parse_numeric(istream, rhs, ch);
			break;
		}
		case '"':
		{
			String string_;
			ch = parse(istream, string_, ch);
			rhs = axl::move(string_);
			break;
		}
		case '[':
		{
			Array array_;
			ch = parse(istream, array_, ch);
			rhs = axl::move(array_);
			break;
		}
		case '{':
		{
			Object object_;
			ch = parse(istream, object_, ch);
			rhs = axl::move(object_);
			break;
		}
		default:
			axl_throw(axl::runtime_error_exception("json::parse(Variant): invalid token"));
	}
	return ch;
}

static inline char_t 
parse(axl::stream::Input & istream, Null & rhs, char_t ch_)
{
	char_t ch = _check_and_skip_spaces(istream, ch_);
	size_t read_ = 1;
	if(ch == 'n'
		&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'u')
		&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'l')
		&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'l')
	)
		return ch;
	if(read_ != 1)
		axl_throw(axl::end_of_stream_exception("json::parse(Null): end of stream"));
	axl_throw(axl::runtime_error_exception("json::parse(Null): expecting `null`"));
	return ch;
}

static inline char_t 
parse(axl::stream::Input & istream, Boolean & rhs, char_t ch_)
{
	char_t ch = _check_and_skip_spaces(istream, ch_);
	size_t read_ = 1;
	if((ch == 't' || ch  == 'f'))
	{
		bool expecting_true_ = ch == 't';
		if(expecting_true_)
		{
			if(
				(1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'r')
				&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'u')
				&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'e')
			)
			{
				rhs = true;
				return ch;
			}
		}
		else
		{
			if(
				(1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'a')
				&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'l')
				&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 's')
				&& (1 == (read_ = istream.read(&ch, sizeof(char_t), 1)) && ch == 'e')
			)
			{
				rhs = false;
				return ch;
			}
		}
	}
	if(read_ != 1)
		axl_throw(axl::end_of_stream_exception("json::parse(Boolean): end of stream"));
	axl_throw(axl::runtime_error_exception("json::parse(Boolean): expecting `true|false`"));
	return ch;
}


static char_t 
parse_numeric(axl::stream::Input & istream, Variant & rhs, char_t ch_)
{
	char_t ch = _check_and_skip_spaces(istream, ch_);
	size_t read_ = 1;
	constexpr size_t max_buffer_size_ = 64; 
	bool signed_          = false;
	bool point_           = false;
	bool just_read_point_ = false;
	switch(ch)
	{
		case '.':
			point_  = true;
			break;
		case '-':
		case '+':
			signed_ = true;
			goto read_digits;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			read_digits:
			char_t buffer[max_buffer_size_+1] {};
			bool exponent_           = false;
			bool just_read_exponent_ = false;
			size_t i          = 0;
			size_t point_i    = 0; 
			size_t exponent_i = 0; 
			buffer[i++] = ch;
			for(read_ = istream.read(&ch, sizeof(char_t), 1); read_ == 1 && i < max_buffer_size_; read_ = istream.read(&ch, sizeof(char_t), 1))
			{
				switch(ch)
				{
					case '.':
						if(point_)
						{
							axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): multiple decimal point tokens"));
							goto end_loop;
						}
						else if(exponent_)
						{
							axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): decimal point token after exponent"));
							goto end_loop;
						}
						else
						{
							point_  = true;
							point_i = i;
							just_read_point_ = true;
						}
						goto record_and_continue;
					case '-':
					case '+':
						if(exponent_)
						{
							if(!just_read_exponent_)
							{
								axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): unexpected sign token"));
								goto end_loop;
							}
						}
						else 
						{
							axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): unexpected sign token"));
							goto end_loop;
						}
						goto record_and_continue;
					case 'e':
					case 'E':
						if(exponent_)
						{
							axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): multiple exponent tokens"));
							goto end_loop;
						}
						else if(just_read_point_)
						{
							axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): exponent token after decimal point token"));
							goto end_loop;
						}
						else
						{
							exponent_           = true;
							just_read_exponent_ = true;
							exponent_i = i;
						}
						goto record_and_continue;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					{
						record_and_continue:
						buffer[i++] = ch;
						if(just_read_point_ && ch != '.')
							just_read_point_ = false;
						if(just_read_exponent_ && ch != 'e')
							just_read_exponent_ = false;
						continue;
					}
					case ',':
					case ']':
					case '}':
					case ' ':
					case '\n':
					case '\r':
					case '\t':
					case '\v':
					case '\f':
						goto end_loop;
					default:
						axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): invalid numeric token"));
						goto end_loop;
				}
			}
			end_loop:
			if(
				   (i == 1 && (signed_ || point_)) // "0" || "."
				|| (point_ && point_i == (i - 1)) // "[+\-0-9]."
			)
			{
				read_ = 1;
				break;
			}
			buffer[i]  = '\0';
			if(point_ || exponent_)
			{
				if(exponent_)
				{
					auto exp_val = atoi(&buffer[exponent_i+1]);
					if(exp_val >= 999)
					{
						if(signed_)
							rhs = number_t(buffer[0] == '-' ? -inf : inf);
						else 
							rhs = number_t(inf);
					}
					else
						rhs = number_t(atof(buffer));
				}
				else
					rhs = number_t(atof(buffer));
			}
			else
				rhs = integer_t(atoll(buffer));
			return ch;
		}
		default: 
			break;
	}
	if(read_ != 1)
		axl_throw(axl::end_of_stream_exception("json::parse_numeric(Variant): end of stream"));
	axl_throw(axl::runtime_error_exception("json::parse_numeric(Variant): invalid numeric token"));
	return ch;
}

static inline char_t 
parse(axl::stream::Input & istream, String & rhs, char_t ch_)
{
	char_t ch = _check_and_skip_spaces(istream, ch_);
	if(ch != '"')
		axl_throw(axl::runtime_error_exception("json::parse(String): `\"` expected at the start of string"));
	size_t read_ = 1;
	size_t i     = 0;
	constexpr size_t buffer_size_ = 1024;
	char_t           buffer[buffer_size_] {};
	String jstr;
	auto flush_buffer = [&]()
	{
		size_t length_ = 0;
		if(jstr)
			length_ = jstr.value().length();
		String jstr_(length_ + i);
		auto & string = jstr_.value();
		if(length_ > 0)
			string.copy(jstr.value(), 0, length_);
		string.copy(string_view_t(&buffer[0], &buffer[i]), length_, i);
		jstr = axl::move(jstr_);
		i = 0;
	};
	bool escape_ = false;
	while(1 == (read_ = istream.read(&ch, sizeof(char_t), 1)))
	{
		if(process_string)
		{
			switch(ch)
			{
				case '"':
					if(escape_)
					{
						escape_ = false;
						goto record_char;
					}
					goto end_loop;
				case '\\':
					if(!escape_)
					{
						escape_ = true;
						continue;
					}
				case '/':
				case 'b':
				case 'f':
				case 'n':
				case 'r':
				case 't':
				case 'u':
				case 'U':
					if(escape_)
					{
						escape_ = false;
						if(ch == 'u')
						{
							if(decode_utf8)
							{
								constexpr size_t N = 4;
								char_t ucode[N] {};
								if(N != (read_ = istream.read(&ucode[0], sizeof(char_t), N)))
									goto end_loop;
								read_ = 1;
								uint16_t unicode_ = 
									  uint16_t(hex_value(ucode[0])) << 12
									| uint16_t(hex_value(ucode[1])) << 8
									| uint16_t(hex_value(ucode[2])) << 4
									| uint16_t(hex_value(ucode[3]));
								if(unicode_ <= 0x007F)
								{
									if(i >= buffer_size_)
										flush_buffer();
									buffer[i++] = char_t(unicode_ & 0x7F);
								}
								else if(unicode_ <= 0x007FF)
								{
									if(i >= buffer_size_ - 2)
										flush_buffer();
									buffer[i++] = char_t(0xC0 | ((unicode_ >> 6) & 0x1F));
									buffer[i++] = char_t(0x80 | (unicode_ & 0x3F));
								}
								else if(unicode_ <= 0xFFFF)
								{
									if(i >= buffer_size_ - 3)
										flush_buffer();
									buffer[i++] = char_t(0xE0 | ((unicode_ >> 12) & 0x0F));
									buffer[i++] = char_t(0x80 | ((unicode_ >> 6) & 0x3F));
									buffer[i++] = char_t(0x80 | (unicode_ & 0x3F));
								}
								else
								{
									if(i >= buffer_size_ - 6)
										flush_buffer();
									buffer[i++] = '\\';
									buffer[i++] = 'u';
									buffer[i++] = ucode[0];
									buffer[i++] = ucode[1];
									buffer[i++] = ucode[2];
									buffer[i++] = ucode[3];
								}
							}
							else
							{
								if(i >= buffer_size_)
									flush_buffer();
								buffer[i++] = '\\';
								buffer[i++] = ch;
							}
						}
						else if(ch == 'U')
						{
							if(decode_utf8_ext)
							{
								constexpr size_t N = 6;
								char_t ucode[N] {};
								if(N != (read_ = istream.read(&ucode[0], sizeof(char_t), N)))
									goto end_loop;
								read_ = 1;
								uint32_t unicode_ = 
									  uint32_t(hex_value(ucode[0])) << 18
									| uint32_t(hex_value(ucode[1])) << 16
									| uint32_t(hex_value(ucode[2])) << 12
									| uint32_t(hex_value(ucode[3])) << 8
									| uint32_t(hex_value(ucode[4])) << 4
									| uint32_t(hex_value(ucode[5]));
								if(unicode_ <= 0x007F)
								{
									if(i >= buffer_size_)
										flush_buffer();
									buffer[i++] = char_t(unicode_ & 0x7F);
								}
								else if(unicode_ <= 0x007FF)
								{
									if(i >= buffer_size_ - 2)
										flush_buffer();
									buffer[i++] = char_t(0xC0 | ((unicode_ >> 6) & 0x1F));
									buffer[i++] = char_t(0x80 | (unicode_ & 0x3F));
								}
								else if(unicode_ <= 0xFFFF)
								{
									if(i >= buffer_size_ - 3)
										flush_buffer();
									buffer[i++] = char_t(0xE0 | ((unicode_ >> 12) & 0x0F));
									buffer[i++] = char_t(0x80 | ((unicode_ >> 6) & 0x3F));
									buffer[i++] = char_t(0x80 | (unicode_ & 0x3F));
								}
								else if(unicode_ <= 0x10FFFF)
								{
									if(i >= buffer_size_ - 4)
										flush_buffer();
									buffer[i++] = char_t(0xF0 | ((unicode_ >> 18) & 0x07));
									buffer[i++] = char_t(0x80 | ((unicode_ >> 12) & 0x3F));
									buffer[i++] = char_t(0x80 | ((unicode_ >> 6) & 0x3F));
									buffer[i++] = char_t(0x80 | (unicode_ & 0x3F));
								}
								else
								{
									if(i >= buffer_size_ - 6)
										flush_buffer();
									buffer[i++] = '\\';
									buffer[i++] = 'U';
									buffer[i++] = ucode[0];
									buffer[i++] = ucode[1];
									buffer[i++] = ucode[2];
									buffer[i++] = ucode[3];
									buffer[i++] = ucode[4];
									buffer[i++] = ucode[5];
								}
							}
							else
							{
								if(i >= buffer_size_)
									flush_buffer();
								buffer[i++] = '\\';
								buffer[i++] = ch;
							}
						}
						else
						{
							ch = [](char_t ech) 
							{
								switch(ech)
								{
									case '\\': return '\\';
									case '/':  return '/';
									case 'b':  return '\b';
									case 'f':  return '\f';
									case 'n':  return '\n';
									case 'r':  return '\r';
									case 't':  return '\t';
									default: return ech;
								}
							}(ch);
							goto record_char;
						}
						continue;
					}
					goto record_char;
				default:
					record_char:
					if(escape_)
						axl_throw(axl::runtime_error_exception("json::parse(String): invalid escaped string token"));
					if(i >= buffer_size_)
						flush_buffer();
					buffer[i++] = ch;
					continue;
			}
		}
		else
		{
			switch(ch)
			{
				case '"':
					if(escape_)
					{
						escape_ = false;
						goto record_char_;
					}
					goto end_loop;
				case '\\':
					escape_ = !escape_;
				default:
					record_char_:
					if(i >= buffer_size_)
						flush_buffer();
					buffer[i++] = ch;
					continue;
			}
		}
	}
	end_loop:
	if(read_ != 1)
		axl_throw(axl::runtime_error_exception("json::parse(String): end of stream"));
	if(ch != '"')
		axl_throw(axl::runtime_error_exception("json::parse(String): `\"` expected at the end of string"));
	flush_buffer();
	rhs = axl::move(jstr);
	return ch;
}

static inline char_t 
parse(axl::stream::Input & istream, Array & rhs, char_t ch_)
{
	char_t ch = _check_and_skip_spaces(istream, ch_);
	if(ch != '[')
		axl_throw(axl::runtime_error_exception("json::parse(Array): `[` expected at the start of array"));
	ch = _skip_spaces(istream);
	if(ch == ']')
		rhs = {};
	else
	{
		bool first_time = true;
		do
		{
			if(ch == ',')
				ch = _skip_spaces(istream);
			else
				ch = _check_and_skip_spaces(istream, ch);
			Variant element_;
			ch = parse(istream, element_, ch);
			if(element_)
			{
				if(first_time)
				{
					rhs = {{ axl::move(element_) }};
					first_time = false;
				}
				else
					rhs.insert(axl::move(element_));
			}
			switch(element_.index)
			{
				case Variant::string_i:
				case Variant::array_i:
				case Variant::object_i:
				case Variant::null_i:
				case Variant::boolean_i:
					ch = _skip_spaces(istream);
					break;
				default: 
					ch = _check_and_skip_spaces(istream, ch);
					break;
			}
		}
		while(ch == ',');
	}
	if(ch != ']')
		axl_throw(axl::runtime_error_exception("json::parse(Array): `}` expected at the end of array"));
	return ch;
}

static inline char_t 
parse(axl::stream::Input & istream, Object & rhs, char_t ch_)
{
	char_t ch = _check_and_skip_spaces(istream, ch_);
	if(ch != '{')
		axl_throw(axl::runtime_error_exception("json::parse(Object): `{` expected at the start of object"));
	ch = _skip_spaces(istream);
	if(ch == '}')
		rhs = {};
	else
	{
		bool first_time = true;
		do
		{
			if(ch == ',')
				ch = _skip_spaces(istream);
			else
				ch = _check_and_skip_spaces(istream, ch);
			String  key_;
			ch = parse(istream, key_, ch);
			ch = _skip_spaces(istream);
			if(ch != ':')
				axl_throw(axl::runtime_error_exception("json::parse(Object): `:` expected after key, in object"));
			ch = _skip_spaces(istream);
			Variant value_;
			ch = parse(istream, value_, ch);
			if(value_)
			{
				if(first_time)
				{
					rhs = Object({ Entry{ axl::move(key_.value()), axl::move(value_) } });
					first_time = false;
				}
				else
					rhs.set(axl::move(key_), axl::move(value_));
			}
			switch(value_.index)
			{
				case Variant::string_i:
				case Variant::array_i:
				case Variant::object_i:
				case Variant::null_i:
				case Variant::boolean_i:
					ch = _skip_spaces(istream);
					break;
				default: 
					ch = _check_and_skip_spaces(istream, ch);
					break;
			}
		}
		while(ch == ',');
	}
	if(ch != '}')
		axl_throw(axl::runtime_error_exception("json::parse(Object): `}` expected at the end of object"));
	return ch;
}


static constexpr char_t
hex_char(uint8_t value) 
{
	if(value <= 9)
		return '0' + value;
	else if(value <= 15)
		return 'A' + value - 10;
	else
		return '0';
}

static constexpr uint8_t  
hex_value(char_t hex_char) 
{
	if(hex_char >= '0' && hex_char <= '9')
		return uint8_t(hex_char - '0');
	else if(hex_char >= 'a' && hex_char <= 'f')
		return 10 + uint8_t(hex_char - 'a');
	else if(hex_char >= 'A' && hex_char <= 'F')
		return 10 + uint8_t(hex_char - 'A');
	else
		return 0;
}

} // namespace json

namespace stream {

static inline Output &
operator<<(Output & ostream, axl::json::Null const & rhs)
{ return axl::json::print(ostream, rhs); }

static inline Output & 
operator<<(Output & ostream, axl::json::Boolean const & rhs) 
{ return axl::json::print(ostream, rhs); }

static inline Output &
operator<<(Output & ostream, axl::json::Integer const & rhs)
{ return axl::json::print(ostream, rhs); }

static inline Output &
operator<<(Output & ostream, axl::json::Number const & rhs)
{ return axl::json::print(ostream, rhs); }

static inline Output &
operator<<(Output & ostream, axl::json::String const & rhs)
{ return axl::json::print(ostream, rhs); }

static inline Output &
operator<<(Output & ostream, axl::json::Array const & rhs)
{ return axl::json::print(ostream, rhs); }

static inline Output &
operator<<(Output & ostream, axl::json::object_t<axl::json::variant_t>::element_t const & rhs)
{ return axl::json::print(ostream, rhs); }

static inline Output &
operator<<(Output & ostream, axl::json::Object const & rhs)
{ return axl::json::print(ostream, rhs); }

static inline Output &
operator<<(Output & ostream, axl::json::Variant const & rhs)
{ return axl::json::print(ostream, rhs); }


static inline Input & 
operator>>(axl::stream::Input & istream, axl::json::Null & rhs)
{
	axl::json::parse(istream, rhs);
	return istream;
}

static inline Input & 
operator>>(axl::stream::Input & istream, axl::json::Boolean & rhs)
{
	axl::json::parse(istream, rhs);
	return istream;
}

static Input & 
operator>>(axl::stream::Input & istream, axl::json::Integer & rhs) axl_except
{
	axl::json::Variant var;
	axl::json::parse_numeric(istream, var);
	axl_throw_if(var.index != axl::json::Variant::integer_i, axl::runtime_error_exception("operator>>(json::Integer): value is not integral"));
	rhs = var.integer;
	return istream;
}

static Input & 
operator>>(axl::stream::Input & istream, axl::json::Number & rhs) axl_except
{
	axl::json::Variant var;
	axl::json::parse_numeric(istream, var);
	axl_throw_if(var.index != axl::json::Variant::number_i, axl::runtime_error_exception("operator>>(json::Number): value is not numeric"));
	rhs = var.number;
	return istream;
}

static inline Input & 
operator>>(axl::stream::Input & istream, axl::json::String & rhs)
{
	axl::json::parse(istream, rhs);
	return istream;
}

static inline Input & 
operator>>(axl::stream::Input & istream, axl::json::Array & rhs)
{
	axl::json::parse(istream, rhs);
	return istream;
}

static inline Input & 
operator>>(axl::stream::Input & istream, axl::json::Object & rhs)
{
	axl::json::parse(istream, rhs);
	return istream;
}

static inline Input & 
operator>>(axl::stream::Input & istream, axl::json::Variant & rhs)
{
	axl::json::parse(istream, rhs);
	return istream;
}


} // namespace stream

static inline hash_t 
hash(axl::json::string_t const & string_)
{
	return axl::hash_string(string_.begin());
}

static inline hash_t 
hash(axl::json::string_view_t const & string_)
{
	return axl::hash_string(string_.begin());
}

static hash_t 
hash(axl::json::Variant const & var)
{
	constexpr hash_t partition_size_      = (axl::json::hash_table_size_ - 4) / 4;
	constexpr hash_t half_partition_size_ = partition_size_ / 2;
	switch(var.index)
	{
		case var.null_i:    return 1;
		case var.boolean_i: return (var.boolean.value() + 2);
		case var.integer_i: return ((axl::hash(var.integer.value()) % partition_size_) + 4);
		case var.number_i:  return ((axl::hash(*reinterpret_cast<uint64_t const *>(&var.number.value())) % partition_size_) + partition_size_);
		case var.string_i:  return (((var.string ? axl::hash(var.string.value()) : 0) % partition_size_) + partition_size_ * 2);
		case var.array_i:   return ((axl::hash(&var) % half_partition_size_) + partition_size_ * 3);
		case var.object_i:  return ((axl::hash(&var) % half_partition_size_) + partition_size_ * 3 + half_partition_size_);
		case var.invalid_i:
		default: return 0;
	}
}

} // namespace axl

#endif // AXL_RESOURCE_JSON_HPP
