#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "table.h"
#include "type.h"
#include "util.h"
#include "var.h"

static const char *table[] = {
  [EX_ADD] = "+",
  [EX_SUB] = "-",
  [EX_MUL] = "*",
  [EX_DIV] = "/",
  [EX_MOD] = "%",
  [EX_BITAND] = "&",
  [EX_BITOR] = "|",
  [EX_BITXOR] = "^",
  [EX_LSHIFT] = "<<",
  [EX_RSHIFT] = ">>",
  [EX_EQ] = "==",
  [EX_NE] = "!=",
  [EX_LT] = "<",
  [EX_LE] = "<=",
  [EX_GE] = ">=",
  [EX_GT] = ">",
  [EX_LOGAND] = "&&",
  [EX_LOGIOR] = "||",
  [EX_ASSIGN] = "=",
  [EX_COMMA] = ",",

  [EX_POS] = "+",
  [EX_NEG] = "-",

  [EX_BITNOT] = "~",  // ~x
  [EX_REF] = "&",     // &
  [EX_DEREF] = "*",   // *
  // [EX_CAST] = "",
  // [EX_MODIFY] = "",  // +=, etc.
};

static const char incdec[][3] = {"++", "--"};

void dump_expr(FILE *fp, Expr *expr) {
  switch (expr->kind) {
  case EX_FIXNUM:
    fprintf(fp, "%" PRId64, expr->fixnum);
    assert(expr->type->kind == TY_FIXNUM);
    if (expr->type->fixnum.is_unsigned)
      fputc('U', fp);
    switch (expr->type->fixnum.kind) {
    case FX_LONG:
      fputc('L', fp);
      break;
    case FX_LLONG:
      fputs("LL", fp);
      break;
    default: break;
    }
    break;
#ifndef __NO_FLONUM
  case EX_FLONUM:
    {
      char buf[64];
      snprintf(buf, sizeof(buf) - 4, "%g", expr->flonum);
      if (strchr(buf, '.') == NULL)
        strcat(buf, ".0");
      if (expr->type->flonum.kind == FL_FLOAT)
        strcat(buf, "f");
      fputs(buf, fp);
    }
    break;
#endif
  case EX_STR:
    {
      StringBuffer sb;
      sb_init(&sb);
      sb_append(&sb, "\"", NULL);
      escape_string(expr->str.buf, expr->str.size, &sb);
      sb_append(&sb, "\"", NULL);
      fputs(sb_to_string(&sb), fp);
    }
    break;
  case EX_VAR:
    fprintf(fp, "%.*s", expr->var.name->bytes, expr->var.name->chars);
    break;
  case EX_ADD:
  case EX_SUB:
  case EX_MUL:
  case EX_DIV:
  case EX_MOD:
  case EX_BITAND:
  case EX_BITOR:
  case EX_BITXOR:
  case EX_LSHIFT:
  case EX_RSHIFT:
  case EX_EQ:
  case EX_NE:
  case EX_LT:
  case EX_LE:
  case EX_GE:
  case EX_GT:
  case EX_LOGAND:
  case EX_LOGIOR:
  case EX_ASSIGN:
    {
      fprintf(fp, "(");
      dump_expr(fp, expr->bop.lhs);
      fprintf(fp, " %s ", table[expr->kind]);
      dump_expr(fp, expr->bop.rhs);
      fprintf(fp, ")");
    }
    break;
  case EX_COMMA:
    {
      fprintf(fp, "(");
      dump_expr(fp, expr->bop.lhs);
      fprintf(fp, ", ");
      dump_expr(fp, expr->bop.rhs);
      fprintf(fp, ")");
    }
    break;

  case EX_POS:
  case EX_NEG:
  case EX_BITNOT:
  case EX_REF:
  case EX_DEREF:
    fputs(table[expr->kind], fp);
    dump_expr(fp, expr->unary.sub);
    break;
  case EX_PREINC:
  case EX_PREDEC:
  case EX_POSTINC:
  case EX_POSTDEC:
    {
#define IS_POST(expr)  ((expr)->kind >= EX_POSTINC)
#define IS_DEC(expr)   (((expr)->kind - EX_PREINC) & 1)
      if (!IS_POST(expr))
        fputs(incdec[IS_DEC(expr)], fp);
      Expr *target = expr->unary.sub;
      if (target->kind != EX_VAR)
        fputc('(', fp);
      dump_expr(fp, target);
      if (target->kind != EX_VAR)
        fputc(')', fp);
      if (IS_POST(expr))
        fputs(incdec[IS_DEC(expr)], fp);
#undef IS_POST
#undef IS_DEC
    }
    break;
  case EX_CAST:
    fprintf(fp, "(");
    print_type(fp, expr->type);
    fprintf(fp, ")");
    dump_expr(fp, expr->unary.sub);
    break;
  case EX_MODIFY:
    {
      Expr *sub = expr->unary.sub;
      fprintf(fp, "(");
      dump_expr(fp, sub->bop.lhs);
      fprintf(fp, " %s= ", table[sub->kind]);
      dump_expr(fp, sub->bop.rhs);
      fprintf(fp, ")");
    }
    break;

  case EX_TERNARY:
    {
      fprintf(fp, "(");
      dump_expr(fp, expr->ternary.cond);
      fprintf(fp, " ? ");
      dump_expr(fp, expr->ternary.tval);
      fprintf(fp, " : ");
      dump_expr(fp, expr->ternary.fval);
      fprintf(fp, ")");
    }
    break;

  case EX_MEMBER:
    {
      dump_expr(fp, expr->member.target);
      fputs(expr->token->kind == TK_DOT ? "." : "->", fp);
      const Name *ident = expr->member.ident->ident;
      fprintf(fp, "%.*s", ident->bytes, ident->chars);
    }
    break;
  case EX_FUNCALL:
    {
      Expr *func = expr->funcall.func;
      Vector *args = expr->funcall.args;
      if (func->kind == EX_VAR) {
        fprintf(fp, "%.*s", func->var.name->bytes, func->var.name->chars);
      } else {
        fprintf(fp, "(");
        dump_expr(fp, func);
        fprintf(fp, ")");
      }
      if (args == NULL) {
        fprintf(fp, "()");
      } else {
        for (int i = 0; i < args->len; ++i) {
          fputs(i == 0 ? "(" : ", ", fp);
          dump_expr(fp, args->data[i]);
        }
        fprintf(fp, ")");
      }
    }
    break;
  case EX_COMPLIT:
  default:
    fprintf(stderr, "kind=%d, ", expr->kind);
    assert(!"not handled");
    break;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "dump_expr: [declarations] expr...\n");
    return 1;
  }

  init_lexer();
  init_global();

  Vector *toplevel = new_vector();
  int i = 1;
  if (i < argc - 1) {
    set_source_string(argv[i++], "*decl*", 1);
    parse(toplevel);
  }

  for (; i < argc; ++i) {
    char *source = argv[i];
    set_source_string(source, "*exp*", 1);

    Expr *expr = parse_expr();
    FILE *fp = stdout;
    fprintf(fp, "%s : ", source);
    print_type(fp, expr->type);
    fputs(" => ", fp);
    dump_expr(fp, expr);
    fputs("\n", fp);
  }

  return 0;
}
