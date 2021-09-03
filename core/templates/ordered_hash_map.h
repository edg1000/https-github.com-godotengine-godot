/*************************************************************************/
/*  ordered_hash_map.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef ORDERED_HASH_MAP_H
#define ORDERED_HASH_MAP_H

#include "core/templates/hash_map.h"
#include "core/templates/list.h"
#include "core/templates/pair.h"

/**
 * A hash map which allows to iterate elements in insertion order.
 * Insertion, lookup, deletion have O(1) complexity.
 * The API aims to be consistent with Map rather than HashMap, because the
 * former is more frequently used and is more coherent with the rest of the
 * codebase.
 * Deletion during iteration is safe and will preserve the order.
 */
template <class K, class V, class Hasher = HashMapHasherDefault, class Comparator = HashMapComparatorDefault<K>, uint8_t MIN_HASH_TABLE_POWER = 3, uint8_t RELATIONSHIP = 8>
class OrderedHashMap {
	typedef HashMap<K, V, Hasher, Comparator, MIN_HASH_TABLE_POWER, RELATIONSHIP> InternalMap;
	typedef List<typename InternalMap::Element *> InternalList;

	InternalList list;
	InternalMap map;

public:
	class Element {
		friend class OrderedHashMap<K, V, Hasher, Comparator, MIN_HASH_TABLE_POWER, RELATIONSHIP>;

		typename InternalList::Element *list_element = nullptr;

		Element(typename InternalList::Element *p_element) :
				list_element(p_element) {}

	public:
		_FORCE_INLINE_ Element() {}

		Element(const Element &other) :
				list_element(other.list_element) {}

		Element next() const {
			return Element(list_element ? list_element->next() : nullptr);
		}

		Element prev() const {
			return Element(list_element ? list_element->prev() : nullptr);
		}

		Element &operator=(const Element &other) {
			list_element = other.list_element;
			return *this;
		}

		_FORCE_INLINE_ bool operator==(const Element &p_other) const {
			return this->list_element == p_other.list_element;
		}
		_FORCE_INLINE_ bool operator!=(const Element &p_other) const {
			return this->list_element != p_other.list_element;
		}

		operator bool() const {
			return (list_element != nullptr);
		}

		KeyValue<K, V> &key_value() const {
			CRASH_COND(!list_element);
			return list_element->get()->key_value();
		}

		const K &key() const {
			CRASH_COND(!list_element);
			return list_element->get()->key();
		}

		V &value() {
			CRASH_COND(!list_element);
			return list_element->get()->value();
		}

		const V &value() const {
			CRASH_COND(!list_element);
			return list_element->get()->value();
		}

		V &get() {
			CRASH_COND(!list_element);
			return list_element->get()->value();
		}

		const V &get() const {
			CRASH_COND(!list_element);
			return list_element->get()->value();
		}
	};

	class ConstElement {
		friend class OrderedHashMap<K, V, Hasher, Comparator, MIN_HASH_TABLE_POWER, RELATIONSHIP>;

		const typename InternalList::Element *list_element = nullptr;

		ConstElement(const typename InternalList::Element *p_element) :
				list_element(p_element) {}

	public:
		_FORCE_INLINE_ ConstElement() {}

		ConstElement(const ConstElement &other) :
				list_element(other.list_element) {
		}

		ConstElement &operator=(const ConstElement &other) {
			list_element = other.list_element;
			return *this;
		}

		ConstElement next() const {
			return ConstElement(list_element ? list_element->next() : nullptr);
		}

		ConstElement prev() const {
			return ConstElement(list_element ? list_element->prev() : nullptr);
		}

		_FORCE_INLINE_ bool operator==(const ConstElement &p_other) const {
			return this->list_element == p_other.list_element;
		}
		_FORCE_INLINE_ bool operator!=(const ConstElement &p_other) const {
			return this->list_element != p_other.list_element;
		}

		operator bool() const {
			return (list_element != nullptr);
		}

		const K &key() const {
			CRASH_COND(!list_element);
			return list_element->get()->key();
		}

		const V &value() const {
			CRASH_COND(!list_element);
			return list_element->get()->value();
		}

		const V &get() const {
			CRASH_COND(!list_element);
			return list_element->get()->value();
		}
	};

	typedef KeyValue<K, V> ValueType;

	struct Iterator {
		_FORCE_INLINE_ KeyValue<K, V> &operator*() const {
			return E.key_value();
		}
		_FORCE_INLINE_ KeyValue<K, V> *operator->() const { return &E.key_value(); }
		_FORCE_INLINE_ Iterator &operator++() {
			E = E.next();
			return *this;
		}
		_FORCE_INLINE_ Iterator &operator--() {
			E = E.prev();
			return *this;
		}

		_FORCE_INLINE_ bool operator==(const Iterator &b) const { return E == b.E; }
		_FORCE_INLINE_ bool operator!=(const Iterator &b) const { return E != b.E; }

		Iterator(Element p_E) { E = p_E; }
		Iterator() {}
		Iterator(const Iterator &p_it) { E = p_it.E; }

	private:
		Element E = Element(nullptr);
	};

	struct ConstIterator {
		_FORCE_INLINE_ const KeyValue<K, V> &operator*() const {
			return E.key_value();
		}
		_FORCE_INLINE_ const KeyValue<K, V> *operator->() const { return &E.key_value(); }
		_FORCE_INLINE_ ConstIterator &operator++() {
			E = E.next();
			return *this;
		}
		_FORCE_INLINE_ ConstIterator &operator--() {
			E = E.prev();
			return *this;
		}

		_FORCE_INLINE_ bool operator==(const ConstIterator &b) const { return E == b.E; }
		_FORCE_INLINE_ bool operator!=(const ConstIterator &b) const { return E != b.E; }

		ConstIterator(const Element p_E) { E = p_E; }
		ConstIterator() {}
		ConstIterator(const ConstIterator &p_it) { E = p_it.E; }

	private:
		const Element E = Element(nullptr);
	};

	_FORCE_INLINE_ Iterator begin() {
		return Iterator(front());
	}
	_FORCE_INLINE_ Iterator end() {
		return Iterator(nullptr);
	}

#if 0
	//to use when replacing find()
	_FORCE_INLINE_ Iterator find(const K &p_key) {
		return Iterator(find(p_key));
	}
#endif
	_FORCE_INLINE_ void remove(const Iterator &p_iter) {
		return erase(p_iter.E);
	}

	_FORCE_INLINE_ ConstIterator begin() const {
		return ConstIterator(front());
	}
	_FORCE_INLINE_ ConstIterator end() const {
		return ConstIterator(nullptr);
	}

#if 0
	//to use when replacing find()
	_FORCE_INLINE_ ConstIterator find(const K &p_key) const {
		return ConstIterator(find(p_key));
	}
#endif

	ConstElement find(const K &p_key) const {
		if (map.has(p_key)) {
			const typename InternalList::Element *it = list.front();
			while (it) {
				if (it->get()->key() == p_key) {
					return ConstElement(it);
				}
				it = it->next();
			}
		}
		return ConstElement(nullptr);
	}

	Element find(const K &p_key) {
		if (map.has(p_key)) {
			typename InternalList::Element *it = list.front();
			while (it) {
				if (it->get()->key() == p_key) {
					return Element(it);
				}
				it = it->next();
			}
		}
		return Element(nullptr);
	}

	Element insert(const K &p_key, const V &p_value) {
		if (!map.has(p_key)) {
			typename InternalMap::Element *new_element = map.set(p_key, p_value);
			typename InternalList::Element *list_element = list.push_back(new_element);
			return Element(list_element);
		}

		map.set(p_key, p_value);
		return find(p_key);
	}

	void erase(Element &p_element) {
		map.erase(p_element.key());
		list.erase(p_element.list_element);
		p_element.list_element = nullptr;
	}

	bool erase(const K &p_key) {
		const Element element = find(p_key);
		if (element.list_element) {
			list.erase(element.list_element);
			map.erase(p_key);
			return true;
		}
		return false;
	}

	inline bool has(const K &p_key) const {
		return map.has(p_key);
	}

	const V &operator[](const K &p_key) const {
		ConstElement e = find(p_key);
		CRASH_COND(!e);
		return e.value();
	}

	V &operator[](const K &p_key) {
		Element e = find(p_key);
		if (!e) {
			// consistent with Map behaviour
			e = insert(p_key, V());
		}
		return e.value();
	}

	inline Element front() {
		return Element(list.front());
	}

	inline Element back() {
		return Element(list.back());
	}

	inline ConstElement front() const {
		return ConstElement(list.front());
	}

	inline ConstElement back() const {
		return ConstElement(list.back());
	}

	inline bool is_empty() const { return list.is_empty(); }
	inline int size() const { return list.size(); }

	const void *id() const {
		return list.id();
	}

	void clear() {
		map.clear();
		list.clear();
	}

private:
	void _copy_from(const OrderedHashMap &p_map) {
		for (ConstElement E = p_map.front(); E; E = E.next()) {
			insert(E.key(), E.value());
		}
	}

public:
	void operator=(const OrderedHashMap &p_map) {
		_copy_from(p_map);
	}

	OrderedHashMap(const OrderedHashMap &p_map) {
		_copy_from(p_map);
	}

	_FORCE_INLINE_ OrderedHashMap() {}
};

#endif // ORDERED_HASH_MAP_H
