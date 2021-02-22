#ifndef PREZ_PARSERS_GENERATOR_TESTING_NULLABLE_GRAMMAR_HPP
#define PREZ_PARSERS_GENERATOR_TESTING_NULLABLE_GRAMMAR_HPP

#include "src/parsers/generator/utils.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

/*
 * S -> tY | YCX
 * C -> CX | Y | b
 * X -> XX | a
 * Y -> ε | b | Yz
 */

/* Variables are positive */
// static constexpr int S = 0; // Defined in utils.hpp
static constexpr int C = 1;
static constexpr int X = 2;
static constexpr int Y = 3;

/* Tokens are negative */
static constexpr int t = -1;
static constexpr int b = -2;
static constexpr int a = -3;
static constexpr int z = -4;

/* Concrete classes of Symbols */
static constexpr int S1 = 0;
static constexpr int S2 = 1;
static constexpr int C1 = 2;
static constexpr int C2 = 3;
static constexpr int C3 = 4;
static constexpr int X1 = 5;
static constexpr int X2 = 6;
static constexpr int Y1 = 7;
static constexpr int Y2 = 8;
static constexpr int Y3 = 9;

GrammarData GRAMMAR_DATA = {
    /* tokens */ {
        {"t", "", NONE, Assoc::NONE, "", "", "", 0},
        {"b", "", NONE, Assoc::NONE, "", "", "", 0},
        {"a", "", NONE, Assoc::NONE, "", "", "", 0},
        {"z", "", NONE, Assoc::NONE, "", "", "", 0},
    },

    /* concretes */
    {
        {"S1", S, NONE, {t, Y}, "", 0},
        {"S2", S, NONE, {Y, C, X}, "", 0},
        {"C1", C, NONE, {C, X}, "", 0},
        {"C2", C, NONE, {Y}, "", 0},
        {"C3", C, NONE, {b}, "", 0},
        {"X1", X, NONE, {X, X}, "", 0},
        {"X2", X, NONE, {a}, "", 0},
        {"Y1", Y, NONE, {}, "", 0},
        {"Y2", Y, NONE, {b}, "", 0},
        {"Y3", Y, NONE, {Y, z}, "", 0},
    },

    /* variables */
    {
        {"S", "", {S1, S2}, "", 0},
        {"C", "", {C1, C2, C3}, "", 0},
        {"X",
         "",
         {
             X1,
             X2,
         },
         "",
         0},
        {"Y", "", {Y1, Y2, Y3}, "", 0},
    }};

#endif
