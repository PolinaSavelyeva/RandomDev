#pragma once

#include <linux/types.h> /* uint8_t and etc. types*/

struct p_ff_poly {
  uint8_t p_ff;
  uint8_t deg;     /* deg is always eq last non-zero index in ff_t */
  uint8_t *coeffs; /* big endian x^2 + 2x + 3 -> {1,2,3} */
};

typedef struct p_ff_poly *p_ff_poly_t;
typedef struct p_ff_poly *ff_t;
typedef struct p_ff_poly const *c_ff_t;

struct ff_elem {
  c_ff_t ff;
  uint8_t deg;
  uint8_t *coeffs;
};

typedef struct ff_elem *ff_elem_t;
typedef struct ff_elem const *c_ff_elem_t;

extern struct p_ff_poly ff_2_8;
extern struct p_ff_poly ff_2_16;
extern struct p_ff_poly ff_2_32;

extern const p_ff_poly_t p_ff_2_8;
extern const p_ff_poly_t p_ff_2_16;
extern const p_ff_poly_t p_ff_2_32;

ff_elem_t ff_copy(c_ff_elem_t elem);
uint8_t ff_is_zero(c_ff_elem_t elem);

ff_elem_t ff_get_zero(c_ff_t ff);
ff_elem_t ff_get_identity(c_ff_t ff);

uint8_t ff_are_eq(c_ff_t fst, c_ff_t snd);
uint8_t ff_elems_are_eq(c_ff_elem_t fst, c_ff_elem_t snd);

ff_elem_t ff_add(c_ff_elem_t fst, c_ff_elem_t snd);
ff_elem_t ff_sub(c_ff_elem_t fst, c_ff_elem_t snd);
ff_elem_t ff_mult(c_ff_elem_t fst, c_ff_elem_t snd);
ff_elem_t ff_div(c_ff_elem_t fst, c_ff_elem_t snd);
ff_elem_t ff_inv_add(c_ff_elem_t elem);
ff_elem_t ff_inv_mult(c_ff_elem_t elem);

void p_ff_poly_free(p_ff_poly_t poly);
void ff_elem_free(ff_elem_t fst);

ff_elem_t ff_2_8_init_elem(uint8_t coeffs);
ff_elem_t ff_2_16_init_elem(uint16_t coeffs);
ff_elem_t ff_2_32_init_elem(uint32_t coeffs);
