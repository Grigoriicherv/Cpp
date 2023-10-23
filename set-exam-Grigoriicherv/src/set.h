#pragma once

#include <cassert>
#include <iterator>
#include <utility>

template <typename T>
class set {
  struct node;
  struct basenode {
    basenode* right;
    basenode* left;
    basenode* parent;

    basenode() = default;
    basenode(basenode* left, basenode* right, basenode* parent) : left(left), right(right), parent(parent){}
    ~basenode() = default;
  };

  struct node : basenode {
    T val;

    node() = delete;
    node(T const& val, basenode* l = nullptr, basenode* r = nullptr, basenode* p = nullptr) : val(val), basenode(l,r,p){}
    ~node() = default;
  };

  template<typename R>
  struct my_iterator : std::iterator<std::bidirectional_iterator_tag, R> {
    friend struct set<T>;

    my_iterator() = default;

    my_iterator(std::nullptr_t) = delete;

    my_iterator(basenode* ptr) : ptr(ptr) {}

    template <typename Q>
    requires std::is_same_v<const Q, R>
    my_iterator(my_iterator<Q> const& other)
        : ptr(other.ptr) {}

    my_iterator& operator=(my_iterator const&) = default;


    my_iterator& operator=(my_iterator const& other) const{
      ptr = other.ptr;
    }

    ~my_iterator() = default;

    my_iterator& operator++() {
      ptr = next(static_cast<node*>(ptr));
      return *this;
    }


    my_iterator operator++(int) {
      my_iterator x = *this;
      ++*this;
      return x;
    }

    my_iterator& operator--() {
      ptr = prev(static_cast<node*>(ptr));
      return *this;
    }

    my_iterator operator--(int) {
      my_iterator x = *this;
      --*this;
      return x;
    }

    const R& operator*() const {
      return static_cast<node*>(ptr)->val;
    }

    const R* operator->() const {
      return &(static_cast<node*>(ptr)->val);
    }

    friend bool operator==(my_iterator const& a, my_iterator const& b) {
      return a.ptr == b.ptr;
    }

    friend bool operator!=(my_iterator const& a, my_iterator const& b) {
      return a.ptr != b.ptr;
    }

  private:
    basenode* ptr;
  };

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = my_iterator<T>;
  using const_iterator = my_iterator<T>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  mutable basenode fake;
  size_t size_set;



public:



  // O(1) nothrow
  set() : fake(), size_set(0) {}

  // O(n) strong
  set(const set& other) : set(){
    size_set = other.size_set;
    if (other.fake.left != nullptr) {
      fake.left = new node(static_cast<node*>(other.fake.left)->val, nullptr, nullptr, &fake);

      if (other.fake.left->right != nullptr) {
        fake.left->right = new node(0, nullptr, nullptr, fake.left);
        copy(static_cast<node*>(fake.left->right), static_cast<node*>(other.fake.left->right));
      }
      if (other.fake.left->left != nullptr) {
        fake.left->left = new node(0, nullptr, nullptr, fake.left);
        copy(static_cast<node*>(fake.left->left), static_cast<node*>(other.fake.left->left));
      }
    }
  }
  void copy(node* in, node* out){
    in->val = out->val;
    if (out->left != nullptr){
      in->left = new node(0, nullptr, nullptr, in);
      copy(static_cast<node*>(in->left), static_cast<node*>(out->left));
    }
    if (out->right != nullptr){
      in->right = new node(0, nullptr, nullptr, in);
      copy(static_cast<node*>(in->right), static_cast<node*>(out->right));
    }
  }

  // O(n) strong
  set& operator=(const set& other){
    set tmp(other);
    swap(tmp, *this);
    return *this;
  }


  // O(n) nothrow
  ~set() noexcept{
    clear();
  }

  //  O(n) nothrow
  void clear() noexcept{
    size_set = 0;
    clearing(fake.left);
    fake.left = nullptr;
  }
  void clearing(basenode* nd){
    if (nd == nullptr){
      return;
    }
    clearing(nd->right);
    clearing(nd->left);
    delete static_cast<node*>(nd);
  }

  // O(1) nothrow
  size_t size() const noexcept{
    return size_set;
  }

  // O(1) nothrow
  bool empty() const noexcept{
    return size_set == 0;
  }

  // nothrow
  const_iterator begin() const noexcept{
    if (fake.left != nullptr) {
      return const_iterator(find_min(fake.left));
    }else{
      return const_iterator(&fake);
    }
  }

  // nothrow
  const_iterator end() const noexcept{
    return const_iterator(&fake);
  }

  // nothrow
  const_reverse_iterator rbegin() const noexcept{
    return const_reverse_iterator(end());
  }

  // nothrow
  const_reverse_iterator rend() const noexcept{
    return const_reverse_iterator(begin());
  }

  // O(h) strong
  std::pair<iterator, bool> insert(T const& el) {
    basenode** cur = &(fake.left);
    basenode* prev = &fake;
    while (*cur != nullptr) {
      prev = *cur;
      if (el < (static_cast<node*>(*cur))->val) {
        cur = &((*cur)->left);
      } else if (static_cast<node*>(*cur)->val < el) {
        cur = &((*cur)->right);
      } else {
        return {iterator(*cur), false};
      }
    }
    node* new_node = new node(el, nullptr, nullptr, prev);
    *cur = new_node;
    size_set++;
    return {iterator(new_node), true};
  }

  // O(h) nothrow
  iterator erase(const_iterator pos){
    iterator it = erasing(pos);
    if (it.ptr != nullptr){
      size_set--;
    }
    return it;
  }

  // O(h) strong
  size_t erase(const T& val){
    const_iterator it = find(val);
    if (it == end()){
      return 0;
    }
    else{
      erase(it);
      return 1;
    }
  }

  iterator erasing(const_iterator pos){
    iterator res(pos.ptr);
    ++res;
    if (pos.ptr->right == nullptr && pos.ptr->left == nullptr){
      basenode* it = pos.ptr;
      it = find_parent_max(it);
      if (it != nullptr) {
        if (pos.ptr->parent->right == pos.ptr) {
          pos.ptr->parent->right = nullptr;
        } else {
          pos.ptr->parent->left = nullptr;
        }
      }
      else{
        fake.left = nullptr;
      }
      delete static_cast<node*>(pos.ptr);
    }
    else if (pos.ptr->right == nullptr || pos.ptr->left == nullptr){
      basenode* child;
      if (pos.ptr->right == nullptr){
        child = pos.ptr->left;
      }
      else{
        child = pos.ptr->right;
      }
      change_leafs(pos.ptr, child);
      delete static_cast<node*>(pos.ptr);
    }
    else{
      basenode* tmp = pos.ptr->right;
      tmp = find_min(tmp);
      change_leafs(tmp, tmp->right);
      swap_links(tmp, pos.ptr);
      if (tmp->parent == nullptr){
        fake.left = tmp;
      }
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
  void swap_links(basenode* first, basenode* second){
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
    if (first->right != nullptr){
      first->right->parent = first;
    }
    if (second->right != nullptr) {
      second->right->parent = second;
    }
    tmp = first->parent;
    first->parent = second->parent;
    second->parent = tmp;
    if (first->parent != nullptr) {
      if (first->parent->right == second){
        first->parent->right = first;
      }
      else{
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
  const_iterator lower_bound(const T& val) const{
    return l_bounding(val, fake.left, &fake);
  }
  const_iterator l_bounding(const T& val, basenode* nd, basenode* res) const{
    if (nd == nullptr){
      return const_iterator(res);
    }
    else if (static_cast<node*>(nd)->val >= val){
      res = nd;
      return l_bounding(val, nd->left, res);
    }
    if (static_cast<node*>(nd)->val < val){
      return l_bounding(val, nd->right, res);
    }
  }


  // O(h) strong
  const_iterator upper_bound(const T& val) const{
    return u_bounding(val, fake.left, &fake);
  }


  const_iterator u_bounding(const T& val, basenode* nd, basenode* res) const{
    if (nd == nullptr){
      return const_iterator(res);
    }
    else if (static_cast<node*>(nd)->val > val){
      res = nd;
      return u_bounding(val, nd->left, res);
    }
    if (static_cast<node*>(nd)->val <= val){
      return u_bounding(val, nd->right, res);
    }
  }

  // O(h) strong
  iterator find(const T& val){
    return finding(val, fake.left);
  }
  iterator finding(const T& val, basenode* nd){
    if (nd == nullptr){
      return iterator(&fake);
    }
    if (val < static_cast<node*>(nd)->val){
      return finding(val, nd->left);
    }
    else if (val > static_cast<node*>(nd)->val){
      return finding(val, nd->right);
    }
    else{
      return iterator(nd);
    }
  }

  // O(1) nothrow
  friend void swap(set& a, set& b) noexcept{
    std::swap(a.size_set, b.size_set);
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
  friend basenode* find_parent_max(basenode* nd){
    if (nd->parent == nullptr){
      return nullptr;
    }
    while(nd->parent->left != nd){
      nd = nd->parent;
      if (nd->parent == nullptr){
        return nullptr;
      }
    }
    return nd->parent;
  }
  friend basenode* find_parent_min(basenode* nd){
    if (nd->parent == nullptr){
      return nullptr;
    }
    while(nd->parent->right != nd){
      nd = nd->parent;
      if (nd->parent == nullptr){
        return nullptr;
      }
    }
    return nd->parent;
  }
};