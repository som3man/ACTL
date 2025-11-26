#pragma once

#include "../Allocation.hpp"

namespace ACTL {
	// Two way node-based resizable container template.
	template <typename Type>
	class List {
	public:
		// Represents a node with content.
		class Node {
		public:
			friend List<Type>;

			Node(const Node&) = delete;

			Node& operator =(const Node&) = delete;

			Type& operator *() {
				return data;
			}

			const Type& operator *() const {
				return data;
			}

			Type* operator ->() {
				return &data;
			}

			const Type* operator ->() const {
				return &data;
			}

			bool hasNext() const {
				return next;
			}

			bool hasPrev() const {
				return prev;
			}

			operator Type& () {
				return data;
			}

			operator const Type& () const {
				return data;
			}

			Node* getNext() {
				return next;
			}

			Node* getPrev() {
				return prev;
			}

			const Node* getNext() const {
				return next;
			}

			const Node* getPrev() const {
				return prev;
			}

			Node& operator ++(int) {
				return *next;
			}

			Node& operator --(int) {
				return *prev;
			}

			Node& operator ++() {
				return *next;
			}

			Node& operator --() {
				return *prev;
			}

			const Node& operator ++(int) const {
				return *next;
			}

			const Node& operator --(int) const {
				return *prev;
			}

			const Node& operator ++() const {
				return *next;
			}

			const Node& operator --() const {
				return *prev;
			}

			Node& operator +(size count) {
				Node* currNode = next;

				count--;

				while (count) {
					currNode = currNode->next;

					count--;
				}

				return *currNode;
			}

			Node& operator -(size count) {
				Node* currNode = prev;

				count--;

				while (count) {
					currNode = currNode->prev;

					count--;
				}

				return *currNode;
			}

			const Node& operator +(size count) const {
				const Node* currNode = next;

				count--;

				while (count) {
					currNode = currNode->next;

					count--;
				}

				return *currNode;
			}

			const Node& operator -(size count) const {
				const Node* currNode = prev;

				count--;

				while (count) {
					currNode = currNode->prev;

					count--;
				}

				return *currNode;
			}

		private:
			Node* prev = nullptr, * next = nullptr;

			Type data;

			template <typename... Args>
			Node(Args&&... args) : data(ACTL::forward<Args...>(args...)) {};
		};

		List() noexcept {

		}

		List(std::initializer_list<Type> list) {
			operator=(list);
		}

		List(const List& Another) {
			operator=(Another);
		}

		List(List&& Another) {
			operator=(ACTL::move(Another));
		}

		~List() noexcept {
			if (!first)
				return;

			Node* nextNode = first->next;

			delete first;

			while (nextNode) {
				const Node* currNode = nextNode;

				nextNode = currNode->next;

				delete currNode;
			}

			first = nullptr;

			last = nullptr;

			length = 0;
		}

		List& operator =(std::initializer_list<Type> list) {
			this->~List();

			for (auto& Object : list)
				EmplaceBack(Object);

			return *this;
		}

		List& operator =(const List& Another) {
			this->~List();

			const Node* CurrNode = Another.first;

			while (CurrNode) {
				const Type& Object = *CurrNode;

				EmplaceBack(Object);

				CurrNode = CurrNode->next;
			}

			return *this;
		}

		List& operator =(List&& Another) {
			this->~List();

			first = Another.first;

			last = Another.last;

			length = Another.length;

			Another.first = nullptr;

			Another.last = nullptr;

			Another.length = 0;

			return *this;
		}

		Node& First() {
			if (isEmpty())
				throw "List is empty!";

			return *first;
		}

		Node& Last() {
			if (isEmpty())
				throw "List is empty!";

			return *last;
		}

		const Node& First() const {
			if (isEmpty())
				throw "List is empty!";

			return *first;
		}

		const Node& Last() const {
			if (isEmpty())
				throw "List is empty!";

			return *last;
		}

		Node& operator [](size index) {
			Node* target = first;

			while (index) {
				target = target->next;

				index--;
			}

			return *target;
		}

		const Node& operator [](size index) const {
			const Node* target = first;

			while (index) {
				target = target->next;

				index--;
			}

			return *target;
		}

		template <typename... Args>
		Node& EmplaceBack(Args&&... args) {
			Node* newNode = new Node(ACTL::forward<Args...>(args...));

			if (!first) {
				first = newNode;

				last = newNode;
			}
			else {
				last->next = newNode;

				newNode->prev = last;

				last = newNode;
			}

			length++;

			return *newNode;
		}

		template <typename... Args>
		Node& EmplaceFront(Args&&... args) {
			Node* newNode = new Node(ACTL::forward<Args...>(args...));

			if (!first) {
				first = newNode;

				last = newNode;
			}
			else {
				first->prev = newNode;

				newNode->next = first;

				first = newNode;
			}

			length++;

			return *newNode;
		}

		void EraseBack() {
			Node* prevNode = last->prev;

			last->prev = nullptr;

			delete last;

			last = prevNode;

			if (last)
				last->next = nullptr;
			else
				first = nullptr;

			length--;
		}

		void EraseFront() {
			Node* nextNode = first->next;

			first->next = nullptr;

			delete first;

			first = nextNode;

			if (first)
				first->prev = nullptr;
			else
				last = nullptr;

			length--;
		}

		template <typename... Args>
		Node& EmplaceBefore(Node& Before, Args&&... args) {
			Node* newNode = new Node(ACTL::forward<Args...>(args...));

			newNode->next = &Before;

			newNode->prev = Before.prev;

			Before.prev = newNode;

			if (newNode->prev)
				newNode->prev->next = newNode;

			length++;

			return *newNode;
		}

		template <typename... Args>
		Node& EmplaceAfter(Node& After, Args&&... args) {
			Node* newNode = new Node(ACTL::forward<Args...>(args...));

			newNode->prev = &After;

			newNode->next = After.next;

			After.next = newNode;

			if (newNode->next)
				newNode->next->prev = newNode;

			length++;

			return *newNode;
		}

		void Erase(const Node& Target) {
			length--;

			if (!length) {
				first = nullptr;

				last = nullptr;
			}
			else if (&Target == first) {
				first = Target.next;

				first->prev = nullptr;
			}
			else if (&Target == last) {
				last = Target.prev;

				last->next = nullptr;
			}
			else {
				Target.prev->next = Target.next;

				Target.next->prev = Target.prev;
			}

			delete &Target;
		}

		bool Contains(const Node& Target) const {
			Node* currNode = first;

			while (currNode) {
				if (&Target == currNode)
					return true;

				currNode = currNode->next;
			}

			return false;
		}

		size getLength() const {
			return length;
		}

		bool isEmpty() const {
			return length == 0;
		}

		// Returns false if List is empty.
		operator bool() const {
			return length;
		}

		/*
		Calls given function with two arguments for each member of list from its beginning.

		First argument - Node from this List.
		Second argument - index(unsigned 64) of a Node.
		*/
		template <typename Func>
		void Foreach(Func func) {
			if (isEmpty())
				return;

			Node* CurrNode = first;

			size i = 0;

			while (CurrNode) {
				func(*CurrNode, i);

				CurrNode = CurrNode->getNext();

				i++;
			}
		}

		/*
		Calls given function with two arguments for each member of list from its beginning.

		First argument - Node from this List.
		Second argument - index(unsigned 64) of a Node.
		*/
		template <typename Func>
		const void Foreach(Func func) const {
			if (isEmpty())
				return;

			const Node* CurrNode = first;

			size i = 0;

			while (CurrNode) {
				func(*CurrNode, i);

				CurrNode = CurrNode->getNext();

				i++;
			}
		}

	private:
		Node* first = nullptr, * last = nullptr;

		size length = 0;
	};

	template <typename Type>
	class List<Type&> {
	public:
		using NoRef = ACTL::NoReferenceType<Type>;

		using PtrList = List<NoRef*>;

		class Node {
		public:
			friend List;

			Node(const Node&) = delete;

			Node& operator =(const Node&) = delete;

			NoRef& operator *() {
				return *pointer;
			}

			const NoRef& operator *() const {
				return *pointer;
			}

			NoRef* operator ->() {
				return pointer;
			}

			const NoRef* operator ->() const {
				return pointer;
			}

			bool hasNext() const {
				return next;
			}

			bool hasPrev() const {
				return prev;
			}

			operator NoRef& () {
				return *pointer;
			}

			operator const NoRef& () const {
				return *pointer;
			}

			Node* getNext() {
				return next;
			}

			Node* getPrev() {
				return prev;
			}

			const Node* getNext() const {
				return next;
			}

			const Node* getPrev() const {
				return prev;
			}

			Node& operator ++(int) {
				return *next;
			}

			Node& operator --(int) {
				return *prev;
			}

			Node& operator ++() {
				return *next;
			}

			Node& operator --() {
				return *prev;
			}

			const Node& operator ++(int) const {
				return *next;
			}

			const Node& operator --(int) const {
				return *prev;
			}

			const Node& operator ++() const {
				return *next;
			}

			const Node& operator --() const {
				return *prev;
			}

			Node& operator +(size count) {
				Node* currNode = next;

				count--;

				while (count) {
					currNode = currNode->next;

					count--;
				}

				return *currNode;
			}

			Node& operator -(size count) {
				Node* currNode = prev;

				count--;

				while (count) {
					currNode = currNode->prev;

					count--;
				}

				return *currNode;
			}

			const Node& operator +(size count) const {
				const Node* currNode = next;

				count--;

				while (count) {
					currNode = currNode->next;

					count--;
				}

				return *currNode;
			}

			const Node& operator -(size count) const {
				const Node* currNode = prev;

				count--;

				while (count) {
					currNode = currNode->prev;

					count--;
				}

				return *currNode;
			}

		private:
			using Ptr = List::PtrList::Node;

			Node* prev = nullptr, * next = nullptr;

			NoRef* pointer = nullptr;

			Node(NoRef* pointer) : pointer(pointer) {};
		};

		List() {};

		List(const List& Other) {
			operator=(Other);
		}

		List(List&& Other) noexcept {
			operator=(ACTL::move(Other));
		}

		List& operator =(const List& Other) {
			pointers = Other.pointers;

			return *this;
		}

		List& operator =(List&& Other) noexcept {
			pointers = ACTL::move(Other.pointers);

			return *this;
		}

		Node& First() {
			return (Node&)pointers.First();
		}

		const Node& First() const {
			return (const Node&)pointers.First();
		}

		Node& Last() {
			return (Node&)pointers.Last();
		}

		const Node& Last() const {
			return (const Node&)pointers.Last();
		}

		Node& operator [](size index) {
			return (Node&)pointers[index];
		}

		const Node& operator [](size index) const {
			return (const Node&)pointers[index];
		}

		Node& EmplaceBack(NoRef& reference) {
			return (Node&)pointers.EmplaceBack(&reference);
		}

		Node& EmplaceFront(NoRef& reference) {
			return (Node&)pointers.EmplaceFront(&reference);
		}

		Node& EmplaceAfter(const Node& After, NoRef& reference) {
			return (Node&)pointers.EmplaceAfter((typename Node::Ptr&)After, &reference);
		}

		Node& EmplaceBefore(const Node& Before, NoRef& reference) {
			return (Node&)pointers.EmplaceBefore((typename Node::Ptr&)Before, &reference);
		}

		void EraseBack() {
			pointers.EraseBack();
		}

		void EraseFront() {
			pointers.EraseFront();
		}

		void Erase(const Node& Target) {
			pointers.Erase((typename Node::Ptr&)Target);
		}

		size getLength() const {
			return pointers.getLength();
		}

		bool isEmpty() const {
			return pointers.isEmpty();
		}

		operator bool() const {
			return pointers.operator bool();
		}

		template <typename Func>
		void Foreach(Func func) {
			if (isEmpty())
				return;

			Node* CurrNode = &First();

			size i = 0;

			while (CurrNode) {
				func(*CurrNode, i);

				CurrNode = CurrNode->getNext();

				i++;
			}
		}

		template <typename Func>
		void Foreach(Func func) const {
			if (isEmpty())
				return;

			const Node* CurrNode = &First();

			size i = 0;

			while (CurrNode) {
				func(*CurrNode, i);

				CurrNode = CurrNode->getNext();

				i++;
			}
		}

	private:
		PtrList pointers = {};
	};

#ifdef ACTL_STD_INCLUDED_IOSTREAM
	template <typename Char, typename Type>
	std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const typename List<Type>::Node& Node) {
		const Type& Object = Node;

		return ostream << Object;
	}

	template <typename Char, typename Type>
	std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const List<Type>& List) {
		ostream << "[" << List.getLength() << "]{";

		auto Lambda = [&](const ACTL::List<Type>::Node& Node, size index) -> void {
			if (index != 0)
				ostream << ",";

			ostream << " " << Node;
		};

		List.Foreach(Lambda);

		return ostream << " }";
	}
#endif
}