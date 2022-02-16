#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

template <typename P, typename T>
class _iterator {
  using stack_type = typename P::stack_type;
  stack_type current;
  P* pool;

 public:
  using value_type = T;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;
  ~_iterator() noexcept {}

  _iterator(P* p, stack_type nd) : current(nd), pool(p){};

  _iterator& operator++() {  // pre-increment
    current = pool->next(current);
    return *this;
  }

  _iterator operator++(int) {  // post-increment
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  reference operator*() { return pool->value(current); }
  typename P::node_t* operator->() { return &(pool->node(current)); }

  friend bool operator==(const _iterator& a, const _iterator& b) {
    return (a.current == b.current);
  }

  friend bool operator!=(const _iterator& a, const _iterator& b) {
    return !(a == b);
  }
};

template <typename T, typename N = std::size_t>
class stack_pool {
  struct node_t {
    T value;
    N next;
    node_t(const T& x, N nxt) : value{x}, next{nxt} {};
    node_t(T&& x, N nxt) : value{std::move(x)}, next{nxt} {};
    explicit node_t(N nxt) : next{nxt} {};
  };
  using stack_type = N;
  using value_type = T;
  std::vector<node_t> pool;
  using size_type = typename std::vector<node_t>::size_type;
  stack_type free_nodes;  // at the beginning, it is empty

  node_t& node(stack_type x) noexcept { return pool[x - 1]; }
  const node_t& node(stack_type x) const noexcept { return pool[x - 1]; }

  template <typename P_, typename T_>
  friend class _iterator;

  template <typename X>
  stack_type push_universal(X&& val, stack_type head) {
    if (empty(free_nodes)) {
      pool.push_back(std::move(node_t(end())));
      free_nodes = pool.size();
    }
    auto new_head = free_nodes;
    free_nodes = next(free_nodes);
    value(new_head) = std::forward<X>(val);
    next(new_head) = head;
    return new_head;
  }

 public:
  stack_pool() : free_nodes{stack_type{0}} {};

  explicit stack_pool(size_type n): stack_pool() {
    pool.reserve(n);
  }  // reserve n nodes in the pool

  using iterator = _iterator<stack_pool<T, N>, T>;
  using const_iterator = _iterator<stack_pool<T, N>, const T>;

  iterator begin(stack_type x) { return iterator(this, x); };
  iterator end(stack_type) {
    return iterator(this, end());
  };  // this is not a typo

  const_iterator begin(stack_type x) const { return const_iterator(this, x); };
  const_iterator end(stack_type) const { return const_iterator(this, end()); };

  const_iterator cbegin(stack_type x) const;
  const_iterator cend(stack_type) const;

  stack_type new_stack() { return end(); };  // return an empty stack

  void reserve(size_type n) {
    pool.reserve(n);
  };  // reserve n nodes in the pool

  size_type capacity() const {
    return pool.capacity();
  };  // the capacity of the pool

  bool empty(stack_type x) const noexcept { return x == end(); };

  stack_type end() const noexcept { return stack_type(0); }


  T& value(stack_type x) { return node(x).value; };
  const T& value(stack_type x) const { return node(x).value; };

  stack_type& next(stack_type x) { return node(x).next; }
  const stack_type& next(stack_type x) const {
    return const_cast<stack_type>(node(x).next);
  };

  stack_type push(T&& val, stack_type head) {
    return push_universal(std::move(val), head);
  };

  stack_type push(const T& val, stack_type head) {
    return push_universal(val, head);
  };

  stack_type pop(stack_type x) {
    auto new_head = next(x);
    next(x) = free_nodes;
    free_nodes = x;
    return new_head;
  };  // delete first node

  // we need to find the bottom
  stack_type free_stack(stack_type x) {
    if (empty(x))
      return x;

    iterator bottom = begin(x);
    for (; bottom->next != end(); ++bottom)
      ;
    bottom->next = free_nodes;
    free_nodes = x;
    return end();
  };
};
