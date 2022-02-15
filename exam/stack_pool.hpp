#include <vector>
template <typename T, typename N = std::size_t>
class stack_pool {
  struct node_t {
    T value;
    N next;
  };
  std::vector<node_t> pool;
  using stack_type = N;
  using value_type = T;
  using size_type = typename std::vector<node_t>::size_type;
  stack_type free_nodes;  // at the beginning, it is empty

  node_t& node(stack_type x) noexcept { return pool[x - 1]; }
  const node_t& node(stack_type x) const noexcept { return pool[x - 1]; }

 public:
  stack_pool() : free_nodes{stack_type{0}} {};

  explicit stack_pool(size_type n) {
    pool.reserve(n);
  }  // reserve n nodes in the pool

  // using iterator = ...;
  // using const_iterator = ...;

  // iterator begin(stack_type x);
  // iterator end(stack_type ); // this is not a typo

  // const_iterator begin(stack_type x) const;
  // const_iterator end(stack_type ) const;

  // const_iterator cbegin(stack_type x) const;
  // const_iterator cend(stack_type ) const;

  stack_type new_stack() { return end(); };  // return an empty stack

  void reserve(size_type n) {
    pool.reserve(n);
  };  // reserve n nodes in the pool

  size_type capacity() const {
    return pool.capacity();
  };  // the capacity of the pool

  bool empty(stack_type x) const { return x == end(); };

  stack_type end() const noexcept { return stack_type(0); }

  T& value(stack_type x) { return node(x).value; };
  const T& value(stack_type x) const { return node(x).value; };

  stack_type& next(stack_type x) { return node(x).next; }
  const stack_type& next(stack_type x) const {
    return const_cast<stack_type>(node(x).next);
  };

  stack_type push(const T& val, stack_type head) {
    if (!empty(free_nodes)) {
      auto new_head = free_nodes;
      free_nodes = next(free_nodes);
      value(new_head) = val;
      next(new_head) = head;
      return new_head;
    }
    node_t new_node(val, head);
    return stack_type(pool.size());
  }

  //stack_type push(T&& val, stack_type head);

  stack_type pop(stack_type x)
    {
        auto new_head= next(x);
        next(x)= free_nodes;
        free_nodes= x;
        return new_head;
    };  // delete first node

  stack_type free_stack(stack_type x);  // to be done with iterators: we need to find bottom of the stack and
                                        // make it point to head of the stack we want to empty
};
