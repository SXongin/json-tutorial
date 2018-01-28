#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <string.h>  /*strlen(), strncmp() */
#include <ctype.h>   /*isdigit() */
#include <errno.h>   /*errno */
#include <math.h>    /*HuGE_VAL */
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* target, lept_type type){
    size_t len = strlen(target);
    int result = strncmp(c->json, target, len);
    if(result != 0){
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += len;
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    const char* now;
    now = c->json;
    if(*now == '-'){
        ++now;
    }

    if(!isdigit(*now)){
        return LEPT_PARSE_INVALID_VALUE;
    }

    if(*now == '0'){
        ++now;
    }else{/* 1 - 9*/
        ++now;
        while(isdigit(*now)){
            ++now;
        }
    }

    if(*now == '.'){
        ++now;
        if(!isdigit(*now)){
            return LEPT_PARSE_INVALID_VALUE;
        }
        ++now;
        while(isdigit(*now)){
            ++now;
        }
    }

    if(*now == 'e' || *now == 'E'){
        ++now;
        if(*now == '-' || *now == '+'){
            ++now;
        }
        if(!isdigit(*now)){
            return LEPT_PARSE_INVALID_VALUE;
        }
        ++now;
        while(isdigit(*now)){
            ++now;
        }
    }

    if(!isspace(*now) && *now != '\0'){
        return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    errno = 0;
    v->n = strtod(c->json, &end);
    if((errno == ERANGE) &&( v->n == HUGE_VAL || -(v->n) == HUGE_VAL)){
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
