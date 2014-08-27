/*
  A C++11 translation from Lisp of the simplified GPS program from the
  start of chapter 4 of Norvig's PAIP.

  by Richard Kelley
*/

#include <cstdio>
#include <list>
#include <string>
#include <algorithm>
#include <functional>

/*
  Type aliases introduced by the using keyword are similar to the
  typedefs you should be familiar with. They have some advantages in
  readability (particularly when working with functions as values), and
  when working with templates.

  The Lisp version of the program uses symbols to represent conditions.
  We can get away with using std::strings in this case.
*/
using Condition = std::string;

struct Op {
  std::string action;
  std::list<Condition> preconds;
  std::list<Condition> add_list;
  std::list<Condition> del_list;

  Op(std::string _action, 
     std::list<Condition> _preconds,
     std::list<Condition> _add_list,
     std::list<Condition> _del_list) : 
    action { _action },
    preconds { _preconds },
    add_list { _add_list },
    del_list { _del_list } { }

};

/*
  The std::function type is declared in the <functional> header. The
  type of a function that accepts an argument of type T and yields an
  argument of type S is S(T). The std::function is parametrized on its
  argument and return types, as you can see below.

  We're using a "lambda expression" here, which is how C++11 implements 
  anonymous functions. The [fn] part of the return value says that we're
  returning a _closure_: a function that captures the values of some of the
  variables in its environment. If we didn't want to capture anything, we'd
  just write [], but then we couldn't access the argument fn.

  If you understand closures, you should be able to explain the last
  sentence in the paragraph above. If you don't understand closures,
  you should learn about them, because I'll ask at least one question
  about this function on the first quiz.
 */
template<typename T>
std::function<bool(T)> complement(std::function<bool(T)> fn) {
  return [fn](T x) { return !fn(x); };
}

/*
  An Op is appropriate for a goal if the goal is in the Op's add_list.

  The function std::find is in the <algorithm> header. It returns an
  iterator to a position in the list it is given. That iterator either
  points to the position of the element you're looking for, or if the
  element is not in the list returns an iterator to the end of the
  list. So we can test for membership in a collection by comparing the
  result of std::find for equality with std::end.

  If you're new to C++11 iterators, you may also be surprised that
  we're not using std::end as a member function. This is new, and
  means that we can do things like get iterators into arrays if we
  need to.
 */
bool appropriate_p(Condition goal, Op op) {
  return std::end(op.add_list) != std::find(std::begin(op.add_list), std::end(op.add_list), goal);
}

/*
  If you haven't seen C++11's version of the "for each" concept, study
  the code below.

  Is there a more concise way to write this function? Is there a more
  concise way to do it _without modifying any of the inputs_?

  Also, is it odd that I'm returning a std::list by value? In C++11,
  this is very defensible. In C++98, it happens that this is often
  defensible. It would be wise to understand why it might not be a
  good idea, and why it could be.
*/
std::list<Op> find_all(Condition goal, std::list<Op> ops, std::function<bool(Condition, Op)> pred) {
  std::list<Op> res{};
  for (const auto& elt : ops) {
    if (pred(goal, elt)) {
      res.push_back(elt);
    }
  }
  return res;
}

std::list<Condition> current_state;
std::list<Op> current_operations;

/*
  The next two functions could be written in a much more general
  way. They should work with any container whose elements can be
  compared for equality...

  Again, I'm returning std::list objects. What's wrong (or not wrong)
  with this?
*/
std::list<Condition> set_diff(std::list<Condition> set1, std::list<Condition> set2) {
  std::list<Condition> res{};
  auto ending = std::end(set2);
  for(const auto& elt : set1) {
    if (std::find(std::begin(set2), std::end(set2), elt) == ending) {
      res.push_back(elt);
    }
  }
  return res;
}

std::list<Condition> set_union(std::list<Condition> set1, std::list<Condition> set2) {
  std::list<Condition> res{};

  for (const auto& elt : set1) {
    res.push_back(elt);
  }

  auto ending = std::end(set2);
  for (const auto& elt : set2) {
    if (std::find(std::begin(set2), std::end(set2), elt) == ending) {
      res.push_back(elt);
    }
  }
  return res;
}

/*
  The functions apply_op and achieve call each other, so one of them
  has to be declared before the other.

  The function std::all_of returns true if its third argument, a
  predicate, returns true for all elements in the container.
*/
bool achieve(Condition goal);
bool apply_op(Op op) {
  if (std::all_of(std::begin(op.preconds), std::end(op.preconds), achieve)) {
    std::printf("Executing operation: %s.\n", op.action.c_str());
    current_state = set_diff(current_state, op.del_list);
    current_state = set_union(current_state, op.add_list);
    return true;
  } else {
    return false;
  }
}

/*
  The function std::any_of returns true if its third argument, a
  predicate, returns true on at least one of the elements in the
  container.
*/
bool achieve(Condition goal) {
  bool left = std::end(current_state) != std::find(std::begin(current_state), std::end(current_state), goal);
  bool right = false;
  auto candidates = find_all(goal, current_operations, appropriate_p);
  if (std::any_of(std::begin(candidates), std::end(candidates), apply_op)) {
    right = true;
  }
  return left || right;
}

void GPS(std::list<Condition> state, std::list<Condition> goals, std::list<Op> ops) {
  if (std::all_of(std::begin(goals), std::end(goals), achieve)) {
    std::printf("SOLVED.\n");
  } else {
    std::printf("FAILED.\n");
  }
}

int main(int argc, char **argv) {
  // here's how complement works, even though we don't end up using it above.
  std::function<bool(int)> even_p = [](int n) { return (n % 2 == 0); };
  std::function<bool(int)> odd_p = complement(even_p);
  std::printf("2 is even: %d\n", !odd_p(2));


  // Here's the GPS code:

  /*
    The definitions below use the "uniform initialization"
    syntax. Except when dealing with a few std::initializer_list
    corner cases, this syntax should be preferred.
  */
  Condition son_at_home{"son-at-home"};
  Condition car_works{"car-works"};
  Condition son_at_school{"son-at-school"};
  Condition car_needs_battery{"car-needs-battery"};
  Condition shop_knows_problem{"shop-knows-problem"};
  Condition shop_has_money{"shop-has-money"};
  Condition know_phone_number{"know-phone-number"};
  Condition in_communication_with_shop{"in-communication-with-shop"};
  Condition have_phone_book{"have-phone-book"};
  Condition have_money{"have-money"};

  /*
    Uncomment one of these four lines to test the GPS function.

    We're using "uniform initialization" syntax to assign values to
    variables of type std::list<Condition>. The utility of this
    approach over that of C++98 should be obvious.
  */
  current_state = {son_at_home, car_needs_battery, have_money, have_phone_book};
  //current_state = {son_at_home, car_works};
  //current_state = {son_at_home};
  //current_state = {son_at_home, car_needs_battery, have_money};

  /*
    Uniform initialization and std::initializer_list show up again:
  */
  current_operations = {
    Op("drive-son-to-school", {son_at_home, car_works}, {son_at_school}, {son_at_home}),
    Op("shop-installs-battery", {car_needs_battery, shop_knows_problem, shop_has_money}, {car_works}, {}),
    Op("tell-shop-problem", {in_communication_with_shop}, {shop_knows_problem}, {}),
    Op("telephone-shop", {know_phone_number}, {in_communication_with_shop}, {}),
    Op("look-up-number", {have_phone_book}, {know_phone_number}, {}),
    Op("give-shop-money", {have_money}, {shop_has_money}, {have_money})
  };

  GPS(current_state, {son_at_school}, current_operations);

  return 0;
}
