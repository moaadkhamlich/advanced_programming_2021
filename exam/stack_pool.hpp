// relevant headers
#include <iterator>
#include <vector>

/// \file
/// Header file of the stack_pool class.
/// \dir
/// Directory containing the header and source files for the stack_pool<T,N>
/// class.

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/*---------------------------------------------------------------------------*\
                        Class template _iterator<P,T>
\*---------------------------------------------------------------------------*/

/** @brief The class _iterator is used as an iterator for accessing the elements
    elements of a stack within stack_pool.

    The class _iterator is used as an iterator for accessing the elements
    of a stack within stack_pool. This is done using as template
    parameters, the stack_pool type and the value type of value stored in each
    node of the stacks, which however does not match the template parameter of
    stack_pool<T,N>, in fact the difference may reside in the const
    attribute, that allows to obtain const_iterator */

template <typename P, typename T>
class _iterator {
  // private members
 private:
  /// stack_type deduced from the template parameter P=stack_pool<T,N>
  using stack_type = typename P::stack_type;

  /// the "pointer" used to access element inside the stack_pool data-structure
  stack_type current;

  /// a raw pointer to stack_pool
  P* pool;

 public:
  /// Destructor
  /// destructor is always noexcept, we do nothing because there
  /// is no resource allocation
  ~_iterator() noexcept {}

  /// typename used to be compliant with standard library
  using value_type = T;
  /// typename used to be compliant with standard library
  using reference = value_type&;
  /// typename used to be compliant with standard library
  using pointer = value_type*;
  /// typename used to be compliant with standard library
  using difference_type = std::ptrdiff_t;
  /// typename used to be compliant with standard library
  using iterator_category = std::forward_iterator_tag;

  /// Defined constructor for the _iterator class.
  /// we check to have a valid iterator by dereferencing it.
  /// Because of this check we cannot mark the constructor as noexcept since
  /// stack_pool.value may throw.
  _iterator(P* p, stack_type nd) : current(nd), pool(p) {
    if (!p->empty(nd))
      *(*this);
  };

  /// Default constructor
  _iterator() : current(stack_type(0)), pool(nullptr){};

  /// Default copy constructor is fine, member-wise copy.
  /// This is marked as noexcept because we assume that the iterator we are
  /// copying has already been successfully created
  _iterator(const _iterator&) noexcept = default;

  /// Move constructor
  /// We cannot use the default one because we have to set pool to nullptr
  /// This is marked as noexcept because we are just moving a supposed valid
  /// and already created iterator
  _iterator(_iterator&& that) noexcept
      : current(std::move(that.current)), pool(that.pool) {
    that.current = stack_type(0);
    that.pool = nullptr;
  };

  /// Move assignment
  /// This is marked as noexcept because we are just moving a supposed valid
  /// and already created iterator
  _iterator& operator=(_iterator&& that) noexcept {
    current = std::move(that.current);
    pool = that.pool;
    that.current = stack_type(0);
    that.pool = nullptr;
    return *this;
  }

  /// Copy assignment
  /// This is marked as noexcept because we are using copy constructor and
  /// move assignment, both noexcept
  _iterator& operator=(const _iterator& that) {
    // copy constructor
    _iterator tmp = that;
    // move assignment
    (*this) = std::move(tmp);
    return *this;
  }

  /// operator pre-increment: throws an exception if iterator=
  /// stack_pool.end()
  _iterator& operator++() {
    current = pool->next(current);
    return *this;
  }

  /// operator post-increment: throws an exception if iterator=
  /// stack_pool.end()
  _iterator operator++(int) {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  /// Dereference operator which return the value of the pointed node.
  /// This cannot be marked noexcept because stack_pool.value may throw
  reference operator*() const { return pool->value(current); }

  /// operator -> used to access the member of the pointed node, e.x. "next"
  /// this cannot be marked as noexcept because stack_pool.node may throw
  typename P::node_t* operator->() const { return &(pool->node(current)); }

  /// The equality operator for two iterators.
  /// Two iterators are equal if they refer to the same pool and point to
  /// the same node.
  friend bool operator==(const _iterator& a, const _iterator& b) noexcept {
    return ((a.current == b.current) && (a.pool == b.pool));
  }

  /// The inequality operator for two iterators.
  /// Two iterators are not equal if they don't refer to the same pool or
  /// they do not point to the same node.
  friend bool operator!=(const _iterator& a, const _iterator& b) noexcept {
    return !(a == b);
  }
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// //

/*---------------------------------------------------------------------------*\
                        Class template stack_pool<T,N>
\*---------------------------------------------------------------------------*/

/// The pool stores each node in a std::vector<node_t>. The "address" of a
/// node is 1+idx, where idx is the index where the node is stored in the
/// vector. This trick allows us to use address 0 as end, so we can use
/// unsigned integers type. The first node stored in the vector will be put at
/// idx == 0, but it will be referenced as 1.

/** prova generazione documentazione */

template <typename T, typename N = std::size_t>
class stack_pool {
  // private members
 private:
  /// struct that implements the concept of a node in the stack data structure
  struct node_t {
    /// the value of a node
    T value;
    /// the index pointing to the next node
    N next;
    /// contractor taking a value, and an index to the next element
    node_t(const T& x, N nxt) : value{x}, next{nxt} {};
    node_t(T&& x, N nxt) : value{std::move(x)}, next{nxt} {};
    explicit node_t(N nxt) : next{nxt} {};
  };

  /// the type of the index: it must be an unsigned int, since we are using 0
  /// as null index
  using stack_type = N;

  /// the type of the values stored in the stack_pool data structure
  using value_type = T;

  /// the vector used to store the nodes of each stack
  std::vector<node_t> pool;

  /// size_type as deduced from the underlying templated vector
  using size_type = typename std::vector<node_t>::size_type;

  /// the stack of free_nodes
  stack_type free_nodes;

  /// Function that returns a reference to the node corresponding to a given
  /// index. I have removed the noexcept keyword since vector::operator[] may
  /// throw
  node_t& node(stack_type x) { return pool[x - 1]; }

  /// function that returns a const reference to the node corresponding to a
  /// given index. I have removed the noexcept keyword since
  /// vector::operator[] may throw
  const node_t& node(stack_type x) const { return pool[x - 1]; }

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

  explicit stack_pool(size_type n) : stack_pool() {
    pool.reserve(n);
  }  // reserve n nodes in the pool

  template <typename L>
  stack_type create_pool(L list) {
    for (auto&& x : list)
      push(std::move(x));
  }

  using iterator = _iterator<stack_pool<T, N>, T>;
  using const_iterator = _iterator<stack_pool<T, N>, const T>;

  iterator begin(stack_type x) { return iterator(this, x); };
  iterator end(stack_type) {
    return iterator(this, end());
  };  // this is not a typo

  const_iterator begin(stack_type x) const { return const_iterator(this, x); };
  const_iterator end(stack_type) const { return const_iterator(this, end()); };

  // const iterator cbegin and cend
  const_iterator cbegin(stack_type x) const { return const_iterator(this, x); };
  const_iterator cend(stack_type) const { return const_iterator(this, end()); };

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
    // empy for loop used to reach the bottom of the stack
    for (; bottom->next != end(); ++bottom)
      ;
    bottom->next = free_nodes;
    free_nodes = x;
    return end();
  };
};
