// Copyright (C) 2015, Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "include/markable.hpp"
#include <cassert>
#include <utility>
#include <string>



#if defined AK_TOOLBOX_USING_BOOST
#include <boost/optional.hpp>
#endif

using namespace ak_toolkit;

template <typename T>
void ignore(T&&) {}

void test_value_ctor()
{
  {
    typedef markable< mark_int<int, -1> > opt_int;
    static_assert (sizeof(opt_int) == sizeof(int), "size waste");
    
    opt_int oi_, oiN1(-1), oi0(0), oi1(1);
    assert (!oi_.has_value());
    assert (!oiN1.has_value());
    assert ( oi0.has_value());
    assert ( oi1.has_value());
    
    assert (oi0.value() == 0);
    assert (oi1.value() == 1);
  }
  {
    typedef markable< mark_int<int, 0> > opt_int;
    static_assert (sizeof(opt_int) == sizeof(int), "size waste");
    opt_int oi_, oiN1(-1), oi0(0), oi1(1);
    assert (!oi_.has_value());
    assert ( oiN1.has_value());
    assert (!oi0.has_value());
    assert ( oi1.has_value());
    
    assert (oiN1.value() == -1);
    assert (oi1.value() == 1);
  }
}

struct string_marked_value : markable_type<std::string>
{
  static std::string marked_value() { return std::string("\0\0", 2); }
  static bool is_marked_value(const std::string& v) { return v == std::string("\0\0", 2); }
};

void test_string_traits()
{
  typedef markable<string_marked_value, class tag_X> opt_str;
  static_assert (sizeof(opt_str) == sizeof(std::string), "size waste");
  
  opt_str os_, os00(std::string("\0\0", 2)), os0(std::string("\0")), osA(std::string("A"));
  assert (!os_.has_value());
  assert (!os00.has_value());
  assert ( os0.has_value());
  assert ( osA.has_value());
}

struct string_in_pair_empty_val : markable_type< std::string, std::pair<bool, std::string> >
{
  static storage_type marked_value() { return storage_type(false, "anything"); }
  static bool is_marked_value(const storage_type& v) { return v.first == false; }
  
  static const value_type& access_value(const storage_type& v) { return v.second; }
  static storage_type store_value(const value_type& v) { return storage_type(true, v); }
  static storage_type store_value(value_type&& v) { return storage_type(true, std::move(v)); }
};

void test_custom_storage()
{
  typedef markable<string_in_pair_empty_val> opt_str;
  opt_str os_, os0(std::string("\0")), osA(std::string("A"));
  
  assert (!os_.has_value());
  
  assert (os0.has_value());
  assert (os0.value() == "");
  
  assert (osA.has_value());
  assert (osA.value() == "A");

  swap (os_, os0);
    
  assert (os_.has_value());
  assert (os_.value() == "");
  assert (!os0.has_value());
}

void test_bool_storage()
{
  typedef markable<mark_bool> opt_bool;
  static_assert (sizeof(opt_bool) == 1, "size waste");
  
  opt_bool ob_, obT(true), obF(false);
  assert (!ob_.has_value());
  
  assert (obT.has_value());
  assert (obT.value() == true);
  
  assert (obF.has_value());
  assert (obF.value() == false);
}

void test_storage_value()
{
  typedef markable< mark_int<int, -1> > opt_int;
  opt_int oi_, oiN1(-1), oi0(0), oi1(1);
  
  assert ( oi_.storage_value() == -1);
  assert (oiN1.storage_value() == -1);
  assert ( oi0.storage_value() ==  0);
  assert ( oi1.storage_value() ==  1);
}

void test_mark_fp_nan()
{
  typedef markable<mark_fp_nan<double>> opt_double;
  opt_double o_, o1 (1.0), oNan (0.0/0.0);
  assert (!o_.has_value());
  assert ( o1.has_value());
  assert (!oNan.has_value());
  
  assert (o1.value() == 1.0);
  
  double v = o_.storage_value();
  assert (v != v);
  
  v = o1.storage_value();
  assert (v == 1.0);
  assert (v == v);
  
  v = oNan.storage_value();
  assert (v != v);
}

void test_mark_value_init()
{
  {
    typedef markable<mark_value_init<int>> opt_t;
    opt_t o_, o1 (1), oE(0);
    
    assert (!o_.has_value());
    assert ( o1.has_value());
    assert (!oE.has_value());
    
    assert (o1.value() == 1);
    
    assert (o_.storage_value() == 0);
    assert (o1.storage_value() == 1);
    assert (oE.storage_value() == 0);
  }
  {
    typedef markable<mark_value_init<std::string>> opt_t;
    opt_t o_, o1 (std::string("one")), oE ((std::string()));
    
    assert (!o_.has_value());
    assert ( o1.has_value());
    assert (!oE.has_value());
    
    assert (o1.value() == "one");
    
    assert (o_.storage_value() == "");
    assert (o1.storage_value() == "one");
    assert (oE.storage_value() == "");
  }
}

int objects_created = 0;
int objects_destroyed = 0;
int objects_created_with_value = 0;

void reset_globals()
{
  objects_created = 0;
  objects_destroyed = 0;
  objects_created_with_value = 0;
}

struct Cont
{
  bool empty_;
  explicit Cont(bool e = true) : empty_(e) { ++objects_created_with_value; }
  bool empty() const { return empty_; }
  bool operator==(const Cont& rhs) const { return empty_ == rhs.empty_; }
};

void test_mark_stl_empty()
{
  reset_globals();
  {
    typedef markable<mark_stl_empty<Cont>> opt_t;
    opt_t o_, o1 (Cont(false)), oE(Cont(true));
    
    assert (objects_created_with_value == 3);
    assert (!o_.has_value());
    assert ( o1.has_value());
    assert (!oE.has_value());
    assert (objects_created_with_value == 3);
       
    assert (o_.storage_value() == Cont(true));
    assert (o1.storage_value() == Cont(false));
    assert (oE.storage_value() == Cont(true));
  }
  {
    typedef markable<mark_stl_empty<std::string>> opt_t;
    opt_t o_, o1 (std::string("one")), oE ((std::string()));
    
    assert (!o_.has_value());
    assert ( o1.has_value());
    assert (!oE.has_value());
    
    assert (o1.value() == "one");
    
    assert (o_.storage_value() == "");
    assert (o1.storage_value() == "one");
    assert (oE.storage_value() == "");
  }
}

class minutes_since_midnight
{
  int minutes_;
  
public:
  bool invariant() const { return minutes_ >= 0 && minutes_ < 24 * 60; }
  
  explicit minutes_since_midnight(int minutes) 
  : minutes_(minutes) 
  {
    assert (invariant());
    ++objects_created;
  } 
  
  minutes_since_midnight(minutes_since_midnight const& rhs)
  : minutes_(rhs.minutes_)
  {
    assert (invariant());
    assert (rhs.invariant());
    ++objects_created;
  }
  
  int as_int() const
  {
    assert (invariant());
    return minutes_;
  }
  
  friend bool operator==(const minutes_since_midnight& l, const minutes_since_midnight& r)
  {
    return l.minutes_ == r.minutes_;
  }
  
  ~minutes_since_midnight()
  {
    assert (invariant());
    ++objects_destroyed;
  }
};

struct mark_minutes : markable_pod_storage_type<minutes_since_midnight, int>
{
  static storage_type marked_value() { return -1; }
  static bool is_marked_value(const storage_type& v) { return v == -1; }
};

void test_mark_raw_storage()
{
  reset_globals();
  {
    typedef markable<mark_minutes> opt_time;
    const minutes_since_midnight t0(0), tM(1439);
    opt_time ot_, ot0 (t0), otM(tM);
    assert (!ot_.has_value());
    assert ( ot0.has_value());
    assert ( otM.has_value());
    assert(objects_created == 4);
    
    assert (ot0.value() == t0);
    assert (otM.value() == tM);
    
    opt_time otM2 = otM;
    assert (otM.has_value());
    assert (otM.value() == tM);
    assert(objects_created == 5);
    
    ot_ = otM2;
    assert (ot_.has_value());
    assert (ot_.value().as_int() == 1439);
    assert(objects_created == 6);
    assert(objects_destroyed == 0);
    
    ot_ = {};
    assert (!ot_.has_value());
    assert ( otM.has_value());
    assert(objects_created == 6);
    assert(objects_destroyed == 1);
    
    swap(ot_, otM);
    assert ( ot_.has_value());
    assert (!otM.has_value());
    assert (ot_.value().as_int() == 1439);
    assert(objects_created == 7);
    assert(objects_destroyed == 2);
  }
  assert(objects_created == objects_destroyed);
}

enum class Dir { N, E, S, W };

void test_mark_enum()
{
  typedef markable<mark_enum<Dir, -1>> opt_dir;
  opt_dir o_, oN(Dir::N), oW(Dir::W);
  
  assert (!o_.has_value());
  assert ( oN.has_value());
  assert ( oW.has_value());
  
  assert (oN.value() == Dir::N);
  assert (oW.value() == Dir::W);
  
  assert (o_.storage_value() == -1);
  assert (oN.storage_value() ==  0);
  assert (oW.storage_value() ==  3);
}

#if defined AK_TOOLBOX_USING_BOOST
void test_optional_as_storage()
{
  typedef markable<mark_optional<boost::optional<int>>> opt_int;
  opt_int oi_, oiN1(-1), oi0(0);
  assert (!oi_.has_value());
  
  assert (oiN1.has_value());
  assert (oiN1.value() == -1);
  
  assert (oi0.has_value());
  assert (oi0.value() == 0);
}
#endif

int main()
{
  test_value_ctor();
  test_string_traits();
  test_custom_storage();
  test_bool_storage();
  test_storage_value();
  test_mark_fp_nan();
  test_mark_value_init();
  test_mark_stl_empty();
  test_mark_raw_storage();
#if defined AK_TOOLBOX_USING_BOOST
  test_optional_as_storage();
#endif
  test_mark_enum();
}
