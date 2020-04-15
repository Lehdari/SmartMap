//
// Project: SmartMap
// File: SmartMap.hpp
//
// Copyright (c) 2020 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "SmartMap.hpp"
#include <iostream>
#include <string>
#include <cassert>


int test()
{
    // Test basic pointer access
    SmartMap c1;
    auto ptr_1_1 = c1.getPointer<std::string, std::string>("paavo");
    *ptr_1_1 = "koira";
    assert(*ptr_1_1 == "koira");

    // Test pointer access with the same key and string literal -> std::string deduction
    auto ptr_1_2 = c1.getPointer<std::string>("paavo");
    assert(*ptr_1_1 == *ptr_1_2);
    assert(*ptr_1_2 == "koira");

    // Test pointer sharedness (target of both pointers get changed)
    *ptr_1_2 = "kissa";
    assert(*ptr_1_1 == *ptr_1_2);
    assert(*ptr_1_1 == "kissa");
    assert(*ptr_1_2 == "kissa");

    // Test access with another key
    auto ptr_1_3 = c1.getPointer<std::string>("mikko");
    *ptr_1_3 = "koira";
    assert(*ptr_1_3 == "koira");
    assert(*ptr_1_1 == "kissa");
    assert(*ptr_1_2 == "kissa");

    // Test access with another type
    auto ptr_1_4 = c1.getPointer<int>("paavo");
    *ptr_1_4 = 10;
    assert(*ptr_1_1 == "kissa");

    // Test access to another container with same name
    SmartMap c2;
    auto ptr_2_1 = c2.getPointer<std::string>("paavo");
    *ptr_2_1 = "koira";
    assert(*ptr_2_1 == "koira");
    assert(*ptr_1_1 == "kissa");

    {
        // Test pointer copy constructor
        auto ptr_1_5 = ptr_1_1;
        *ptr_1_5 = "kala";
        assert(*ptr_1_5 == "kala");
        assert(*ptr_1_1 == "kala");

        // Test pointer copy assignment to empty pointer
        SmartMap::Pointer<std::string> ptr_1_6;
        ptr_1_6 = ptr_1_1;
        *ptr_1_6 = "pupu";
        assert(*ptr_1_6 == "pupu");
        assert(*ptr_1_1 == "pupu");

        // Test pointer copy assignment to existing pointer
        ptr_1_6 = ptr_1_3;
        assert(*ptr_1_6 == *ptr_1_3);

        // Test pointer move constructor
        auto ptr_1_7 = std::move(ptr_1_5);
        assert(*ptr_1_7 == "pupu");

        // Test pointer move assignment to empty pointer
        SmartMap::Pointer<std::string> ptr_1_8;
        ptr_1_8 = std::move(ptr_1_6);
        assert(*ptr_1_8 == "koira");

        // Test pointer destructor
    }
    auto ptr_1_5 = c1.getPointer<std::string>("liisa");
    *ptr_1_5 = "kissa";
    assert(*ptr_1_5 == "kissa");

    // Test SmartMap copy constructor
    SmartMap c3 = c1;
    auto ptr_3_1 = c3.getPointer<std::string>("paavo");
    assert(*ptr_3_1 == "pupu");
    assert(*ptr_1_1 == "pupu");
    *ptr_3_1 = "koira";
    assert(*ptr_3_1 == "koira");
    assert(*ptr_1_1 == "pupu");

    // Test SmartMap copy assignment
    SmartMap c4;
    c4 = c2;
    auto ptr_4_1 = c3.getPointer<std::string>("paavo");
    assert(*ptr_4_1 == "koira");
    assert(*ptr_2_1 == "koira");
    *ptr_4_1 = "possu";
    assert(*ptr_4_1 == "possu");
    assert(*ptr_2_1 == "koira");

    // Test SmartMap move constructor
    SmartMap c5 = std::move(c1);
    auto ptr_5_1 = c5.getPointer<std::string>("paavo");
    assert(*ptr_5_1 == "pupu");
    *ptr_5_1 = "kissa";
    assert(*ptr_1_1 == "kissa");

    // Test SmartMap move assignment
    SmartMap c6;
    c6 = std::move(c2);
    auto ptr_6_1 = c6.getPointer<std::string>("paavo");
    assert(*ptr_6_1 == "koira");
    *ptr_6_1 = "vuohi";
    assert(*ptr_2_1 == "vuohi");

    return 0;
}

int main()
{
    return test();
}
