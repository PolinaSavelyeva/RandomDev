#include "ff.h"

#include <linux/slab.h> /* memory managment functions */

// x^8 + x^4 + x^3 + x^2 + 1
uint8_t irr_poly_2_8[] = {1, 0, 0, 0, 1, 1, 1, 0, 1};
struct p_ff_poly ff_2_8 = {.p_ff = 2, .deg = 8, .coeffs = irr_poly_2_8};

// x^16 + x^9 + x^8 + x^7 + x^6 + x^4 + x^3 + x^2 + 1
uint8_t irr_poly_2_16[] = {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1};
struct p_ff_poly ff_2_16 = {.p_ff = 2, .deg = 16, .coeffs = irr_poly_2_16};

// x^32 + x^22 + x^2 + x^1 + 1
uint8_t irr_poly_2_32[] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};
struct p_ff_poly ff_2_32 = {.p_ff = 2, .deg = 32, .coeffs = irr_poly_2_32};

const p_ff_poly_t p_ff_2_8 = &ff_2_8;
const p_ff_poly_t p_ff_2_16 = &ff_2_16;
const p_ff_poly_t p_ff_2_32 = &ff_2_32;

static void *xkmalloc(size_t n) {
  /* Uses flag GFP_USER or can uses GFP_KERNEL to allocate normal kernel ram
  https://archive.kernel.org/oldlinux/htmldocs/kernel-api/API-kmalloc.html
  Kmalloc allocates memory of size lesser than one page size (around 4kB) */
  void *res = kmalloc(n, GFP_USER);
  if (!res) return NULL;
  return res;
}

static void *xkcalloc(size_t nmemb, size_t size) {
  void *res = kcalloc(nmemb, size, GFP_USER);
  if (!res) return NULL;
  return res;
}

static ff_elem_t ff_kcalloc_elem(c_ff_t ff) {
  ff_elem_t elem = xkmalloc(sizeof(struct ff_elem));
  elem->ff = ff;
  elem->deg = ff->deg - 1;
  elem->coeffs = xkcalloc(ff->deg, 1);

  return elem;
}

ff_elem_t ff_get_zero(c_ff_t ff) { return ff_kcalloc_elem(ff); }

ff_elem_t ff_get_identity(c_ff_t ff) {
  ff_elem_t identity = ff_kcalloc_elem(ff);
  identity->coeffs[identity->deg] = 1;

  return identity;
}

uint8_t ff_are_eq(c_ff_t fst, c_ff_t snd) {
  return (fst->p_ff == snd->p_ff && fst->deg == snd->deg &&
          !memcmp(fst->coeffs, snd->coeffs, (fst->deg + 1)));
}

uint8_t ff_elems_are_eq(c_ff_elem_t fst, c_ff_elem_t snd) {
  return (ff_are_eq(fst->ff, snd->ff) && fst->deg == snd->deg &&
          !memcmp(fst->coeffs, snd->coeffs, fst->deg + 1));
}

ff_elem_t ff_add(c_ff_elem_t fst, c_ff_elem_t snd) {
  if (!ff_are_eq(fst->ff, snd->ff)) return NULL;

  ff_elem_t res = ff_kcalloc_elem(fst->ff);

  for (uint8_t i = 0; i < res->deg + 1; i++) {
    res->coeffs[i] = (fst->coeffs[i] + snd->coeffs[i]) % fst->ff->p_ff;
  }

  return res;
}

ff_elem_t ff_inv_add(c_ff_elem_t elem) {
  ff_elem_t res = ff_kcalloc_elem(elem->ff);

  for (uint8_t i = 0; i < res->deg + 1; i++) {
    res->coeffs[i] = (elem->ff->p_ff - elem->coeffs[i]) % elem->ff->p_ff;
  }

  return res;
}

void p_ff_poly_free(p_ff_poly_t poly) {
  kfree(poly->coeffs);
  kfree(poly);
}

void ff_elem_free(ff_elem_t fst) {
  kfree(fst->coeffs);
  kfree(fst);
}

ff_elem_t ff_sub(c_ff_elem_t fst, c_ff_elem_t snd) {
  if (!fst || !snd || !ff_are_eq(fst->ff, snd->ff)) return NULL;

  ff_elem_t inv_snd = ff_inv_add(snd);
  ff_elem_t res = ff_add(fst, inv_snd);
  ff_elem_free(inv_snd);

  return res;
}

/* Return the index of the first non-zero element of polynomial
   If all elements are 0 returns -1 */
static int real_deg(uint8_t deg, const uint8_t *coeffs) {
  for (uint8_t i = 0; i <= deg; i++) {
    if (coeffs[i]) return deg - i;
  }

  return -1;
}

static uint64_t uint64_pow(uint64_t base, uint64_t power) {
  uint64_t res = 1;

  while (power > 0) {
    if (power % 2) {
      res *= base;
    }
    power /= 2;
    base *= base;
  }
  return res;
}

uint8_t ff_is_zero(c_ff_elem_t elem) {
  return real_deg(elem->deg, elem->coeffs) == -1 ? 1 : 0;
}

ff_elem_t ff_mult(c_ff_elem_t fst, c_ff_elem_t snd) {
  if (!fst || !snd || !ff_are_eq(fst->ff, snd->ff)) return NULL;

  uint8_t mult_deg = 2 * fst->deg;
  uint8_t *coeffs = xkcalloc(mult_deg + 1, 1);

  for (uint8_t i = 0; i <= fst->deg; i++) {
    for (uint8_t j = 0; j <= snd->deg; j++) {
      coeffs[i + j] =
          (coeffs[i + j] + fst->coeffs[i] * snd->coeffs[j]) % fst->ff->p_ff;
    }
  }

  int real_deg_res = real_deg(mult_deg, coeffs);

  for (uint8_t i = mult_deg - real_deg_res; i <= mult_deg - fst->ff->deg; i++) {
    uint8_t q =
        (uint64_pow(fst->ff->coeffs[0], fst->ff->p_ff - 2) * fst->coeffs[i]) %
        fst->ff->p_ff;
    for (uint8_t j = 0; j <= fst->ff->deg; j++) {
      int16_t tmp =
          ((int16_t)coeffs[i + j] - (int16_t)(q * fst->ff->coeffs[j])) %
          fst->ff->p_ff;
      coeffs[i + j] = tmp >= 0 ? tmp : tmp + fst->ff->p_ff;
    }
  }

  ff_elem_t res = ff_kcalloc_elem(fst->ff);
  memcpy(res->coeffs, coeffs + fst->deg, fst->deg + 1);
  kfree(coeffs);

  return res;
}

ff_elem_t ff_copy(c_ff_elem_t elem) {
  ff_elem_t res = ff_kcalloc_elem(elem->ff);
  memcpy(res->coeffs, elem->coeffs, res->deg + 1);

  return res;
}

static ff_elem_t ff_elem_pow(c_ff_elem_t base, uint64_t power) {
  ff_elem_t tmp_base = ff_copy(base);
  ff_elem_t res = ff_get_identity(base->ff);

  while (power > 0) {
    if (power % 2) {
      ff_elem_t tmp = ff_mult(res, tmp_base);
      ff_elem_free(res);
      res = tmp;
    }
    power /= 2;
    ff_elem_t tmp = ff_mult(tmp_base, tmp_base);
    ff_elem_free(tmp_base);
    tmp_base = tmp;
  }
  ff_elem_free(tmp_base);

  return res;
}

ff_elem_t ff_inv_mult(c_ff_elem_t elem) {
  if (!elem || ff_is_zero(elem)) return NULL;

  return ff_elem_pow(elem, uint64_pow(elem->ff->p_ff, elem->ff->deg) - 2);
}

ff_elem_t ff_div(c_ff_elem_t fst, c_ff_elem_t snd) {
  if (!fst || !snd || !ff_are_eq(fst->ff, snd->ff)) return NULL;

  ff_elem_t snd_inv = ff_inv_mult(snd);
  ff_elem_t res = snd_inv ? ff_mult(fst, snd_inv) : NULL;

  ff_elem_free(snd_inv);

  return res;
}

ff_elem_t ff_2_8_init_elem(uint8_t coeffs) {
  ff_elem_t res = ff_kcalloc_elem(&ff_2_8);

  uint8_t i = res->deg;
  while (coeffs) {
    res->coeffs[i] = coeffs % 2;
    coeffs /= 2;
    i--;
  }

  return res;
}

ff_elem_t ff_2_16_init_elem(uint16_t coeffs) {
  ff_elem_t res = ff_kcalloc_elem(&ff_2_16);

  uint8_t i = res->deg;
  while (coeffs) {
    res->coeffs[i] = coeffs % 2;
    coeffs /= 2;
    i--;
  }

  return res;
}

ff_elem_t ff_2_32_init_elem(uint32_t coeffs) {
  ff_elem_t res = ff_kcalloc_elem(&ff_2_32);

  uint8_t i = res->deg;
  while (coeffs) {
    res->coeffs[i] = coeffs % 2;
    coeffs /= 2;
    i--;
  }

  return res;
}
