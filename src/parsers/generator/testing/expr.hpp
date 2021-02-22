#ifndef PREZ_PARSERS_GENERATOR_TESTING_EXPR_HPP
#define PREZ_PARSERS_GENERATOR_TESTING_EXPR_HPP

enum BinOp { PLUS, MINUS, TIMES, DIVIDE };
enum UOp { NEG, BITNEG };

/* Expr */
struct Expr {
  virtual ~Expr(){};
  virtual int eval() const = 0;
};

struct EInt : Expr {
  EInt(int i) : i_(i) {}
  int eval() const override { return i_; }
  int i_;
};

struct EBinOp : Expr {
  EBinOp(Expr* e1, BinOp op, Expr* e2) : e1_(e1), e2_(e2), op_(op) {}
  ~EBinOp() {
    delete e1_;
    delete e2_;
  }
  int eval() const override {
    int x = e1_->eval();
    int y = e2_->eval();
    switch (op_) {
    case PLUS:
      return x + y;
    case MINUS:
      return x - y;
    case TIMES:
      return x * y;
    case DIVIDE:
      return x / y;
    }
  }

  Expr* e1_;
  Expr* e2_;
  BinOp op_;
};

struct EUOp : Expr {
  EUOp(Expr* e, UOp op) : e_(e), op_(op) {}
  ~EUOp() { delete e_; }
  int eval() const override {
    int x = e_->eval();
    switch (op_) {
    case NEG:
      return -x;
    case BITNEG:
      return ~x;
    }
  }

  Expr* e_;
  UOp op_;
};

#endif
