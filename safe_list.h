#ifndef SAFELIST_H
#define SAFELIST_H

#include <cstddef>
#include <list>
#include <string>
#include <concepts>
#include <utility>
#include <set>
#include <memory>

namespace safe {
    template <typename T>
    class list {
        std::list<T> elements;

    public:
        // Forward declarations for friend classes
        template<typename IterType, typename ContainerType>
        class iterator_base;
        class iterator;
        class const_iterator;

    private:
        // In order to make iterators safe for deletion we need to update them
        // when a removal occurs.
        mutable std::set<iterator*> active_iterators;
        mutable std::set<const_iterator*> active_const_iterators;

    public:
        // Base iterator template for shared logic between const and non-const iterators
        template<typename IterType, typename ContainerType>
        class iterator_base {
        protected:
            IterType current;
            IterType next;
            ContainerType* container;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = std::conditional_t<std::is_const_v<std::remove_pointer_t<ContainerType>>, const T*, T*>;
            using reference = std::conditional_t<std::is_const_v<std::remove_pointer_t<ContainerType>>, const T&, T&>;

            constexpr iterator_base(IterType curr, ContainerType* cont)
                : current(curr), container(cont) {
                if (curr != cont->end()) {
                    next = curr;
                    ++next;
                } else {
                    next = curr;
                }
            }

            constexpr void handle_erasure(IterType erased_pos, IterType next_pos) {
                // If our current position is the erased position, update it
                if (current == erased_pos) {
                    current = next_pos;
                }
                // If our next position is the erased position, update it
                if (next == erased_pos) {
                    next = next_pos;
                }
            }

            constexpr auto& operator++() {
                current = next;
                if (next != container->end()) {
                    ++next;
                }
                return *this;
            }

            constexpr auto operator++(int) {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr bool operator==(const iterator_base& other) const {
                return current == other.current;
            }

            constexpr bool operator!=(const iterator_base& other) const {
                return current != other.current;
            }

            // Get the next iterator position without advancing
            constexpr IterType getNext() const {
                return next;
            }

            // Get the current underlying STL iterator
            constexpr IterType getBase() const {
                return current;
            }
        };

        // Non-const iterator
        class iterator : public iterator_base<typename std::list<T>::iterator, std::list<T>> {
            using Base = iterator_base<typename std::list<T>::iterator, std::list<T>>;
            friend class list<T>;

        public:
            // basic constructor
            constexpr iterator(typename std::list<T>::iterator curr, std::list<T>* cont)
                : Base(curr, cont) {
                if (cont) {
                    const_cast<list<T>*>(reinterpret_cast<const list<T>*>(cont))->active_iterators.insert(this);
                }
            }

            // copy constructor
            constexpr iterator(const iterator& other)
                : Base(other) {
                if (this->container) {
                    const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_iterators.insert(this);
                }
            }

            // move constructor
            constexpr iterator(iterator&& other) noexcept
                : Base(std::move(other)) {
                if (this->container) {
                    // Deregister the other iterator and register this one
                    auto* list_ptr = const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container));
                    list_ptr->active_iterators.erase(&other);
                    list_ptr->active_iterators.insert(this);
                }
            }

            constexpr iterator& operator=(const iterator& other) {
                if (this != &other) {
                    // Deregister from current container if any
                    if (this->container) {
                        const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_iterators.erase(this);
                    }

                    // Call base class assignment
                    Base::operator=(other);

                    // Register with the new container if any
                    if (this->container) {
                        const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_iterators.insert(this);
                    }
                }
                return *this;
            }

            ~iterator() {
                if (this->container) {
                    const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_iterators.erase(this);
                }
            }

            constexpr T operator*() const {
                return *this->current;
            }

            constexpr T* operator->() const {
                return const_cast<T*>(&(*this->current));
            }
        };

        // Const iterator
        class const_iterator : public iterator_base<typename std::list<T>::const_iterator, const std::list<T>> {
            using Base = iterator_base<typename std::list<T>::const_iterator, const std::list<T>>;
            friend class list<T>;

        public:
            // base constructor
            constexpr const_iterator(typename std::list<T>::const_iterator curr, const std::list<T>* cont)
                : Base(curr, cont) {
                if (cont) {
                    const_cast<list<T>*>(reinterpret_cast<const list<T>*>(cont))->active_const_iterators.insert(this);
                }
            }

            // copy constructor
            constexpr const_iterator(const const_iterator& other)
                : Base(other) {
                if (this->container) {
                    const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_const_iterators.insert(this);
                }
            }

            // move constructor
            constexpr const_iterator(const_iterator&& other) noexcept
                : Base(std::move(other)) {
                if (this->container) {
                    // Deregister the other iterator and register this one
                    auto* list_ptr = const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container));
                    list_ptr->active_const_iterators.erase(&other);
                    list_ptr->active_const_iterators.insert(this);
                }
            }

            constexpr const_iterator& operator=(const const_iterator& other) {
                if (this != &other) {
                    // Deregister from current container if any
                    if (this->container) {
                        const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_const_iterators.erase(this);
                    }

                    // Call base class assignment
                    Base::operator=(other);

                    // Register with the new container if any
                    if (this->container) {
                        const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_const_iterators.insert(this);
                    }
                }
                return *this;
            }

            ~const_iterator() {
                if (this->container) {
                    const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_const_iterators.erase(this);
                }
            }

            // conversion from non-const iterator
            constexpr const_iterator(const iterator& it)
                : Base(it.getBase(), nullptr) {
                if (it.getNext() != typename std::list<T>::iterator()) {
                    this->next = it.getNext();
                } else {
                    this->next = this->current;
                }

                if (this->container) {
                    const_cast<list<T>*>(reinterpret_cast<const list<T>*>(this->container))->active_const_iterators.insert(this);
                }
            }

            constexpr T operator*() const {
                return *this->current;
            }

            constexpr const T* operator->() const {
                return &(*this->current);
            }
        };

        // non-const iterators
        [[nodiscard]] constexpr iterator begin() { return iterator(elements.begin(), &elements); }
        [[nodiscard]] constexpr iterator end() { return iterator(elements.end(), &elements); }

        // const iterators
        [[nodiscard]] constexpr const_iterator begin() const { return const_iterator(elements.begin(), &elements); }
        [[nodiscard]] constexpr const_iterator end() const { return const_iterator(elements.end(), &elements); }
        [[nodiscard]] constexpr const_iterator cbegin() const { return const_iterator(elements.begin(), &elements); }
        [[nodiscard]] constexpr const_iterator cend() const { return const_iterator(elements.end(), &elements); }

        // helper method to erase with custom iterator
        constexpr iterator erase(iterator it) {
            auto erased_pos = it.getBase();
            auto next_pos = it.getNext();

            // Notify all iterators before erasure
            for (auto* iter_ptr : active_iterators) {
                iter_ptr->handle_erasure(erased_pos, next_pos);
            }

            // Notify all const iterators before erasure (using a cast to make the comparison work)
            for (auto* iter_ptr : active_const_iterators) {
                iter_ptr->handle_erasure(
                    typename std::list<T>::const_iterator(erased_pos),
                    typename std::list<T>::const_iterator(next_pos)
                );
            }

            // Perform the actual erasure
            elements.erase(erased_pos);

            // Return an iterator pointing to the position after the erased element
            return iterator(next_pos, &elements);
        }

        [[nodiscard]] constexpr size_t size() const { return elements.size(); }
        constexpr void clear() { elements.clear(); }
        [[nodiscard]] constexpr T& front() { return elements.front(); }
        [[nodiscard]] constexpr const T& front() const { return elements.front(); }

        [[nodiscard]] constexpr T& back() { return elements.back(); }
        [[nodiscard]] constexpr const T& back() const { return elements.back(); }


        constexpr void push_back(const T& o) requires std::copyable<T> {
            elements.push_back(o);
        }

        constexpr void push_back(T&& o) {
            elements.push_back(std::move(o));
        }

        constexpr void push_front(const T& o) requires std::copyable<T> {
            elements.push_front(o);
        }

        constexpr void push_front(T&& o) {
            elements.push_front(std::move(o));
        }

        constexpr void pop_back() { elements.pop_back(); }
        constexpr void pop_front() { elements.pop_front(); }

        // remove method uses erase to notify iterators of changes
        constexpr void remove(const T& o) {
            for (auto it = begin(); it != end(); ) {
                if (*it == o) {
                    it = erase(it);
                } else {
                    ++it;
                }
            }
        }

        [[nodiscard]] constexpr auto empty() const noexcept { return elements.empty(); }
    };
} // namespace safe

// C++20 compatible std::erase/erase_if implementation
namespace std {
    template <typename T>
    size_t erase(safe::list<T>& c, const T& value) {
        size_t count = 0;
        for (auto it = c.begin(); it != c.end(); ) {
            if (*it == value) {
                it = c.erase(it);
                ++count;
            } else {
                ++it;
            }
        }
        return count;
    }

    template <typename T, typename Pred>
    size_t erase_if(safe::list<T>& c, Pred pred) {
        size_t count = 0;
        for (auto it = c.begin(); it != c.end(); ) {
            if (pred(*it)) {
                it = c.erase(it);
                ++count;
            } else {
                ++it;
            }
        }
        return count;
    }
} // namespace std extensions

#endif // SAFELIST_H
