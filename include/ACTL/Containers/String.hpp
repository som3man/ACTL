#pragma once

#include "../Allocation.hpp"

namespace ACTL {
	template <typename Char>
	extern size GetStringLength(const Char* str) {
		size i = 0;

		while (str[i])
			i++;

		return i;
	}

	// Null terminated string template.
	template <typename Char>
	class String {
	public:
		// Reference to the valid static String with zero length.
		static const String& Null() {
			auto Create = []() -> String {
				String New = String(0);

				New.first = (Char*)alloc(sizeof(Char));

				*New.first = 0;

				New.last = New.first + 1;

				New.ending = New.last;

				return New;
			};

			static String NullString = Create();

			return NullString;
		}

		// Creates new string from float value.
		static String FromFloat(f64 number, u8 precision, String Separator = ".") {
			const i64 intPart = i64(number);

			String NewStr = FromInt(intPart);

			if (!precision)
				return NewStr;

			f64 fracPart = number - f64(intPart) + 1.0;

			fracPart = fracPart * pow(10.0, f64(precision));

			const size sepLen = Separator.GetLength();

			Separator += FromUint(u64(fracPart));

			Separator.Erase(sepLen, 1);

			return NewStr + Separator;
		}

		// Creates new string from unsigned integer.
		static String FromUint(u64 number) {
			static const Char digits[10] = {
				'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
			};

			String NewStr = String(1);

			u64 numCopy = number;

			i8 numLength = 1;

			while (numCopy >= 10) {
				numCopy /= 10;

				numLength++;
			}

			const size capacity = size(numLength + 1);

			NewStr.first = (Char*)alloc(capacity * 2 * sizeof(Char));

			NewStr.last = NewStr.first + capacity;

			NewStr.ending = NewStr.last + capacity;

			for (size i = 0; number != 0; i++) {
				NewStr[i] = digits[number % 10];

				number /= 10;
			}

			NewStr[numLength] = 0;

			return NewStr.Reversed();
		}

		// Creates new string from signed integer.
		static String FromInt(i64 number) {
			if (number < 0)
				return String("-") + FromUint(u64(-number));

			return FromUint(u64(number));
		}

#ifdef ACTL_STD_INCLUDED_STRING
		String(const std::basic_string<Char>& string) {
			operator=(string);
		}

		String& operator =(const std::basic_string<Char>& string) {
			Delete();

			const size capacity = string.size() + 1;

			SetCapacity(capacity);

			last = first + capacity;

			for (size i = 0; i < capacity; i++)
				new(first + i) Char(string[i]);

			return *this;
		}

		String& operator +=(const std::basic_string<Char>& string) {
			Emplace(GetLength(), string);

			return *this;
		}

		String operator +(const std::basic_string<Char>& string) const {
			String NewStr = String(1);

			const size length = GetLength(), otherLength = string.size();

			const size capacity = length + otherLength + 1;

			NewStr.first = (Char*)alloc(capacity * sizeof(Char));

			for (size i = 0; i < length; i++)
				NewStr[i] = operator[](i);

			for (size i = 0; i < otherLength; i++)
				NewStr[i + length] = string[i];

			NewStr[length + otherLength] = 0;

			NewStr.last = NewStr.first + capacity;

			NewStr.ending = NewStr.last;

			return NewStr;
		}

		void Emplace(size index, const std::basic_string<Char>& string) {
			const size length = GetLength(), otherLength = string.size();

			const size capacity = length + otherLength + 1;

			if (GetCapacity() >= capacity) {
				if (index)
					for (size i = length; i >= index; i--)
						first[i + otherLength] = operator[](i);
				else
					for (size i = length; i != u64max; i--)
						first[i + otherLength] = operator[](i);

				for (size i = 0; i < otherLength; i++)
					first[i + index] = string[i];

				last = first + capacity;

				return;
			}

			Char* ptr = (Char*)alloc(capacity * 2 * sizeof(Char));

			for (size i = 0; i < index; i++)
				ptr[i] = operator[](i);

			for (size i = 0; i < otherLength; i++)
				ptr[i + index] = string[i];

			for (size i = index; i < length; i++)
				ptr[i + otherLength] = operator[](i);

			ptr[length + otherLength] = 0;

			Delete();

			first = ptr;

			last = first + capacity;

			ending = last + capacity;
		}
#endif

		String() noexcept {
			Init();
		}

		String(const Char* str) {
			Init();

			operator=(str);
		}

		String(const String& Another) {
			Init();

			operator=(Another);
		}

		String(String&& Another) noexcept {
			Init();

			operator=(ACTL::move(Another));
		}

		~String() {
			Delete();
		}

		String& operator =(const Char* str) {
			Delete();

			const size capacity = GetStringLength(str) + 1;

			SetCapacity(capacity);

			last = first + capacity;

			for (size i = 0; i < capacity; i++)
				new(first + i) Char(str[i]);

			return *this;
		}

		String& operator =(const String& Another) {
			Delete();

			if (!Another)
				return *this;

			const size capacity = Another.GetLength() + 1;

			SetCapacity(capacity);

			last = first + capacity;

			for (size i = 0; i < capacity; i++)
				new(first + i) Char(Another[i]);

			return *this;
		}

		String& operator =(String&& Another) noexcept {
			Delete();

			first = Another.first;

			last = Another.last;

			ending = Another.ending;

			Another.first = Null().first;

			Another.last = Null().last;

			Another.ending = Null().ending;

			return *this;
		}

		String& operator +=(const Char* str) {
			Emplace(GetLength(), str);

			return *this;
		}

		String& operator +=(const String& Another) {
			Emplace(GetLength(), Another);

			return *this;
		}

		String operator +(const Char* str) const {
			String NewStr = String(1);

			const size length = GetLength(), otherLength = GetStringLength(str);

			const size capacity = length + otherLength + 1;

			NewStr.first = (Char*)alloc(capacity * sizeof(Char));

			for (size i = 0; i < length; i++)
				NewStr[i] = operator[](i);

			for (size i = 0; i < otherLength; i++)
				NewStr[i + length] = str[i];

			NewStr[length + otherLength] = 0;

			NewStr.last = NewStr.first + capacity;

			NewStr.ending = NewStr.last;

			return NewStr;
		}

		String operator +(const String& Another) const {
			String NewStr = String(1);

			const size length = GetLength(), otherLength = Another.GetLength();

			const size capacity = length + otherLength + 1;

			NewStr.first = (Char*)alloc(capacity * sizeof(Char));

			for (size i = 0; i < length; i++)
				NewStr[i] = operator[](i);

			for (size i = 0; i < otherLength; i++)
				NewStr[i + length] = Another[i];

			NewStr[length + otherLength] = 0;

			NewStr.last = NewStr.first + capacity;

			NewStr.ending = NewStr.last;

			return NewStr;
		}

		void Emplace(size index, const Char* str) {
			const size length = GetLength(), otherLength = GetStringLength(str);

			const size capacity = length + otherLength + 1;

			if (GetCapacity() >= capacity) {
				for (size i = length; i >= index; i--)
					first[i + otherLength] = operator[](i);

				for (size i = 0; i < otherLength; i++)
					first[i + index] = str[i];

				last = first + capacity;

				return;
			}

			Char* ptr = (Char*)alloc(capacity * 2 * sizeof(Char));

			for (size i = 0; i < index; i++)
				ptr[i] = operator[](i);

			for (size i = 0; i < otherLength; i++)
				ptr[i + index] = str[i];

			for (size i = index; i < length; i++)
				ptr[i + otherLength] = operator[](i);

			ptr[length + otherLength] = 0;

			Delete();

			first = ptr;

			last = first + capacity;

			ending = last + capacity;
		}

		void Emplace(size index, const String& Another) {
			const size length = GetLength(), otherLength = Another.GetLength();

			const size capacity = length + otherLength + 1;

			if (GetCapacity() >= capacity) {
				if (index)
					for (size i = length; i >= index; i--)
						first[i + otherLength] = operator[](i);
				else
					for (size i = length; i != u64max; i--)
						first[i + otherLength] = operator[](i);

				for (size i = 0; i < otherLength; i++)
					first[i + index] = Another[i];

				last = first + capacity;

				return;
			}

			Char* ptr = (Char*)alloc(capacity * 2 * sizeof(Char));

			for (size i = 0; i < index; i++)
				ptr[i] = operator[](i);

			for (size i = 0; i < otherLength; i++)
				ptr[i + index] = Another[i];

			for (size i = index; i < length; i++)
				ptr[i + otherLength] = operator[](i);

			ptr[length + otherLength] = 0;

			Delete();

			first = ptr;

			last = first + capacity;

			ending = last + capacity;
		}

		void Erase(size index, size count) {
			const size length = GetLength(), tailLength = length - (index + count);

			for (size i = 0; i <= tailLength; i++)
				first[index + i] = first[index + count + i];

			last -= count;
		}

		// Normalizes given signed value to strings length.
		size ToRange(i64 index) const {
			const size length = GetLength();

			if (!length)
				return 0;

			while (index < 0)
				index += length;

			while (index >= length)
				index -= length;

			return index;
		}

		bool isNull() const {
			return GetLength() == 0;
		}

		operator bool() const {
			return !isNull();
		}

		operator Char* () {
			return first;
		}

		operator const Char* () const {
			return first;
		}

		Char& operator [](size index) {
			return first[index];
		}

		const Char& operator [](size index) const {
			return first[index];
		}

		Char* begin() {
			return first;
		}

		Char* end() {
			return last - 1;
		}

		const Char* begin() const {
			return first;
		}

		const Char* end() const {
			return last - 1;
		}

		void Clear() {
			first[0] = 0;

			last = first + 1;
		}

		size GetLength() const {
			if (first == last)
				return 0;

			return last - first - 1;
		}

		void ShrinkToFit() {
			const size length = GetLength() + 1;

			if (length == GetCapacity())
				return;

			Char* ptr = (Char*)alloc(length * sizeof(Char));

			MoveData(ptr, length);

			Delete();

			first = ptr;

			ending = first + length;

			last = ending;
		}

		void SetCapacity(size newCapacity) {
			if (!newCapacity)
				return Delete();

			if (newCapacity == GetCapacity())
				return;

			Char* ptr = (Char*)alloc(newCapacity * sizeof(Char));

			if (first) {
				MoveData(ptr, newCapacity);

				Delete();
			}
			else {
				last = ptr;

				*ptr = 0;
			}

			first = ptr;

			ending = first + newCapacity;
		}

		String Reversed() const {
			String NewStr = String(1);

			const size capacity = GetCapacity(), length = GetLength();

			NewStr.first = (Char*)alloc(capacity * sizeof(Char));

			NewStr.last = NewStr.first + length + 1;

			NewStr.ending = NewStr.first + capacity;

			for (size i = 0; i < length; i++)
				NewStr[i] = first[length - 1 - i];

			NewStr[length] = 0;

			return NewStr;
		}

		size GetCapacity() const {
			return ending - first;
		}

		size GetSize() const {
			return GetCapacity() * sizeof(Char);
		}

		size GetFullSize() const {
			return GetSize() + sizeof(String);
		}

	private:
		Char* first, * last, * ending;

		String(int) noexcept {
			first = nullptr; last = nullptr; ending = nullptr;
		}

		void Init() {
			first = Null().first;

			ending = Null().ending;

			last = Null().last;
		}

		void Delete() {
			if (first == Null().first)
				return;

			dealloc(first);

			Init();
		}

		void MoveData(Char* ptr, size count) {
			if (last >= first + count) {
				for (size i = 0; i < count; i++)
					new(ptr + i) Char(ACTL::move(first[i]));

				last = ptr + count;
			}
			else {
				const size length = GetLength();

				for (size i = 0; i < length; i++)
					new(ptr + i) Char(ACTL::move(first[i]));

				last = ptr + length;
			}
		}
	};

#ifdef ACTL_STD_INCLUDED_IOSTREAM 
	template <typename Char>
	std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& stream, const ACTL::String<Char>& String) {
		return stream << String.begin();
	}
#endif
}