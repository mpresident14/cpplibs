#ifndef LR1_GRAMMAR_HPP
#define LR1_GRAMMAR_HPP

#include "src/generator/utils.hpp"

namespace test::expr_grammar {
/*
 * S    -> Expr
 * Expr -> INT
 *       | Expr PLUS Expr
 *       | Expr STAR Expr
 */

/* Variables are positive */
static constexpr int EXPR = 1;

/* ExprType classes of Symbols */
static constexpr int EINT = 1;
static constexpr int EPLUS = 2;
static constexpr int ETIMES = 3;

/* Tokens are negative */
static constexpr int INT = -1;
static constexpr int PLUS = -2;
static constexpr int STAR = -3;

GrammarData GRAMMAR_DATA = {
  /* tokens */ {
      { "INT", "int", NONE, Assoc::NONE, "stoi(std::string(#str))", "", "[1-9][0-9]*", 0 },
      { "PLUS", "", 1, Assoc::LEFT, "", "", "\\+", 0 },
      { "STAR", "", 2, Assoc::LEFT, "", "", "\\*", 0 },
      { "whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+", 0 },
  },

  /* concretes */
  {
      { "SCONC", S, NONE, { EXPR }, "Start(#0*)", 0 },
      { "EINT", EXPR, NONE, { INT }, "new EInt(#0)", 0 },
      { "EPLUS", EXPR, NONE, { EXPR, PLUS, EXPR }, "new EBinOp(#0, PLUS, #2)", 0 },
      { "ETIMES", EXPR, NONE, { EXPR, STAR, EXPR }, "new EBinOp(#0, TIMES, #2)", 0 },
  },

  /* variables */
  { { "S", "Start", { SCONC }, "", 0 },
    { "Expr", "Expr*", { EINT, EPLUS, ETIMES }, "delete #obj;", 0 } }
};

/* Will result in conflict because no operator precedence */
GrammarData BAD_GRAMMAR_DATA = {
  /* tokens */ {
      { "INT", "int", NONE, Assoc::NONE, "stoi(std::string(#str))", "", "[1-9][0-9]*", 0 },
      { "PLUS", "", NONE, Assoc::LEFT, "", "", "\\+", 0 },
      { "STAR", "", NONE, Assoc::LEFT, "", "", "\\*", 0 },
      { "whitespace", "", SKIP_TOKEN, Assoc::NONE, "", "", "[ \t\n]+", 0 },
  },

  /* concretes */
  {
      { "SCONC", S, NONE, { EXPR }, "Start(#0*)", 0 },
      { "EINT", EXPR, NONE, { INT }, "new EInt(#0)", 0 },
      { "EPLUS", EXPR, NONE, { EXPR, PLUS, EXPR }, "new EBinOp(#0, PLUS, #2)", 0 },
      { "ETIMES", EXPR, NONE, { EXPR, STAR, EXPR }, "new EBinOp(#0, TIMES, #2)", 0 },
  },

  /* variables */
  { { "S", "Start", { SCONC }, "", 0 },
    { "Expr", "Expr*", { EINT, EPLUS, ETIMES }, "delete #obj;", 0 } }
};

}  // namespace test::expr_grammar
#endif
