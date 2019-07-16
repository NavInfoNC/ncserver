/*
MIT License

Copyright (c) 2019 GIS Core R&D Department, NavInfo Co., Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "stdafx.h"
#include "ncserver.h"

#include <stdint.h>

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

size_t SuperFastHash(const char * data, int len) {
	uint32_t hash = len, tmp;
	int rem;

	if (len <= 0 || data == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (; len > 0; len--) {
		hash += get16bits(data);
		tmp = (get16bits(data + 2) << 11) ^ hash;
		hash = (hash << 16) ^ tmp;
		data += 2 * sizeof(uint16_t);
		hash += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
	case 3: hash += get16bits(data);
		hash ^= hash << 16;
		hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
		hash += hash >> 11;
		break;
	case 2: hash += get16bits(data);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;
	case 1: hash += (signed char)*data;
		hash ^= hash << 10;
		hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

#include <unordered_map>

struct CStringHash
{
	size_t operator()(const char* pstr) const
	{
		return SuperFastHash(pstr, (int)strlen(pstr));
	};
};

struct CStringEqual
{
	bool operator()(const char* l, const char* r) const
	{
		return strcmp(l, r) == 0;
	};
};

typedef std::unordered_map < const char*, const char*, CStringHash, CStringEqual> Map;

struct StaticStringMapImple : public StaticStringMap
{
	Map m_map;
};

StaticStringMap* StaticStringMap_alloc() {
	return new StaticStringMapImple();
}

void StaticStringMap_free(StaticStringMap* o_) {
	StaticStringMapImple* o = (StaticStringMapImple*)o_;
	delete o;
}

void StaticStringMap::set(const char* name, const char* value) {
	StaticStringMapImple* o = (StaticStringMapImple*)this;
	o->m_map[name] = value;
}

const char* StaticStringMap::get(const char* name) {
	StaticStringMapImple* o = (StaticStringMapImple*)this;
	Map::const_iterator iter = o->m_map.find(name);
	if (iter != o->m_map.end())
		return iter->second;
	else
		return NULL;
}

void StaticStringMap::clear() {
	StaticStringMapImple* o = (StaticStringMapImple*)this;
	o->m_map.clear();
}

//////////////////////////////////////////////////////////////////////////

struct StaticStringMapIteratorImple : public RequestParameteterIterator
{
	Map::const_iterator m_i, m_begin, m_end;
};

RequestParameteterIterator* RequestParameteterIterator_alloc()
{
	StaticStringMapIteratorImple* o = new StaticStringMapIteratorImple;
	return o;
}

void RequestParameteterIterator_free(RequestParameteterIterator* o)
{
	StaticStringMapIteratorImple* p = (StaticStringMapIteratorImple*)o;
	delete p;
}

bool RequestParameteterIterator::next()
{
	StaticStringMapIteratorImple* o = (StaticStringMapIteratorImple*)this;
	if (o->m_i == o->m_end)
		return false;

	o->name = o->m_i->first;
	o->value = o->m_i->second;
	o->m_i++;
	return true;
}

void RequestParameteterIterator::_init(StaticStringMap* map_)
{
	StaticStringMapIteratorImple* o = (StaticStringMapIteratorImple*)this;
	StaticStringMapImple* map = (StaticStringMapImple*)map_;
	o->m_i = o->m_begin = map->m_map.begin();
	o->m_end = map->m_map.end();
}

void RequestParameteterIterator::reset()
{
	StaticStringMapIteratorImple* o = (StaticStringMapIteratorImple*)this;
	o->m_i = o->m_begin;
}