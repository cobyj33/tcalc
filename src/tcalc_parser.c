#include "tcalc.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

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
  int32_t exprLen;
  tcalc_token* toks;
  int32_t toksLen;
  int32_t i;
  tcalc_exprtree *tree;
  int32_t treeLen;
  int32_t treeCap;
} tcalc_pctx;

typedef tcalc_err (tcalc_parsefunc_func_t)(tcalc_pctx*, int32_t*);

static tcalc_err tcalc_parsefunc_expression(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_logic_or(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_logic_and(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_equality(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_relation(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_term(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_factor(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_exponentiation(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_unary(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_primary(tcalc_pctx* pctx, int32_t* outTreeInd);
static tcalc_err tcalc_parsefunc_func(tcalc_pctx* pctx, int32_t* outTreeInd);

static tcalc_err tcalc_pctx_alloc_node(tcalc_pctx *pctx, int32_t *outTreeInd);

tcalc_err tcalc_lex_parse(
  const char* expr, int32_t exprLen, tcalc_token *tokenBuffer,
  int32_t tokenBufferCapacity, tcalc_exprtree *treeBuffer,
  int32_t treeBufferCapacity, int32_t *outTokenCount, int32_t *outTreeNodeCount,
  int32_t *outExprRootInd
) {
  *outTokenCount = 0;
  *outTreeNodeCount = 0;
  *outExprRootInd = 0;
  tcalc_err err = TCALC_ERR_OK;

  int32_t tokenCount = 0, treeNodeCount = 0, exprRootInd = 0;

  err = tcalc_tokenize_infix(
    expr, exprLen, tokenBuffer, tokenBufferCapacity, &tokenCount
  );
  if (err) return err;

  err = tcalc_create_exprtree_infix(
    expr, exprLen, tokenBuffer, tokenCount, treeBuffer,
    treeBufferCapacity, &treeNodeCount, &exprRootInd
  );
  if (err) return err;

  *outTokenCount = tokenCount;
  *outTreeNodeCount = treeNodeCount;
  *outExprRootInd = exprRootInd;
  return err;
}

tcalc_err tcalc_create_exprtree_infix(
  const char* expr, int32_t exprLen, tcalc_token *tokens, int32_t tokensLen,
  tcalc_exprtree *destBuffer, int32_t destCapacity, int32_t* outDestLength,
  int32_t* outExprRootInd
) {
  *outExprRootInd = -1;
  *outDestLength = -1;
  tcalc_err err = TCALC_ERR_OK;

  tcalc_pctx pctx = {
    .expr = expr,
    .exprLen = exprLen,
    .toks = tokens,
    .toksLen = tokensLen,
    .tree = destBuffer,
    .treeCap = destCapacity,
    .treeLen = 0
  };

  err = tcalc_parsefunc_expression(&pctx, outExprRootInd);

  if (err == TCALC_ERR_OK && pctx.i < pctx.toksLen)
  {
    err = TCALC_ERR_UNPROCESSED_INPUT;
    tcalc_errstkaddf(
      __func__,
      "Failed to process all input "
      "(processed %" PRId32 " tokens of %" PRId32 " total tokens)",
      pctx.i,
      pctx.toksLen
    );
  }

  *outDestLength = pctx.treeLen;
  return err;
}

static bool tcalc_lbstr_in_ntntstrs(
  const char* token_str, int32_t tokenLen, const char** nt_ntstrs
);

static bool tcalc_pctx_should_insert_implicit_mult(const tcalc_pctx* pctx);
static bool tcalc_pctx_iscurrtype(const tcalc_pctx* pctx, tcalc_token_type type);
static bool tcalc_pctx_is_curr_tok_in_optlist(
  const tcalc_pctx* pctx, const char** nt_ntstr_operators
);

static tcalc_err tcalc_parsefunc_binops_leftassoc(
  tcalc_pctx* pctx, const char** operators,
  tcalc_parsefunc_func_t higher_prec_parsefunc, int32_t *outTreeInd
);


/**
 * General function for parsing grammar rules for infix binary operators
 * with the grammar "higher_precedence_nonterminal binary_operators higher_precedence_nonterminal"
 *
 * @param operators a NULL-terminated array of operator strings to match.
*/
static tcalc_err tcalc_parsefunc_binops_leftassoc(
  tcalc_pctx* pctx, const char** operators,
  tcalc_parsefunc_func_t higher_prec_parsefunc, int32_t *outTreeInd
) {
  *outTreeInd = -1;
  tcalc_err err = TCALC_ERR_OK;
  const int32_t savedTreeLen = pctx->treeLen;
  int32_t leftTreeInd = -1;
  cleanup_on_err(err, higher_prec_parsefunc(pctx, &leftTreeInd));

  while (tcalc_pctx_is_curr_tok_in_optlist(pctx, operators)) {
    int32_t operatorInd = pctx->i;
    pctx->i++; // consume current operator
    cleanup_if(err, pctx->i >= pctx->toksLen, TCALC_ERR_MALFORMED_BINEXP);

    int32_t rightTreeInd = -1;
    cleanup_on_err(err, higher_prec_parsefunc(pctx, &rightTreeInd));

    int32_t tempTreeInd = -1;
    cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &tempTreeInd));

    pctx->tree[tempTreeInd] = (tcalc_exprtree){
      .type = TCALC_EXPRTREE_NODE_TYPE_BINARY,
      .as = {
        .binary = {
          .tokenIndOImplMult = operatorInd,
          .leftTreeInd = leftTreeInd,
          .rightTreeInd = rightTreeInd
        }
      }
    };

    leftTreeInd = tempTreeInd;
  }

  *outTreeInd = leftTreeInd;
  return TCALC_ERR_OK;

  cleanup:
    pctx->treeLen = savedTreeLen;
    return err;
}

static tcalc_err tcalc_parsefunc_expression(tcalc_pctx* pctx, int32_t *outTreeInd) {
  return tcalc_parsefunc_logic_or(pctx, outTreeInd);
}

static tcalc_err tcalc_parsefunc_logic_or(tcalc_pctx* pctx, int32_t *outTreeInd) {
  const char* operators[] = { "||", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_logic_and, outTreeInd);
}

static tcalc_err tcalc_parsefunc_logic_and(tcalc_pctx* pctx, int32_t *outTreeInd) {
  const char* operators[] = { "&&", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_equality, outTreeInd);
}

static tcalc_err tcalc_parsefunc_equality(tcalc_pctx* pctx, int32_t *outTreeInd) {
  const char* operators[] = { "=", "==", "!=", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_relation, outTreeInd);
}

static tcalc_err tcalc_parsefunc_relation(tcalc_pctx* pctx, int32_t *outTreeInd) {
  const char* operators[] = { "<", "<=", ">", ">=", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_term, outTreeInd);
}

static tcalc_err tcalc_parsefunc_term(tcalc_pctx* pctx, int32_t *outTreeInd) {
  const char* operators[] = { "+", "-", NULL };
  return tcalc_parsefunc_binops_leftassoc(pctx, operators, tcalc_parsefunc_factor, outTreeInd);
}

static tcalc_err tcalc_parsefunc_factor(tcalc_pctx* pctx, int32_t *outTreeInd) {
  *outTreeInd = -1;
  const int32_t savedTreeLen = pctx->treeLen;
  const char* operators[] = { "*", "", "/", "%", NULL };
  int32_t leftTreeInd = -1;
  tcalc_err err = TCALC_ERR_OK;
  cleanup_on_err(err, tcalc_parsefunc_unary(pctx, &leftTreeInd));

  while ( tcalc_pctx_is_curr_tok_in_optlist(pctx, operators) ||
          tcalc_pctx_should_insert_implicit_mult(pctx)) {
    const int32_t operatorIndOImplMult =
      tcalc_pctx_should_insert_implicit_mult(pctx) ?  -(pctx->i) : pctx->i++;
    // ! only consumes current token if we did not insert an implicit multiplication
    cleanup_if(err, pctx->i >= pctx->toksLen, TCALC_ERR_MALFORMED_BINEXP);

    int32_t rightTreeInd = -1;
    cleanup_on_err(err, tcalc_parsefunc_unary(pctx, &rightTreeInd));

    int32_t tempTreeInd = -1;
    cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &tempTreeInd));
    pctx->tree[tempTreeInd] = (tcalc_exprtree){
      .type = TCALC_EXPRTREE_NODE_TYPE_BINARY,
      .as = {
        .binary = {
          .tokenIndOImplMult = operatorIndOImplMult,
          .leftTreeInd = leftTreeInd,
          .rightTreeInd = rightTreeInd
        }
      }
    };

    leftTreeInd = tempTreeInd;
  }

  *outTreeInd = leftTreeInd;
  return TCALC_ERR_OK;

  cleanup:
    pctx->treeLen = savedTreeLen;
    return err;
}

// unary -> ( "+" | "-" | "!" )* exponentiation
static tcalc_err tcalc_parsefunc_unary(tcalc_pctx* pctx, int32_t *outTreeInd) {
  const char* operators[] = { "+", "-", "!", NULL };
  *outTreeInd = -1;
  const int32_t savedTreeLen = pctx->treeLen;

  tcalc_err err = TCALC_ERR_OK;
  int32_t unaryHeadInd = -1; // linked-list like structure
  int32_t unaryTailInd = -1;
  int32_t primaryTreeInd = -1;

  while (tcalc_pctx_is_curr_tok_in_optlist(pctx, operators)) {
    const int32_t operatorInd = pctx->i; // non-owning
    pctx->i++; // consume current operator
    cleanup_if(err, pctx->i >= pctx->toksLen, TCALC_ERR_MALFORMED_UNEXP);

    int32_t unaryNodeInd = -1;
    cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &unaryNodeInd));
    pctx->tree[unaryNodeInd] = (tcalc_exprtree){
      .type = TCALC_EXPRTREE_NODE_TYPE_UNARY,
      .as = { .unary = { .tokenInd = operatorInd, .childTreeInd = -1 }
      }
    };

    if (unaryTailInd >= 0) {
      pctx->tree[unaryTailInd].as.unary.childTreeInd = unaryNodeInd;
      unaryTailInd = unaryNodeInd;
    } else {
      unaryTailInd = unaryNodeInd;
      unaryHeadInd = unaryNodeInd;
    }
  }

  cleanup_on_err(err, tcalc_parsefunc_exponentiation(pctx, &primaryTreeInd));

  if (unaryTailInd >= 0) {
    pctx->tree[unaryTailInd].as.unary.childTreeInd = primaryTreeInd;
  } else {
    unaryHeadInd = primaryTreeInd;
  }

  *outTreeInd = unaryHeadInd;
  return TCALC_ERR_OK;

  cleanup:
    pctx->treeLen = savedTreeLen;
    return err;
}

// exponentiation -> primary ( ( "^" | "**" ) exponentiation )
static tcalc_err tcalc_parsefunc_exponentiation(tcalc_pctx* pctx, int32_t *outTreeInd) {
  const char* operators[] = { "^", "**", NULL };
  *outTreeInd = -1;
  const int32_t savedTreeLen = pctx->treeLen;
  tcalc_err err = TCALC_ERR_OK;
  int32_t treeInd = -1;

  cleanup_on_err(err, tcalc_parsefunc_primary(pctx, &treeInd));

  // note that we use an **if** here instead of a **while** like other cases
  if (tcalc_pctx_is_curr_tok_in_optlist(pctx, operators)) {
    const int32_t operatorInd = pctx->i;
    pctx->i++;
    cleanup_if(err, pctx->i >= pctx->toksLen, TCALC_ERR_MALFORMED_BINEXP);

    int32_t rstTreeInd = -1;
    cleanup_on_err(err, tcalc_parsefunc_exponentiation(pctx, &rstTreeInd)); // right recursion

    int32_t tempTreeInd = -1;
    cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &tempTreeInd));
    pctx->tree[tempTreeInd] = (tcalc_exprtree){
      .type = TCALC_EXPRTREE_NODE_TYPE_BINARY,
      .as = {
        .binary = {
          .tokenIndOImplMult = operatorInd,
          .leftTreeInd = treeInd,
          .rightTreeInd = rstTreeInd
        }
      }
    };

    treeInd = tempTreeInd;
  }


  *outTreeInd = treeInd;
  return err;

  cleanup:
    pctx->treeLen = savedTreeLen;
    return err;
}

static tcalc_err tcalc_parsefunc_primary(tcalc_pctx* pctx, int32_t *outTreeInd) {
  tcalc_err err = TCALC_ERR_OK;
  *outTreeInd = -1;
  const int32_t savedTreeLen = pctx->treeLen;
  int32_t nodeTreeInd = -1;

  // since this is the final main rule, other rules fallthrough to this rule
  // and we must bounds-check here.
  cleanup_if(err, pctx->i >= pctx->toksLen, TCALC_ERR_MALFORMED_INPUT);

  switch (pctx->toks[pctx->i].type)
  {
    case TCALC_TOK_NUM:
    {
      cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &nodeTreeInd));
      pctx->tree[nodeTreeInd] = (tcalc_exprtree){
        .type = TCALC_EXPRTREE_NODE_TYPE_VALUE,
        .as = { .value = { .tokenInd = pctx->i } }
      };
      pctx->i++; // consume number token
    }
    break;
    case TCALC_TOK_GRPSTRT:
    {
      pctx->i++; // consume group start symbol
      cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &nodeTreeInd));
      cleanup_if(err, !tcalc_pctx_iscurrtype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_UNBAL_GRPSYMS);
      pctx->i++; // consume group end symbol
    }
    break;
    case TCALC_TOK_ID:
    {
      // assume that this ID represents a function if a '(' follows it.
      // Otherwise, assume that this ID represents a variable
      if (pctx->i + 1 < pctx->toksLen && pctx->toks[pctx->i + 1].type == TCALC_TOK_GRPSTRT)
      {
        cleanup_on_err(err, tcalc_parsefunc_func(pctx, &nodeTreeInd));
      }
      else
      {
        cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &nodeTreeInd));
        pctx->tree[nodeTreeInd] = (tcalc_exprtree){
          .type = TCALC_EXPRTREE_NODE_TYPE_VALUE,
          .as = { .value = { .tokenInd = pctx->i } }
        };
        pctx->i++; // consume identifier token
      }
    }
    break;
    default:
    {
      err = TCALC_ERR_UNKNOWN_TOKEN;
      goto cleanup;
    }
    break;
  }

  *outTreeInd = nodeTreeInd;
  return err;

  cleanup:
    pctx->treeLen = savedTreeLen;
    return err;
}

static tcalc_err tcalc_parsefunc_func(tcalc_pctx* pctx, int32_t *outTreeInd) {
  assert(pctx->i < pctx->toksLen);
  assert(pctx->toks[pctx->i].type == TCALC_TOK_ID);

  *outTreeInd = -1;
  const int32_t savedTreeLen = pctx->treeLen;

  tcalc_err err = TCALC_ERR_OK;

  int32_t funcTreeInd = -1;
  cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &funcTreeInd));
  pctx->tree[funcTreeInd] = (tcalc_exprtree){
    .type = TCALC_EXPRTREE_NODE_TYPE_FUNC,
    .as = { .func = { .tokenInd = pctx->i, .funcArgHeadInd = -1 } }
  };
  pctx->i++; // consume function identifier

  cleanup_if(err, !tcalc_pctx_iscurrtype(pctx, TCALC_TOK_GRPSTRT), TCALC_ERR_UNCALLED_FUNC);
  pctx->i++; // consume opening parentheses

  int argCount = 0;

  if (!tcalc_pctx_iscurrtype(pctx, TCALC_TOK_GRPEND))
  {
    cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &(pctx->tree[funcTreeInd].as.func.funcArgHeadInd)));
    int32_t headExprInd = -1;
    cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &headExprInd));
    pctx->tree[pctx->tree[funcTreeInd].as.func.funcArgHeadInd] = (tcalc_exprtree){
      .type = TCALC_EXPRTREE_NODE_TYPE_FUNCARG,
      .as = { .funcarg = { .exprInd = headExprInd, .nextArgInd = -1 } }
    };

    int32_t argListTailInd = pctx->tree[funcTreeInd].as.func.funcArgHeadInd;

    while (tcalc_pctx_iscurrtype(pctx, TCALC_TOK_PSEP))
    {
      pctx->i++; // consume parameter separator ','
      cleanup_if(err, pctx->i >= pctx->toksLen, TCALC_ERR_MALFORMED_FUNC);
      cleanup_if(err, argCount >= TCALC_MAX_FUNC_ARG_COUNT, TCALC_ERR_FUNC_TOO_MANY_ARGS);

      int32_t argListNodeInd = -1;
      cleanup_on_err(err, tcalc_pctx_alloc_node(pctx, &argListNodeInd));
      int32_t argExprInd = -1;
      cleanup_on_err(err, tcalc_parsefunc_expression(pctx, &argExprInd));
      pctx->tree[argListNodeInd] = (tcalc_exprtree){
        .type = TCALC_EXPRTREE_NODE_TYPE_FUNCARG,
        .as = { .funcarg = { .exprInd = argExprInd, .nextArgInd = -1 } }
      };

      pctx->tree[argListTailInd].as.funcarg.nextArgInd = argListNodeInd;
      argListTailInd = argListNodeInd;
      argCount++;
    }
  }

  cleanup_if(err, !tcalc_pctx_iscurrtype(pctx, TCALC_TOK_GRPEND), TCALC_ERR_UNCLOSED_FUNC);
  pctx->i++; // consume ending parentheses

  *outTreeInd = funcTreeInd;
  return err;

  cleanup:
    pctx->treeLen = savedTreeLen;
    return err;
}

static bool tcalc_pctx_iscurrtype(const tcalc_pctx* pctx, tcalc_token_type type){
  return pctx->i < pctx->toksLen && pctx->toks[pctx->i].type == type;
}

static bool tcalc_lbstr_in_ntntstrs(const char* s, int32_t strl, const char** ntntstrs) {
  int i = 0;
  while (ntntstrs[i] != NULL && !tcalc_streq_ntlb(ntntstrs[i], s, strl)) i++;
  return ntntstrs[i] != NULL;
}

static bool tcalc_pctx_is_curr_tok_in_optlist(const tcalc_pctx* pctx, const char** nt_ntstr_operators)
{
  return pctx->i < pctx->toksLen &&
    tcalc_lbstr_in_ntntstrs(
      tcalc_token_startcp(pctx->expr, pctx->toks[pctx->i]),
      tcalc_token_len(pctx->toks[pctx->i]),
      nt_ntstr_operators
    );
}

static bool tcalc_pctx_should_insert_implicit_mult(const tcalc_pctx* pctx)
{
  if (!(pctx->i < pctx->toksLen && pctx->i > 0))
    return false;

  const tcalc_token lastToken = pctx->toks[pctx->i - 1];
  const tcalc_token currToken = pctx->toks[pctx->i];
  return
      (
        lastToken.type == TCALC_TOK_NUM ||
        lastToken.type == TCALC_TOK_GRPEND
      ) &&
      (
        currToken.type == TCALC_TOK_GRPSTRT ||
        currToken.type == TCALC_TOK_ID
      );
}

static tcalc_err tcalc_pctx_alloc_node(tcalc_pctx *pctx, int32_t *outTreeInd)
{
  assert(pctx != NULL);
  assert(outTreeInd != NULL);

  *outTreeInd = -1;
  tcalc_err err = TCALC_ERR_NOMEM;
  if (pctx->treeLen < pctx->treeCap)
  {
    err = TCALC_ERR_OK;
    *outTreeInd = pctx->treeLen;
    pctx->treeLen++;
  }
  return err;
}
