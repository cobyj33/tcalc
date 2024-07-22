#include "tcalc_exprtree.h"

#include "tcalc_context.h"
#include "tcalc_string.h"
#include "tcalc_func.h"
#include "tcalc_mem.h"
#include "tcalc_tokens.h"

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

term -> factor ( ( "+" | "-" ) factor )*
factor -> unary ( ( "*" | "/" | "%" ) unary )*

unary -> ( "+" | "-" )* exponentiation

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
  const tcalc_ctx* ctx;
  tcalc_token** toks;
  size_t nb_toks;
  size_t i;
} tcalc_pctx;

typedef tcalc_err (tcalc_parsefunc_func_t)(tcalc_pctx*, tcalc_exprtree**);

static tcalc_err tcalc_parsefunc_expression(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_term(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_factor(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_exponentiation(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_unary(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_primary(tcalc_pctx* pctx, tcalc_exprtree** out);
static tcalc_err tcalc_parsefunc_func(tcalc_pctx* pctx, tcalc_exprtree** out);

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
 * Convert infix tokens into rpn-formatted tokens
 * Convert rpn-formatted tokens into returned expression tree
*/

// expression rule
tcalc_err tcalc_create_exprtree_infix(const char* infix, const tcalc_ctx* ctx, tcalc_exprtree** out) {
  assert(out != NULL);
  *out = NULL;
  tcalc_err err = TCALC_ERR_OK;

  tcalc_pctx pctx;
  pctx.ctx = ctx;
  pctx.i = 0;
  pctx.toks = NULL;
  pctx.nb_toks = 0;
  ret_on_err(err, tcalc_tokenize_infix_ctx(infix, ctx, &(pctx.toks), &(pctx.nb_toks)));

  err = tcalc_parsefunc_expression(&pctx, out);
  TCALC_ARR_FREE_F(pctx.toks, pctx.nb_toks, tcalc_token_free);
  return err;
}

static int tcalc_pctx_isnextstr(tcalc_pctx* pctx, const char* str);
static int tcalc_pctx_isnexttype(tcalc_pctx* pctx, tcalc_token_type type);
static tcalc_err tcalc_parsefunc_binops_leftassoc(tcalc_pctx* pctx,  const char** operators, tcalc_parsefunc_func_t higher_prec_parsefunc, tcalc_exprtree** out);

/**
 * @param operators a NULL-terminated array of operator strings to match.
*/
static int tcalc_token_in_operator_list(tcalc_token* token, const char** operators);

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

  while (pctx->i < pctx->nb_toks && tcalc_token_in_operator_list(pctx->toks[pctx->i], operators)) {
    tcalc_token* operator = pctx->toks[pctx->i];
    pctx->i++; // consume current operator
    cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_MALFORMED_BINEXP);

    tcalc_exprtree *right = NULL;
    cleanup_on_err(err, higher_prec_parsefunc(pctx, &right));

    tcalc_exprtree* temp = NULL;
    if (tc_failed(err, tcalc_exprtree_node_alloc(operator, 2, &temp))) {
      // since right has not yet been added to the left subtree,  we would need
      // to free it before jumping to cleanup
      tcalc_exprtree_free(right);
      goto cleanup;
    }

    temp->children[0] = left;
    temp->children[1] = right;
    left = temp;
    temp = NULL;
  }

  *out = left;

  return TCALC_ERR_OK;
  cleanup:
    *out = NULL;
    tcalc_exprtree_free(left);
    return err;
}

static tcalc_err tcalc_parsefunc_expression(tcalc_pctx* pctx, tcalc_exprtree** out) {
  return tcalc_parsefunc_term(pctx, out);
}

static tcalc_err tcalc_parsefunc_term(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "+", "-", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_factor, out);
}

static tcalc_err tcalc_parsefunc_factor(tcalc_pctx* pctx, tcalc_exprtree** out) {
  const char* operators[] = { "*", "/", "%", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_unary, out);
}

// unary -> ( "+" | "-" )* exponentiation
static tcalc_err tcalc_parsefunc_unary(tcalc_pctx* pctx, tcalc_exprtree** out) {
  tcalc_err err = TCALC_ERR_OK;
  tcalc_exprtree* unaryhead = NULL, *unarytail = NULL; //linked-list-like structure
  tcalc_exprtree* primary = NULL;

  while (tcalc_pctx_isnextstr(pctx, "+") || tcalc_pctx_isnextstr(pctx, "-")) {
    tcalc_exprtree* unarylistnode = NULL;
    cleanup_on_err(err, tcalc_exprtree_node_alloc(pctx->toks[pctx->i], 1, &unarylistnode));

    if (unarytail != NULL) {
      unarytail->children[0] = unarylistnode;
      unarytail = unarylistnode;
    } else {
      unarytail = unarylistnode;
      unaryhead = unarylistnode;
    }
    pctx->i++;
  }

  cleanup_on_err(err, tcalc_parsefunc_exponentiation(pctx, &primary));

  if (unarytail != NULL) {
    unarytail->children[0] = primary;
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

  if (pctx->i < pctx->nb_toks && tcalc_token_in_operator_list(pctx->toks[pctx->i], operators)) {
    tcalc_token* operator = pctx->toks[pctx->i];
    pctx->i++;
    cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_MALFORMED_BINEXP);

    tcalc_exprtree* rst = NULL;
    cleanup_on_err(err, tcalc_parsefunc_exponentiation(pctx, &rst)); // right recursion

    tcalc_exprtree* temp = NULL;
    if (tc_failed(err, tcalc_exprtree_node_alloc(operator, 2, &temp))) {
      tcalc_exprtree_free(rst);
      goto cleanup;
    }

    temp->children[0] = tree;
    temp->children[1] = rst;
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

  if (pctx->toks[pctx->i]->type == TCALC_TOK_NUM) { //number
    cleanup_on_err(err, tcalc_exprtree_node_alloc(pctx->toks[pctx->i], 0, &node));
    pctx->i++; // consume number token
  } else if (pctx->toks[pctx->i]->type == TCALC_TOK_GRPSTRT) { // parenthesized expression
    pctx->i++; // consume group start symbol
    cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &node));

    cleanup_if(err, !tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_UNBAL_GRPSYMS);
    pctx->i++; // consume group end symbol
  } else if (pctx->toks[pctx->i]->type == TCALC_TOK_ID) { // variable or function
    if (tcalc_ctx_hasvar(pctx->ctx, pctx->toks[pctx->i]->val)) { // variable
      cleanup_on_err(err, tcalc_exprtree_node_alloc(pctx->toks[pctx->i], 0, &node));
      pctx->i++; // consume variable
    } else if (tcalc_ctx_hasfunc(pctx->ctx, pctx->toks[pctx->i]->val)) { // function
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
  assert(pctx->toks[pctx->i]->type == TCALC_TOK_ID);
  tcalc_err err = TCALC_ERR_OK;
  tcalc_exprtree* funchead = NULL;

  size_t arity = 0;
  if (tcalc_ctx_hasunfunc(pctx->ctx, pctx->toks[pctx->i]->val)) arity = 1;
  else if (tcalc_ctx_hasbinfunc(pctx->ctx, pctx->toks[pctx->i]->val)) arity = 2;
  else {
    err = TCALC_ERR_UNKNOWN_ID; // should be unreachable
    goto cleanup;
  }

  cleanup_on_err(err, tcalc_exprtree_node_alloc(pctx->toks[pctx->i], arity, &funchead));
  pctx->i++; // consume function name

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

  if (parent->nb_children > 0) {
    cleanup_if(err, tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_WRONG_ARITY);
    cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &(parent->children[0])));

    size_t childi = 1;
    while (childi < parent->nb_children && tcalc_pctx_isnexttype(pctx, TCALC_TOK_PSEP)) {
      pctx->i++; // consume separator
      cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &(parent->children[childi])) );
      childi++;
    }

    cleanup_if(err, pctx->i >= pctx->nb_toks, TCALC_ERR_UNCLOSED_FUNC);
    cleanup_if(err, childi < parent->nb_children, TCALC_ERR_WRONG_ARITY);
    cleanup_if(err, childi == parent->nb_children && !tcalc_pctx_isnexttype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_WRONG_ARITY);
  }

  return err;

  cleanup:
    tcalc_exprtree_free_children(parent);
    return err;
}

static int tcalc_pctx_isnextstr(tcalc_pctx* pctx, const char* str) {
  return pctx->i < pctx->nb_toks && strcmp(pctx->toks[pctx->i]->val, str) == 0;
}

static int tcalc_pctx_isnexttype(tcalc_pctx* pctx, tcalc_token_type type){
  return pctx->i < pctx->nb_toks && pctx->toks[pctx->i]->type == type;
}

static int tcalc_token_in_operator_list(tcalc_token* token, const char** operators) {
  for (unsigned int i = 0; operators[i] != NULL; i++) {
    if (strcmp(token->val, operators[i]) == 0)
      return 1;
  }
  return 0;
}
