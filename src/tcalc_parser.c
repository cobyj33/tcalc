#include "tcalc_exprtree.h"

#include "tcalc_context.h"
#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_tokens.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*
Syntactical Description of the recursive descent tcalc parser

Comments are prepended by a hash (#) symbol

```
# lowest precedence, top of parse tree

expression -> term

logic_or -> logic_and ( '||' logic_and )*
logic_and -> equality ( '&&' equality )*
equality -> relation ( ( '=' | '==' | '!=' ) relation )*
relation -> term ( ( '<' | '<=' | '>' | '>=' ) term )*

term -> factor ( ( "+" | "-" ) factor )*
factor -> unary ( ( "*" | "/" | "%" ) unary )*

# Maybe having logical NOT in the 'unary' rule is weird? I don't know where else
# to put it though. Perhaps it could go above 'term', since arithmetic expressions
# cannot be acted on by the Logical NOT operator anyway unlike in C and C-like
# languages

unary -> ( "+" | "-" | "!" )* exponentiation

exponentiation -> primary ( ( "^" | "**" ) exponentiation )

primary -> variable | NUMBER | func | ( "(" expression ")" );
func -> funcname "(" funcargs? ")"
funcargs -> expression ( "," expression )*

variable -> IDENTIFIER
funcname -> IDENTIFIER

# highest precedence, bottom of parse tree
```
*/

typedef struct tcalc_pctx {
  const char* expr;
  const tcalc_ctx* ctx;
  tcalc_token* toks;
  size_t nb_toks;
  size_t i;
} tcalc_pctx;

typedef tcalc_err (tcalc_parsefunc_func_t)(tcalc_pctx*, tcalc_exprtree**);

static tcalc_err tcalc_parsefunc_expression(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_logic_or(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_logic_and(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_equality(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_relation(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_term(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_factor(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_exponentiation(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_unary(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_primary(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_func(tcalc_pctx* pctx, tcalc_exprtree** out);

static tcalc_err tcalc_exprtree_node_alloc_binary(const tcalc_token token, tcalc_exprtree* left, tcalc_exprtree* right, tcalc_exprtree** out);
static tcalc_err tcalc_exprtree_node_alloc_unary(const tcalc_token token, tcalc_exprtree* child, tcalc_exprtree** out);
static tcalc_err tcalc_exprtree_node_alloc_value(const tcalc_token token, tcalc_exprtree** out);

/**
 *
 * @param parent the expression tree node to attach parameter arguments to.
 *
 * The arity of the parsed function arguments is expected to be equal to the
 *
*/
static tcalc_err tcalc_parsefunc_funcargs(tcalc_pctx* pctx, tcalc_exprtree* parent);

/**
 * General Pipeline:
 *
 * Convert infix expression into infix tokens
*/

// expression rule
tcalc_err tcalc_create_exprtree_infix(const char* expr, const tcalc_ctx* ctx, tcalc_exprtree** out) {
  assert(out != NULL);
  *out = NULL;
  tcalc_err err = TCALC_ERR_OK;

  tcalc_pctx pctx = { .expr = expr, .ctx = ctx, .i = 0, .toks = NULL, .nb_toks = 0 };
  ret_on_err(err, tcalc_tokenize_infix(expr, &(pctx.toks), &(pctx.nb_toks)));

  err = tcalc_parsefunc_expression(&pctx, out);
  free(pctx.toks);
  return err;
}

/**
 * @param operators a NULL-terminated array of NUL-terminated operator strings to match.
*/
static bool tcalc_lbstr_in_ntntstrs(const char* token_str, size_t tokenLen, const char** nt_ntstrs);

static bool tcalc_pctx_should_insert_implicit_mult(const tcalc_pctx* pctx);
static bool tcalc_pctx_isnexttype(const tcalc_pctx* pctx, tcalc_token_type type);
static bool tcalc_pctx_is_curr_tok_in_optlist(const tcalc_pctx* pctx, const char** nt_ntstr_operators);
static tcalc_err tcalc_parsefunc_binops_leftassoc(tcalc_pctx* pctx,  const char** operators, tcalc_parsefunc_func_t higher_prec_parsefunc, tcalc_exprtree** out);



/**
 * General function for parsing grammar rules for infix binary operators
 * with the grammar "higher_precedence_nonterminal binary_operators higher_precedence_nonterminal"
 *
 * @param operators a NULL-terminated array of operator strings to match.
*/
static tcalc_err tcalc_parsefunc_binops_leftassoc(tcalc_pctx* pctx, const char** operators, tcalc_parsefunc_func_t higher_prec_parsefunc, tcalc_exprtree** out) {
  tcalc_err err = TCALC_ERR_OK;
  tcalc_exprtree* left = NULL;
  cleanup_on_err(err, higher_prec_parsefunc(pctx, &left));

  while (tcalc_pctx_is_curr_tok_in_optlist(pctx, operators)) {
    const tcalc_token operator = pctx->toks[pctx->i];
    pctx->i++; // consume current operator
    cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_MALFORMED_BINEXP);

    tcalc_exprtree *right = NULL;
    cleanup_on_err(err, higher_prec_parsefunc(pctx, &right));

    tcalc_exprtree* temp = NULL;
    if (tc_failed(err, tcalc_exprtree_node_alloc_binary(operator, left, right, &temp))) {
      tcalc_exprtree_free(right);
      goto cleanup;
    }

    left = temp;
  }

  *out = left;

  return TCALC_ERR_OK;
  cleanup:
    *out = NULL;
    tcalc_exprtree_free(left);
    return err;
}

static tcalc_err tcalc_parsefunc_expression(tcalc_pctx* pctx, tcalc_exprtree** out) {
  return tcalc_parsefunc_logic_or(pctx, out);
}

static tcalc_err tcalc_parsefunc_logic_or(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "||", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_logic_and, out);
}

static tcalc_err tcalc_parsefunc_logic_and(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "&&", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_equality, out);
}

static tcalc_err tcalc_parsefunc_equality(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "=", "==", "!=", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_relation, out);
}

static tcalc_err tcalc_parsefunc_relation(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "<", "<=", ">", ">=", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_term, out);
}

static tcalc_err tcalc_parsefunc_term(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "+", "-", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_factor, out);
}

static tcalc_err tcalc_parsefunc_factor(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "*", "", "/", "%", NULL };
  tcalc_exprtree* left = NULL;
  tcalc_err err = TCALC_ERR_OK;

  cleanup_on_err(err, tcalc_parsefunc_unary(pctx, &left));

  while (tcalc_pctx_is_curr_tok_in_optlist(pctx, operators) || tcalc_pctx_should_insert_implicit_mult(pctx)) {
    const tcalc_token operator = tcalc_pctx_should_insert_implicit_mult(pctx) ?
      (struct tcalc_token){ .type = TCALC_TOK_BINOP, .start = pctx->toks[pctx->i].start, .xend = pctx->toks[pctx->i].start } :
      pctx->toks[pctx->i++];
    // ! only consumes current token if we did not insert an implicit multiplication

    cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_MALFORMED_BINEXP);

    tcalc_exprtree *right = NULL;
    cleanup_on_err(err, tcalc_parsefunc_unary(pctx, &right));

    tcalc_exprtree* temp = NULL;
    if (tc_failed(err, tcalc_exprtree_node_alloc_binary(operator, left, right, &temp))) {
      tcalc_exprtree_free(right);
      goto cleanup;
    }

    left = temp;
  }

  *out = left;

  return TCALC_ERR_OK;
  cleanup:
    *out = NULL;
    tcalc_exprtree_free(left);
    return err;
}

// unary -> ( "+" | "-" | "!" )* exponentiation
static tcalc_err tcalc_parsefunc_unary(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "+", "-", "!", NULL };
  tcalc_err err = TCALC_ERR_OK;
  tcalc_exprtree* unaryhead = NULL, *unarytail = NULL; //linked-list-like structure
  tcalc_exprtree* primary = NULL;

  while (tcalc_pctx_is_curr_tok_in_optlist(pctx, operators)) {
    const tcalc_token operator = pctx->toks[pctx->i]; // non-owning
    pctx->i++; // consume current operator
    cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_MALFORMED_UNEXP);

    tcalc_exprtree* unarylistnode = NULL;
    cleanup_on_err(err, tcalc_exprtree_node_alloc_unary(operator, NULL, &unarylistnode));

    if (unarytail != NULL) {
      unarytail->as.unary.child = unarylistnode;
      unarytail = unarylistnode;
    } else {
      unarytail = unarylistnode;
      unaryhead = unarylistnode;
    }
  }

  cleanup_on_err(err, tcalc_parsefunc_exponentiation(pctx, &primary));

  if (unarytail != NULL) {
    unarytail->as.unary.child = primary;
  } else {
    unaryhead = primary;
  }

  *out = unaryhead;
  return TCALC_ERR_OK;

  cleanup:
    *out = NULL;
    tcalc_exprtree_free(unaryhead);
    return err;
}

// exponentiation -> primary ( ( "^" | "**" ) exponentiation )
static tcalc_err tcalc_parsefunc_exponentiation(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "^", "**", NULL };
  tcalc_err err = TCALC_ERR_OK;
  tcalc_exprtree* tree = NULL;

  cleanup_on_err(err, tcalc_parsefunc_primary(pctx, &tree));

  if (tcalc_pctx_is_curr_tok_in_optlist(pctx, operators)) {
    const tcalc_token operator = pctx->toks[pctx->i];
    pctx->i++;
    cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_MALFORMED_BINEXP);

    tcalc_exprtree* rst = NULL;
    cleanup_on_err(err, tcalc_parsefunc_exponentiation(pctx, &rst)); // right recursion

    tcalc_exprtree* temp = NULL;
    if (tc_failed(err, tcalc_exprtree_node_alloc_binary(operator, tree, rst, &temp))) {
      tcalc_exprtree_free(rst);
      goto cleanup;
    }

    tree = temp;
  }


  *out = tree;
  return err;

  cleanup:
    *out = NULL;
    tcalc_exprtree_free(tree);
    return err;
}

static tcalc_err tcalc_parsefunc_primary(tcalc_pctx* pctx, tcalc_exprtree** out) {
  tcalc_err err = TCALC_ERR_OK;
  tcalc_exprtree* node = NULL;

  // since this is the final main rule, other rules fallthrough to this rule
  // and we must bounds-check here.
  cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_MALFORMED_INPUT);

  if (pctx->toks[pctx->i].type == TCALC_TOK_NUM) { //number
    cleanup_on_err(err, tcalc_exprtree_node_alloc_value(pctx->toks[pctx->i], &node));
    pctx->i++; // consume number token
  } else if (pctx->toks[pctx->i].type == TCALC_TOK_GRPSTRT) { // parenthesized expression
    pctx->i++; // consume group start symbol
    cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &node));
    cleanup_if(err, !tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_UNBAL_GRPSYMS);
    pctx->i++; // consume group end symbol
  } else if (pctx->toks[pctx->i].type == TCALC_TOK_ID) { // variable or function
    if (tcalc_ctx_hasvar(pctx->ctx, pctx->expr + pctx->toks[pctx->i].start, tcalc_token_len(pctx->toks[pctx->i]))) { // variable
      cleanup_on_err(err, tcalc_exprtree_node_alloc_value(pctx->toks[pctx->i], &node));
      pctx->i++; // consume variable
    } else if (tcalc_ctx_hasfunc(pctx->ctx, pctx->expr + pctx->toks[pctx->i].start, tcalc_token_len(pctx->toks[pctx->i]))) { // function
      cleanup_on_err(err, tcalc_parsefunc_func(pctx, &node));
    } else {
      err = TCALC_ERR_UNKNOWN_ID;
      goto cleanup;
    }
  } else {
    err = TCALC_ERR_UNKNOWN_TOKEN;
    goto cleanup;
  }

  *out = node;
  return err;

  cleanup:
    *out = NULL;
    tcalc_exprtree_free(node);
    return err;
}

static tcalc_err tcalc_parsefunc_func(tcalc_pctx* pctx, tcalc_exprtree** out) {
  assert(pctx->i < pctx->nb_toks);
  assert(pctx->toks[pctx->i].type == TCALC_TOK_ID);
  tcalc_err err = TCALC_ERR_OK;
  tcalc_exprtree* funchead = NULL;

  if (tcalc_ctx_hasunfunc(pctx->ctx, pctx->expr + pctx->toks[pctx->i].start, tcalc_token_len(pctx->toks[pctx->i]))) {
    cleanup_on_err(err, tcalc_exprtree_node_alloc_unary(pctx->toks[pctx->i], NULL, &funchead));
    pctx->i++;
  } else if (tcalc_ctx_hasbinfunc(pctx->ctx, pctx->expr + pctx->toks[pctx->i].start, tcalc_token_len(pctx->toks[pctx->i]))) {
    cleanup_on_err(err, tcalc_exprtree_node_alloc_binary(pctx->toks[pctx->i], NULL, NULL, &funchead));
    pctx->i++;
  } else {
    err = TCALC_ERR_UNKNOWN_ID; // should be unreachable
    goto cleanup;
  }

  cleanup_if(err, !tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPSTRT), TCALC_ERR_UNCALLED_FUNC);
  pctx->i++; // consume opening parentheses

  cleanup_on_err(err, tcalc_parsefunc_funcargs(pctx, funchead));

  cleanup_if(err, !tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_UNCLOSED_FUNC);
  pctx->i++; // consume ending parentheses

  *out = funchead;
  return err;

  cleanup:
    *out = NULL;
    tcalc_exprtree_free(funchead);
    return err;
}

static tcalc_err tcalc_parsefunc_funcargs(tcalc_pctx* pctx, tcalc_exprtree* parent) {
  tcalc_err err = TCALC_ERR_OK;

  switch (parent->type) {
    case TCALC_EXPRTREE_NODE_TYPE_BINARY: {
      cleanup_if(err, tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_WRONG_ARITY);

      cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &(parent->as.binary.left) ));
      cleanup_if(err, !tcalc_pctx_isnexttype(pctx, TCALC_TOK_PSEP), TCALC_ERR_WRONG_ARITY);
      pctx->i++; // consume ',' parameter separator

      cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &(parent->as.binary.right) ));
      cleanup_if(err, tcalc_pctx_isnexttype(pctx, TCALC_TOK_PSEP), TCALC_ERR_WRONG_ARITY);

      return err;
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_UNARY: {
      cleanup_if(err, tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_WRONG_ARITY);
      cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &(parent->as.unary.child) ));
      cleanup_if(err, tcalc_pctx_isnexttype(pctx, TCALC_TOK_PSEP), TCALC_ERR_WRONG_ARITY);
      return err;
    } break;
    case TCALC_EXPRTREE_NODE_TYPE_VALUE: {
      // Invalid, assert on debug and return error on release
      assert(0 && "unreachable");
      return TCALC_ERR_INVALID_ARG;
    } break;
  }

  return err;

  cleanup:
    tcalc_exprtree_free_children(parent);
    return err;
}

static bool tcalc_pctx_isnexttype(const tcalc_pctx* pctx, tcalc_token_type type){
  return pctx->i < pctx->nb_toks && pctx->toks[pctx->i].type == type;
}

static bool tcalc_lbstr_in_ntntstrs(const char* s, size_t strl, const char** ntntstrs) {
  for (unsigned int i = 0; ntntstrs[i] != NULL; i++) {
    if (tcalc_streq_ntlb(ntntstrs[i], s, strl))
      return true;
  }
  return false;
}

static tcalc_err tcalc_exprtree_node_alloc_binary(const tcalc_token token, tcalc_exprtree* left, tcalc_exprtree* right, tcalc_exprtree** out) {
  *out = NULL;
  tcalc_exprtree* binary = calloc(1, sizeof(*binary));
  if (binary == NULL) return TCALC_ERR_BAD_ALLOC;

  binary->type = TCALC_EXPRTREE_NODE_TYPE_BINARY;
  binary->as.binary.token = token;
  binary->as.binary.left = left;
  binary->as.binary.right = right;
  *out = binary;
  return TCALC_ERR_OK;
}

static tcalc_err tcalc_exprtree_node_alloc_unary(const tcalc_token token, tcalc_exprtree* child, tcalc_exprtree** out)
{
  *out = NULL;
  tcalc_exprtree* unary = calloc(1, sizeof(*unary));
  if (unary == NULL) return TCALC_ERR_BAD_ALLOC;

  unary->type = TCALC_EXPRTREE_NODE_TYPE_UNARY;
  unary->as.unary.token = token;
  unary->as.unary.child = child;
  *out = unary;
  return TCALC_ERR_OK;
}

static tcalc_err tcalc_exprtree_node_alloc_value(const tcalc_token token, tcalc_exprtree** out)
{
  *out = NULL;
  tcalc_exprtree* value = calloc(1, sizeof(*value));
  if (value == NULL) return TCALC_ERR_BAD_ALLOC;

  value->type = TCALC_EXPRTREE_NODE_TYPE_VALUE;
  value->as.value.token = token;
  *out = value;
  return TCALC_ERR_OK;
}

static bool tcalc_pctx_is_curr_tok_in_optlist(const tcalc_pctx* pctx, const char** nt_ntstr_operators)
{
  return pctx->i < pctx->nb_toks && tcalc_lbstr_in_ntntstrs(pctx->expr + pctx->toks[pctx->i].start, tcalc_token_len(pctx->toks[pctx->i]), nt_ntstr_operators);
}

static bool tcalc_pctx_should_insert_implicit_mult(const tcalc_pctx* pctx)
{
  if (!(pctx->i < pctx->nb_toks && pctx->i > 0)) return false;

  const tcalc_token lastToken = pctx->toks[pctx->i - 1];
  const tcalc_token currToken = pctx->toks[pctx->i];
  return
      (
        lastToken.type == TCALC_TOK_NUM ||
        lastToken.type == TCALC_TOK_GRPEND ||
        (
          lastToken.type == TCALC_TOK_ID &&
          tcalc_ctx_hasvar(pctx->ctx, tcalc_token_startcp(pctx->expr, lastToken), tcalc_token_len(lastToken))
        )
      ) &&
      (
        currToken.type == TCALC_TOK_GRPSTRT ||
        currToken.type == TCALC_TOK_ID
      );
}