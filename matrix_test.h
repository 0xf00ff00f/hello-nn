#pragma once

#include "matrix.h"

#include <cassert>

static_assert([] {
    // initializer list
    Matrixf a{2, 2, {1, 2, 3, 4}};
    assert(a.rows() == 2);
    assert(a.cols() == 2);
    assert((a[0, 0] == 1));
    assert((a[0, 1] == 2));
    assert((a[1, 0] == 3));
    assert((a[1, 1] == 4));
    assert((a == Matrixf{2, 2, {1, 2, 3, 4}}));

    // copy
    Matrixf c = a;
    assert(c.rows() == 2);
    assert(c.cols() == 2);
    assert((c[0, 0] == 1));
    assert((c[0, 1] == 2));
    assert((c[1, 0] == 3));
    assert((c[1, 1] == 4));
    assert(c == a);

    // move
    Matrixf b = std::move(a);
    assert(a.rows() == 0);
    assert(a.cols() == 0);
    assert(b.rows() == 2);
    assert(b.cols() == 2);
    assert((b[0, 0] == 1));
    assert((b[0, 1] == 2));
    assert((b[1, 0] == 3));
    assert((b[1, 1] == 4));

    return true;
}());
