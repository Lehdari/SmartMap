SmartMap
========

SmartMap is proof-of-concept heterogeneous associative container with dedicated smart pointers.
SmartMaps can store data of any type, which can be accessed with any key type allowed by an
`std` associative container. The data is accessed via smart pointers created for each key value.

Key design goals
----------------

- Heterogeneous storage
- Heterogeneous key types
- Fast data access via pointers
- Pointers can be copied and moved without them ever breaking
- SmartMaps can be copied and moved without them or their pointers ever breaking
    - After move, pointers point to the moved-to SmartMap
    - After copy, pointers point to the original SmartMap
    - Destroying a SmartMap invalidates all its pointers(invalidation can be checked)

Example
-------
```
SmartMap map1;
auto ptr1 = map1.getPointer<std::string, int>(1337);
*ptr1 = "foo";
auto ptr2 = map1.getPointer<std::string>(1337);
printf("%s\n", (*ptr2).c_str()); // prints foo

auto ptr3 = ptr1;
printf("%s\n", (*ptr3).c_str()); // prints foo

*ptr1 = "bar";
printf("%s\n", (*ptr2).c_str()); // prints bar
printf("%s\n", (*ptr3).c_str()); // prints bar

SmartMap map2 = std::move(map1);
auto ptr4 = map2.getPointer<std::string>(1337);
*ptr4 = "baz";
printf("%s\n", (*ptr1).c_str()); // prints baz
printf("%s\n", (*ptr2).c_str()); // prints baz
printf("%s\n", (*ptr3).c_str()); // prints baz
printf("%s\n", (*ptr4).c_str()); // prints baz
```
