#pragma once

#include "matrix.h"

#include <numeric>

static_assert([] {
    // initializer list
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    assert(a.rows() == 3);
    assert(a.cols() == 2);
    assert((a[0, 0] == 1));
    assert((a[0, 1] == 2));
    assert((a[1, 0] == 3));
    assert((a[1, 1] == 4));
    assert((a[2, 0] == 5));
    assert((a[2, 1] == 6));

    // equals
    assert((a == Matrixf{ 3, 2, { 1, 2, 3, 4, 5, 6 } }));

    // copy
    Matrixf c = a;
    assert(c.rows() == 3);
    assert(c.cols() == 2);
    assert((c[0, 0] == 1));
    assert((c[0, 1] == 2));
    assert((c[1, 0] == 3));
    assert((c[1, 1] == 4));
    assert((c[2, 0] == 5));
    assert((c[2, 1] == 6));
    assert(c == a);

    // move
    Matrixf b = std::move(a);
    assert(a.rows() == 0);
    assert(a.cols() == 0);
    assert(b.rows() == 3);
    assert(b.cols() == 2);
    assert((b[0, 0] == 1));
    assert((b[0, 1] == 2));
    assert((b[1, 0] == 3));
    assert((b[1, 1] == 4));
    assert((b[2, 0] == 5));
    assert((b[2, 1] == 6));

    return true;
}());

// row view
static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    assert(a.cols() == 2);

    auto r0 = a.row(0);
    assert(r0.size() == 2);
    assert(r0[0] == 1);
    assert(r0[1] == 2);

    auto r1 = a.row(1);
    assert(r1.size() == 2);
    assert(r1[0] == 3);
    assert(r1[1] == 4);

    auto r2 = a.row(2);
    assert(r2.size() == 2);
    assert(r2[0] == 5);
    assert(r2[1] == 6);

    r2[1] = 7;
    assert((a[2, 1] == 7));

    return true;
}());

// column view
static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    assert(a.rows() == 3);

    auto c0 = a.column(0);
    assert(c0.size() == 3);
    assert(c0[0] == 1);
    assert(c0[1] == 3);
    assert(c0[2] == 5);

    auto c1 = a.column(1);
    assert(c1.size() == 3);
    assert(c1[0] == 2);
    assert(c1[1] == 4);
    assert(c1[2] == 6);

    std::ranges::fill(c0, 1);
    return true;
}());

// column iterator
static_assert([] {
    Matrixf a{ 3, 2, { 0, 0, 0, 0, 0, 0 } };

    auto c = a.column(1);
    assert(c.size() == 3);
    assert(std::distance(c.begin(), c.end()) == 3);
    std::ranges::iota(c, 1);

    assert(c[0] == 1);
    assert(c[1] == 2);
    assert(c[2] == 3);

    assert((a == Matrixf{ 3, 2, { 0, 1, 0, 2, 0, 3 } }));

    return true;
}());

// multiplication
static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    Matrixf b = a * 2;
    assert((Matrixf{ a * 2 } == Matrixf{ 3, 2, { 2, 4, 6, 8, 10, 12 } }));
    assert((Matrixf{ 2 * a } == Matrixf{ 3, 2, { 2, 4, 6, 8, 10, 12 } }));
    return true;
}());

static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    Matrixf b{ 2, 3, { 7, 8, 9, 10, 11, 12 } };
    Matrixf c = a * b;
    assert((c == Matrixf{ 3, 3, { 27, 30, 33, 61, 68, 75, 95, 106, 117 } }));
    return true;
}());

static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    Matrixf b{ 3, 2, { 7, 8, 9, 10, 11, 12 } };
    Matrixf c = a + b;
    assert((c == Matrixf{ 3, 2, { 8, 10, 12, 14, 16, 18 } }));
    return true;
}());

static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    Matrixf b{ 3, 2, { 7, 8, 9, 10, 11, 12 } };
    Matrixf c = a - b;
    assert((c == Matrixf{ 3, 2, { -6, -6, -6, -6, -6, -6 } }));
    return true;
}());

static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    Matrixf b{ 3, 2, { 7, 8, 9, 10, 11, 12 } };
    Matrixf c = a % b;
    assert((c == Matrixf{ 3, 2, { 7, 16, 27, 40, 55, 72 } }));
    return true;
}());

static_assert([] {
    Matrixf a{ 3, 2, { 1, 2, 3, 4, 5, 6 } };
    Matrixf t = transposed(a);
    assert(t.rows() == 2);
    assert(t.cols() == 3);
    assert((t == Matrixf{ 2, 3, { 1, 3, 5, 2, 4, 6 } }));
    return true;
}());
