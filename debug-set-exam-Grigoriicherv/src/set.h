#pragma once

#include <cassert>
#include <iterator>
#include <list>
#include <memory>
#include <unordered_map>
#include <utility>

template <typename T>
class set {
  struct node;
  struct basenode;

  template <typename R>
  struct my_iterator {
  public:
    using value_type = T;

    using reference = value_type&;
    using const_reference = const value_type&;

    using pointer = value_type*;
    using const_pointer = const value_type*;

    using iterator_category = std::bidirectional_iterator_tag;
    using iterator_concept = std::bidirectional_iterator_tag;

    using difference_type = ptrdiff_t;

    friend class set<T>;

    my_iterator() = default;

    my_iterator(std::nullptr_t) = delete;

    my_iterator(basenode* ptr, const set* st, bool beg) : ptr(ptr), my_set(st), begin(beg), valid(ptr->valid) {}

    template <typename Q>
    requires std::is_same_v<const Q, R>
    my_iterator(const my_iterator<Q>& other) : ptr(other.ptr) {}

    my_iterator& operator=(const my_iterator& other) = default;

    ~my_iterator() = default;

    my_iterator& operator++() {

      begin = false;
      if (ptr == nullptr) {
        std::abort();
      }

      if (my_set->valid_nodes.contains(ptr)) {
        if (!my_set->valid_nodes.at(ptr) && !(*valid.get())) {
          std::abort();
        }
      }

      if (ptr == finding_fake() && ptr->parent == nullptr && ptr->right == nullptr && ptr->left == nullptr) {
        std::abort();
      }

      ptr = next(static_cast<node*>(ptr));
      valid = ptr->valid;
      return *this;
    }

    my_iterator operator++(int) {
      my_iterator x = *this;
      ++*this;
      return x;
    }

    my_iterator& operator--() {
      if (ptr == nullptr) {
        std::abort();
      }
      if (begin) {
        std::abort();
      }
      if (my_set->valid_nodes.contains(ptr)) {
        if (!my_set->valid_nodes.at(ptr) && !(*valid.get())) {
          std::abort();
        }
      }
      if (ptr == finding_fake() && ptr->parent == nullptr && ptr->right == nullptr && ptr->left == nullptr) {
        std::abort();
      }
      ptr = prev(static_cast<node*>(ptr));
      valid = ptr->valid;
      return *this;
    }

    my_iterator operator--(int) {
      my_iterator x = *this;
      --*this;
      return x;
    }

    const R& operator*() const {

      if (ptr == nullptr) {}
      if (ptr == nullptr || ptr == &my_set->fake || !(*valid.get())) {
        std::abort();
      }
      return static_cast<node*>(ptr)->val;
    }

    const R* operator->() const {
      return &(static_cast<node*>(ptr)->val);
    }

    friend bool operator==(const my_iterator& a, const my_iterator& b) {
      if (a.ptr == nullptr || b.ptr == nullptr) {
        std::abort();
      }

      if (a.finding_fake() != b.finding_fake()) {
        std::abort();
      }
      return a.ptr == b.ptr;
    }

    friend bool operator!=(const my_iterator& a, const my_iterator& b) {
      if (a.ptr == nullptr || b.ptr == nullptr) {
        std::abort();
      }
      if (a.finding_fake() != b.finding_fake()) {
        std::abort();
      }
      return a.ptr != b.ptr;
    }

    friend void swap(my_iterator& a, my_iterator& b) noexcept {
      if (a.ptr == b.ptr) {
        std::abort();
      }
      using std::swap;
      swap(a.ptr, b.ptr);
      swap(a.begin, b.begin);
      std::swap(a.my_set, b.my_set);
      std::swap(a.valid, b.valid);
    }

  private:
    basenode* ptr = nullptr;
    const set* my_set = nullptr;
    bool begin = false;
    std::shared_ptr<bool> valid = std::make_shared<bool>(false);

    basenode* finding_fake() const {
      basenode* tmp = ptr;

      while (tmp->parent != nullptr) {
        tmp = tmp->parent;
      }
      return tmp;
    }

    basenode* finding_on_ptr(basenode* target, basenode* nd) {
      if (nd == target) {
        return nd;
      }
      if (nd->left != nullptr) {
        return finding_on_ptr(target, nd->left);
      }
      if (nd->right != nullptr) {
        return finding_on_ptr(target, nd->right);
      }
      return my_set->fake;
    }
  };

  struct basenode {
    basenode* left;
    basenode* right;
    basenode* parent;
    std::shared_ptr<bool> valid = nullptr;

    basenode() = default;

    basenode(basenode* left, basenode* right, basenode* parent) : left(left), right(right), parent(parent) {
      valid = std::make_shared<bool>(true);
    }

    ~basenode() {
      if (valid != nullptr) {
        *valid.get() = false;
      }
    }
  };

  struct node : basenode {
    T val;

    node() = delete;

    node(const T& val, basenode* l = nullptr, basenode* r = nullptr, basenode* p = nullptr)
        : basenode(l, r, p),
          val(val) {}

    ~node() = default;
  };

  mutable basenode fake;
  size_t size_set;
  std::unordered_map<basenode*, bool> valid_nodes;

public:
  using iterator = my_iterator<T>;
  using const_iterator = my_iterator<T>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // O(1) nothrow
  set() : fake(), size_set(0) {}

  // O(n) strong
  set(const set& other) : set() {
    size_set = other.size_set;
    if (other.fake.left != nullptr) {
      fake.left = new node(static_cast<node*>(other.fake.left)->val, nullptr, nullptr, &fake);
      fake.left->valid = std::make_shared<bool>(true);
      valid_nodes.insert(std::make_pair(fake.left, true));
      if (other.fake.left->right != nullptr) {
        fake.left->right = new node(0, nullptr, nullptr, fake.left);
        fake.left->right->valid = std::make_shared<bool>(true);
        valid_nodes.insert(std::make_pair(fake.left->right, true));
        copy(static_cast<node*>(fake.left->right), static_cast<node*>(other.fake.left->right));
      }
      if (other.fake.left->left != nullptr) {
        fake.left->left = new node(0, nullptr, nullptr, fake.left);
        fake.left->left->valid = std::make_shared<bool>(true);
        valid_nodes.insert(std::make_pair(fake.left->left, true));
        copy(static_cast<node*>(fake.left->left), static_cast<node*>(other.fake.left->left));
      }
    }
  }

  void copy(node* in, node* out) {
    in->val = out->val;
    if (out->left != nullptr) {
      in->left = new node(0, nullptr, nullptr, in);
      in->left->valid = std::make_shared<bool>(true);
      valid_nodes.insert(std::make_pair(in->left, true));
      copy(static_cast<node*>(in->left), static_cast<node*>(out->left));
    }
    if (out->right != nullptr) {
      in->right = new node(0, nullptr, nullptr, in);
      in->right->valid = std::make_shared<bool>(true);
      valid_nodes.insert(std::make_pair(in->right, true));
      copy(static_cast<node*>(in->right), static_cast<node*>(out->right));
    }
  }

  // O(n) strong
  set& operator=(const set& other) {
    set tmp(other);
    swap(tmp, *this);
    return *this;
  }

  // O(n) nothrow
  ~set() noexcept {
    clear();
  }

  // O(n) nothrow
  void clear() noexcept {
    size_set = 0;
    clearing(fake.left);
    fake.left = nullptr;
  }

  void clearing(basenode* nd) {
    if (nd == nullptr) {
      return;
    }
    clearing(nd->right);
    clearing(nd->left);
    if (valid_nodes.contains(nd)) {
      valid_nodes[nd] = false;
    }
    delete static_cast<node*>(nd);
  }

  // O(1) nothrow
  size_t size() const noexcept {
    return size_set;
  }

  // O(1) nothrow
  bool empty() const noexcept {
    return size_set == 0;
  }

  // nothrow
  const_iterator begin() const noexcept {
    if (fake.left != nullptr) {
      return const_iterator(find_min(fake.left), this, true);
    } else {
      return const_iterator(&fake, this, true);
    }
  }

  // nothrow
  const_iterator end() const noexcept {
    return const_iterator(&fake, this, false);
  }

  // nothrow
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  // nothrow
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  // O(h) strong
  std::pair<iterator, bool> insert(const T& el) {
    basenode** cur = &(fake.left);
    basenode* prev = &fake;
    while (*cur != nullptr) {
      prev = *cur;
      if (el < (static_cast<node*>(*cur))->val) {
        cur = &((*cur)->left);
      } else if (static_cast<node*>(*cur)->val < el) {
        cur = &((*cur)->right);
      } else {
        return {iterator(*cur, this, false), false};
      }
    }
    node* new_node = new node(el, nullptr, nullptr, prev);
    try {
      new_node->valid = std::make_shared<bool>(true);
    } catch (...) {
      delete new_node;
      throw;
    }
    try {
      valid_nodes.emplace(std::make_pair(new_node, true));
    } catch (...) {
      new_node->valid.reset();
      delete new_node;
      throw;
    }

    *cur = new_node;
    size_set++;
    return {iterator(new_node, this, false), true};
  }

  // O(h) nothrow
  iterator erase(const_iterator pos) {
    iterator it = erasing(pos);
    if (it.ptr != nullptr) {
      size_set--;
    }
    return it;
  }

  // O(h) strong
  size_t erase(const T& val) {
    const_iterator it = find(val);
    if (it == end()) {
      return 0;
    } else {
      erase(it);
      return 1;
    }
  }

  void nulling_node(basenode* nd) {
    nd->right = nullptr;
    nd->left = nullptr;
    nd->parent = nullptr;
  }

  iterator erasing(const_iterator pos) {
    if (pos.ptr == nullptr) {
      std::abort();
    }

    if (&fake != pos.finding_fake()) {
      std::abort();
    }

    iterator res(pos.ptr, this, false);
    ++res;
    if (pos.ptr->right == nullptr && pos.ptr->left == nullptr) {
      basenode* it = pos.ptr;
      it = find_parent_max(it);
      if (it != nullptr) {
        if (pos.ptr->parent->right == pos.ptr) {
          pos.ptr->parent->right = nullptr;
        } else {
          pos.ptr->parent->left = nullptr;
        }
      } else {
        fake.left = nullptr;
      }
      nulling_node(pos.ptr);
      valid_nodes[pos.ptr] = false;
      delete static_cast<node*>(pos.ptr);
    } else if (pos.ptr->right == nullptr || pos.ptr->left == nullptr) {
      basenode* child;
      if (pos.ptr->right == nullptr) {
        child = pos.ptr->left;
      } else {
        child = pos.ptr->right;
      }
      change_leafs(pos.ptr, child);
      nulling_node(pos.ptr);
      valid_nodes[pos.ptr] = false;
      delete static_cast<node*>(pos.ptr);
    } else {
      basenode* tmp = pos.ptr->right;
      tmp = find_min(tmp);
      change_leafs(tmp, tmp->right);
      swap_links(tmp, pos.ptr);
      if (tmp->parent == nullptr) {
        fake.left = tmp;
      }
      nulling_node(pos.ptr);
      valid_nodes[pos.ptr] = false;
      delete static_cast<node*>(pos.ptr);
    }
    return res;
  }

  void change_leafs(basenode* from, basenode* to) {
    if (from->parent == nullptr) {
      fake.left = to;
    } else {
      if (from->parent->right == from) {
        from->parent->right = to;
        if (to != nullptr) {
          to->parent = from->parent;
        }
      } else {
        from->parent->left = to;
        if (to != nullptr) {
          to->parent = from->parent;
        }
      }
    }
    from->right = nullptr;
    from->parent = nullptr;
    from->left = nullptr;
  }

  void swap_links(basenode* first, basenode* second) {
    basenode* tmp = first->left;
    first->left = second->left;
    second->left = tmp;
    if (first->left != nullptr) {
      first->left->parent = first;
    }
    if (second->left != nullptr) {
      second->left->parent = second;
    }
    tmp = first->right;
    first->right = second->right;
    second->right = tmp;
    if (first->right != nullptr) {
      first->right->parent = first;
    }
    if (second->right != nullptr) {
      second->right->parent = second;
    }
    tmp = first->parent;
    first->parent = second->parent;
    second->parent = tmp;
    if (first->parent != nullptr) {
      if (first->parent->right == second) {
        first->parent->right = first;
      } else {
        first->parent->left = first;
      }
    }
    if (second->parent != nullptr) {
      if (second->parent->right == first) {
        second->parent->right = first;
      } else {
        second->parent->left = first;
      }
    }
  }

  // O(h) strong
  const_iterator lower_bound(const T& val) const {
    return l_bounding(val, fake.left, &fake);
  }

  const_iterator l_bounding(const T& val, basenode* nd, basenode* res) const {
    if (nd == nullptr) {
      return const_iterator(res, this, false);
    } else if (static_cast<node*>(nd)->val >= val) {
      res = nd;
      return l_bounding(val, nd->left, res);
    } else {
      return l_bounding(val, nd->right, res);
    }
  }

  // O(h) strong
  const_iterator upper_bound(const T& val) const {
    return u_bounding(val, fake.left, &fake);
  }

  const_iterator u_bounding(const T& val, basenode* nd, basenode* res) const {
    if (nd == nullptr) {
      return const_iterator(res, this, false);
    } else if (static_cast<node*>(nd)->val > val) {
      res = nd;
      return u_bounding(val, nd->left, res);
    } else {
      return u_bounding(val, nd->right, res);
    }
  }

  // O(h) strong
  iterator find(const T& val) const {
    return finding(val, fake.left);
  }

  iterator finding(const T& val, basenode* nd) const {
    if (nd == nullptr) {
      return iterator(&fake, this, false);
    }
    if (val < static_cast<node*>(nd)->val) {
      return finding(val, nd->left);
    } else if (val > static_cast<node*>(nd)->val) {
      return finding(val, nd->right);
    } else {
      return iterator(nd, this, false);
    }
  }

  // O(1) nothrow
  friend void swap(set& a, set& b) noexcept {
    std::swap(a.size_set, b.size_set);
    std::swap(a.valid_nodes, b.valid_nodes);
    if (a.fake.left != nullptr) {
      a.fake.left->parent = &b.fake;
    }
    if (b.fake.left != nullptr) {
      b.fake.left->parent = &a.fake;
    }
    std::swap(a.fake.left, b.fake.left);
  }

  friend basenode* find_min(basenode* cur) noexcept {
    return (cur->left == nullptr) ? cur : find_min(cur->left);
  }

  friend basenode* find_max(basenode* cur) noexcept {
    return (cur->right == nullptr) ? cur : find_max(cur->right);
  }

  static basenode* next(basenode* cur) noexcept {
    if (cur->right != nullptr) {
      return find_min(cur->right);
    }
    while (cur->parent != nullptr && cur->parent->left != cur) {
      cur = cur->parent;
    }
    return cur->parent;
  }

  static basenode* prev(basenode* cur) noexcept {
    if (cur->left != nullptr) {
      return find_max(cur->left);
    }
    while (cur->parent != nullptr && cur->parent->right != cur) {
      cur = cur->parent;
    }
    return cur->parent;
  }

  friend basenode* find_parent_max(basenode* nd) {
    if (nd->parent == nullptr) {
      return nullptr;
    }
    while (nd->parent->left != nd) {
      nd = nd->parent;
      if (nd->parent == nullptr) {
        return nullptr;
      }
    }
    return nd->parent;
  }

  friend basenode* find_parent_min(basenode* nd) {
    if (nd->parent == nullptr) {
      return nullptr;
    }
    while (nd->parent->right != nd) {
      nd = nd->parent;
      if (nd->parent == nullptr) {
        return nullptr;
      }
    }
    return nd->parent;
  }
};
