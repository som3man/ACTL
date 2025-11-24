#pragma once

#include "../Allocation.hpp"

#include <initializer_list>

namespace ACTL {
	// One-buffer resizable container template.
	template <typename Type>
	class Array {
	public:
		// Copies contents from buffer.
		static Array<Type> Copy(const Type* from, const Type* to) {
			Array<Type> NewArray = 0;

			const size length = to - from;

			NewArray.first = (Type*)alloc(length * sizeof(Type));

			NewArray.last = NewArray.first + length;

			NewArray.ending = NewArray.last;

			for (size i = 0; i < length; i++)
				new(NewArray.first + i) Type(from[i]);

			return NewArray;
		}

		// Moves contents from buffer.
		static Array<Type> Move(Type* from, Type* to) {
			static_assert(ACTL::hasMoveConstructor<Type>, "Type must be move-constructible!");

			Array<Type> NewArray = 0;

			const size length = to - from;

			NewArray.first = (Type*)alloc(length * sizeof(Type));

			NewArray.last = NewArray.first + length;

			NewArray.ending = NewArray.last;

			for (size i = 0; i < length; i++)
				new(NewArray.first + i) Type(ACTL::move(from[i]));

			return NewArray;
		}

		// Creates Array with given length and initialized objects.
		template <typename... Args>
		static Array<Type> Sized(size length, Args&&... args) {
			Array<Type> NewArray = 0;

			NewArray.first = (Type*)alloc(length * sizeof(Type));

			NewArray.last = NewArray.first + length;

			NewArray.ending = NewArray.last;

			for (Type* ptr = NewArray.first; ptr < NewArray.last; ptr++)
				new(ptr) Type(ACTL::forward<Args...>(args...));

			return NewArray;
		}

#ifdef ACTL_STD_INCLUDED_VECTOR
		// Construction from vector by copying each object.
		Array(const std::vector<Type>& vector) {
			operator=(vector);
		}

		// Construction from vector by moving each object.
		Array(std::vector<Type>&& vector) {
			operator=(ACTL::move(vector));
		}

		// Assigning from vector by copying each object.
		Array& operator =(const std::vector<Type>& vector) {
			Delete();

			const size length = vector.size();

			first = (Type*)alloc(vector.capacity() * sizeof(Type));

			last = first + length;

			ending = first + vector.capacity();

			for (size i = 0; i < length; i++)
				new(first + i) Type(vector[i]);

			return *this;
		}

		// Assigning from vector by moving each object.
		Array& operator =(std::vector<Type>&& vector) {
			static_assert(ACTL::hasMoveConstructor<Type>, "Type must be move-constructible!");

			Delete();

			const size length = vector.size();

			first = (Type*)alloc(vector.capacity() * sizeof(Type));

			last = first + length;

			ending = first + vector.capacity();

			for (size i = 0; i < length; i++)
				new(first + i) Type(ACTL::move(vector[i]));

			vector.resize(0);

			vector.shrink_to_fit();

			return *this;
		}

		// Creates vector with Array`s length and capacity and copies objects from this Array.
		operator std::vector<Type>() const {
			std::vector<Type> vector = {};

			vector.reserve(GetCapacity());

			const size length = GetLength();

			vector.resize(length);

			for (size i = 0; i < length; i++)
				vector[i] = first[i];

			return vector;
		}

		// Moves Array`s contents to a given vector.
		void MoveToVector(std::vector<Type>& vector) {
			vector.resize(0);

			for (size i = 0; i < GetLength(); i++)
				vector.emplace_back(ACTL::move(first[i]));

			Delete();
		}
#endif

		Array() noexcept {

		}

		// Inits with given capacity.
		Array(size capacity) {
			operator=(capacity);
		}

		// Inits with copied objects.
		Array(const Array& Another) {
			operator=(Another);
		}

		Array(Array&& Another) noexcept {
			operator=(ACTL::move(Another));
		}

		// Copies each object from list.
		Array(const std::initializer_list<Type>& list) noexcept {
			operator=(list);
		}

		~Array() noexcept {
			Delete();
		}

		Array& operator =(size capacity) {
			SetCapacity(capacity);

			return *this;
		}

		Array& operator =(const Array& Another) {
			Clear();

			SetCapacity(Another.GetCapacity());

			last = first + Another.GetLength();

			for (size i = 0; i < Another.GetLength(); i++)
				new(first + i) Type(Another.first[i]);

			return *this;
		}

		Array& operator =(Array&& Another) noexcept {
			Delete();

			first = Another.first;

			last = Another.last;

			ending = Another.ending;

			Another.first = nullptr;

			Another.last = nullptr;

			Another.ending = nullptr;

			return *this;
		}

		// Will copy each object from list.
		Array& operator =(const std::initializer_list<Type>& list) noexcept {
			Clear();

			SetCapacity(list.size());

			last = first + list.size();

			for (size i = 0; i < list.size(); i++)
				new(first + i) Type(list.begin()[i]);

			return *this;
		}

		// Returns true if Array has zero length.
		bool isEmpty() const noexcept {
			return first == last;
		}

		// Returns false if Array has zero length.
		operator bool() const noexcept {
			return first != last;
		}

		// Normalizes value to Array`s bounds. Returns nullindex if Array is empty.
		size ToRange(i64 index) const noexcept {
			if (isEmpty())
				return nullindex;

			const i64 length = (i64)GetLength();

			while (index < 0)
				index += length;

			while (index >= length)
				index -= length;

			return (size)index;
		}

		Type* begin() noexcept {
			return first;
		}

		Type* end() noexcept {
			return last;
		}

		const Type* begin() const noexcept {
			return first;
		}

		const Type* end() const noexcept {
			return last;
		}

		Type& operator [](size index) noexcept {
			return first[index];
		}

		const Type& operator [](size index) const noexcept {
			return first[index];
		}

		// Destructs all contents.
		void Clear() noexcept {
			if (!first)
				return;

			while (last != first) {
				last--;

				last->~Type();
			}
		}

		// Destructs and deallocates all contents.
		void Delete() {
			if (!first)
				return;

			while (last != first) {
				last--;

				last->~Type();
			}

			dealloc(first);

			first = nullptr;

			ending = nullptr;

			last = nullptr;
		}

		void SetCapacity(size newCapacity) {
			if (!newCapacity) {
				Delete();

				return; 
			}

			if (newCapacity == GetCapacity())
				return;

			Type* ptr = (Type*)alloc(newCapacity * sizeof(Type));

			if (first) {
				MoveData(ptr, newCapacity);

				dealloc(first);
			}
			else
				last = ptr;

			first = ptr;

			ending = first + newCapacity;
		}

		size GetCapacity() const {
			return ending - first;
		}

		size GetLength() const {
			return last - first;
		}

		// Returns the size of contents in bytes.
		size GetSize() const {
			return GetLength() * sizeof(Type);
		}

		void ShrinkToFit() {
			SetCapacity(GetLength());
		}

		// Destructs and constructs object.
		template <typename... Args>
		Type& Reinit(size index, Args&&... args) const {
			(first + index)->~Type();

			new(first + index) Type(ACTL::forward<Args...>(args...));
		}

		template <typename... Args>
		Type& EmplaceBack(Args&&... args) {
			if (last == ending)
				SetCapacity(ending ? GetCapacity() * 2 : 1);

			new(last) Type(ACTL::forward<Args...>(args...));

			last++;

			return last[-1];
		}

		void EraseBack() noexcept {
			last--;

			last->~Type();
		}

		template <typename... Args>
		Type& Emplace(size index, Args&&... args) {
			if (last <= ending + 1)
				SetCapacity(ending ? GetCapacity() * 2 : 2);

			for (size i = GetLength(); i != index; i--) {
				if constexpr (hasMoveConstructor<Type>)
					new (first + i) Type(ACTL::move(first[i - 1]));
				else if constexpr (hasCopyConstructor<Type>) {
					new (first + i) Type(first[i - 1]);

					first[i - 1].~Type();
				}
			}

			last++;

			new (first + index) Type(ACTL::forward<Args...>(args...));

			return first[index];
		}

		void Erase(size index) {
			(first + index)->~Type();

			last--;

			for (size i = index; i < GetLength(); i++) {
				if constexpr (hasMoveConstructor<Type>)
					new(first + i) Type(ACTL::move(first[i + 1]));
				else if constexpr (hasCopyConstructor<Type>) {
					new (first + i) Type(first[i + 1]);

					first[i + 1].~Type();
				}
			}
		}

		size GetIndex(const Type& value) const {
			for (int i = 0; i < GetLength(); i++)
				if (first[i] == value)
					return i;

			return nullindex;
		}

		bool Contains(const Type& Target) const noexcept {
			return (&Target >= first) && (&Target < last);
		}

	private:
		/*
		first - pointer to the first element.
		last - pointer to the next element after the last CONSTRUCTED one.
		ending - pointer to the next element after the last ALLOCATED one.
		*/
		Type* first = nullptr, * last = nullptr, * ending = nullptr;

		void MoveData(Type* ptr, size count) {
			if constexpr (hasMoveConstructor<Type>) {
				if (last >= first + count) {
					for (size i = 0; i < count; i++)
						new(ptr + i) Type(ACTL::move(first[i]));

					last = ptr + count;
				}
				else {
					const size length = GetLength();

					for (size i = 0; i < length; i++)
						new(ptr + i) Type(ACTL::move(first[i]));

					last = ptr + length;
				}
			}
			else if constexpr (hasCopyConstructor<Type>) {
				if (last >= first + count) {
					for (size i = 0; i < count; i++) {
						new(ptr + i) Type(first[i]);

						first[i].~Type();
					}

					last = ptr + count;
				}
				else {
					const size length = GetLength();

					for (size i = 0; i < length; i++) {
						new(ptr + i) Type(first[i]);

						first[i].~Type();
					}

					last = ptr + length;
				}
			}
		}
	};

#ifdef ACTL_STD_INCLUDED_IOSTREAM
	template <typename Char, typename Type>
	std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Array<Type>& Array) {
		ostream << "[" << Array.GetLength() << "]{ ";

		if (Array) {
			for (size i = 0; i < Array.GetLength() - 1; i++)
				ostream << Array[i] << ", ";

			ostream << Array[Array.GetLength() - 1] << " ";
		}

		ostream << "}";

		return ostream;
	}
#endif
}