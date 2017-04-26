/*
 * Copyright (c) 2017 Raphael Sousa Santos <contact@raphaelss.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef NATIVE_FUNC_DEFINED
#define NATIVE_FUNC_DEFINED

#define SV_NATIVE_FUNC_DEFAULT 0x00
#define SV_NATIVE_FUNC_REST 0x01
#define SV_NATIVE_FUNC_PURE 0x02
#define SV_NATIVE_FUNC_MACRO 0x04

/* Forward declarations */
typedef struct Stu Stu;
typedef struct Env Env;
typedef struct Sv Sv;

typedef struct Sv_native_func Sv_native_func;
typedef struct Sv_native_closure Sv_native_closure;
typedef Sv *(*Sv_native_func_t)(Stu *, Env *, Sv **);

extern Sv_native_func *Sv_native_func_new(Stu *, Sv_native_func_t, unsigned, unsigned char);
extern Sv *Sv_native_func_call(Stu *,  Env *, Sv_native_func *, Sv *);
extern Sv *Sv_native_closure_call(Stu *, Env *, Sv_native_closure *, Sv *);
extern Sv *Sv_native_func_register(Stu *, const char *, Sv_native_func_t, unsigned, unsigned);
extern int Sv_native_func_is_macro(Sv_native_func *);

#endif
