/*
 * Copyright (c) 2017 Mikey Austin <mikey@jackiemclean.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libstu/stu.h>
#include "test.h"

int
main(void)
{
    unsigned long nil_addr;

    TEST_START;

    TEST_OK((unsigned long) NIL == 0, "NIL is initially NULL");

    Stu *stu1 = Stu_new();
    TEST_OK((unsigned long) NIL != 0, "NIL is not NULL");
    nil_addr = (unsigned long) NIL;

    Stu *stu2 = Stu_new();
    TEST_OK((unsigned long) NIL != 0, "NIL is still not NULL");
    TEST_OK((unsigned long) NIL == nil_addr, "NIL was preserved");

    Stu_destroy(&stu1);
    TEST_OK((unsigned long) NIL == nil_addr, "NIL preserved after destroy");

    Stu_destroy(&stu2);
    TEST_OK((unsigned long) NIL == 0, "NIL has been cleaned up");

    TEST_FINISH;
}
