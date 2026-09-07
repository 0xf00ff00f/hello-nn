#pragma once

#include <algorithm>
#include <utility>
#include <cassert>

template<typename ExprT>
class MatrixExpression
{
public:
    static constexpr bool IsLeaf = false;

    constexpr auto operator[](std::size_t r, std::size_t c) const
    {
        return static_cast<const ExprT &>(*this)[r, c];
    }

    constexpr std::size_t rows() const
    {
        return static_cast<const ExprT &>(*this).rows();
    }

    constexpr std::size_t cols() const
    {
        return static_cast<const ExprT &>(*this).cols();
    }
};

template<typename T>
struct Matrix : public MatrixExpression<Matrix<T>> {
public:
    static constexpr bool IsLeaf = true;

    constexpr Matrix(std::size_t rows, std::size_t cols)
        : m_rows(rows), m_cols(cols), m_data(new T[m_rows * (m_cols + 1)]) // extra row of padding for column iterator end
    {
    }

    constexpr Matrix(std::size_t rows, std::size_t cols, std::initializer_list<T> data)
        : Matrix{ rows, cols }
    {
        assert(data.size() == rows * cols);
        std::ranges::copy(data, begin());
    }

    template<typename E>
    constexpr Matrix(const MatrixExpression<E> &expr)
        : Matrix{ expr.rows(), expr.cols() }
    {
        for (std::size_t row = 0; row < m_rows; ++row) {
            for (std::size_t col = 0; col < m_cols; ++col) {
                operator[](row, col) = expr[row, col];
            }
        }
    }

    constexpr ~Matrix() { delete[] m_data; }

    constexpr Matrix(const Matrix &other)
        : m_rows(other.m_rows), m_cols(other.m_cols), m_data(new T[m_rows * m_cols])
    {
        std::ranges::copy(other, begin());
    }

    constexpr Matrix &operator=(const Matrix &other)
    {
        if (this != &other) {
            Matrix temp{ other };
            std::swap(m_rows, temp.m_rows);
            std::swap(m_cols, temp.m_cols);
            std::swap(m_data, temp.m_data);
        }
        return *this;
    }

    constexpr Matrix(Matrix &&other)
        : m_rows(std::exchange(other.m_rows, 0)), m_cols(std::exchange(other.m_cols, 0)), m_data(std::exchange(other.m_data, nullptr))
    {
    }

    constexpr Matrix &operator=(Matrix &&other)
    {
        if (this != &other) {
            delete[] m_data;
            m_rows = std::exchange(other.m_rows, 0);
            m_cols = std::exchange(other.m_cols, 0);
            m_data = std::exchange(other.m_data, nullptr);
        }
        return *this;
    }

    constexpr std::size_t rows() const { return m_rows; }
    constexpr std::size_t cols() const { return m_cols; }

    constexpr T &operator[](std::size_t r, std::size_t c) { return m_data[r * m_cols + c]; }
    constexpr const T &operator[](std::size_t r, std::size_t c) const { return m_data[r * m_cols + c]; }

    constexpr T *begin() { return m_data; }
    constexpr T *end() { return m_data + m_rows * m_cols; }

    constexpr const T *begin() const { return const_cast<Matrix *>(this)->begin(); }
    constexpr const T *end() const { return const_cast<Matrix *>(this)->end(); }

    constexpr bool operator==(const Matrix &other) const
    {
        return m_rows == other.m_rows && m_cols == other.m_cols && std::equal(begin(), end(), other.begin());
    }

    template<typename U>
    class RowView
    {
    public:
        constexpr std::size_t size() const { return m_size; }

        constexpr U &operator[](std::size_t i) const
        {
            return m_ptr[i];
        }

        constexpr auto begin() const { return m_ptr; }
        constexpr auto end() const { return m_ptr + m_size; }

    private:
        constexpr RowView(U *ptr, std::size_t size)
            : m_ptr{ ptr }
            , m_size{ size }
        {
        }

        U *m_ptr{ 0 };
        std::size_t m_size{ 0 };

        friend class Matrix;
    };

    constexpr auto row(std::size_t i) { return RowView<T>{ m_data + i * m_cols, m_cols }; }
    constexpr auto row(std::size_t i) const { return RowView<const T>{ m_data + i * m_cols, m_cols }; }

    template<typename U>
    class ColumnView
    {
    public:
        constexpr std::size_t size() const { return m_size; }

        constexpr U &operator[](std::size_t i) const
        {
            return m_ptr[i * m_stride];
        }

        class Iterator
        {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = U;

            constexpr Iterator() = default;

            constexpr Iterator &operator++()
            {
                m_ptr += m_stride;
                return *this;
            }

            constexpr Iterator operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            constexpr value_type &operator*() const { return *m_ptr; }

            constexpr bool operator==(const Iterator &) const = default;

        private:
            constexpr explicit Iterator(U *ptr, std::ptrdiff_t stride)
                : m_ptr(ptr)
                , m_stride(stride)
            {
            }

            U *m_ptr{ nullptr };
            std::ptrdiff_t m_stride{ 0 };

            friend class Matrix;
        };
        static_assert(std::forward_iterator<Iterator>);

        constexpr auto begin() const { return Iterator{ m_ptr, m_stride }; }
        constexpr auto end() const { return Iterator{ m_ptr + m_size * m_stride, m_stride }; }

    private:
        constexpr ColumnView(U *ptr, std::size_t size, std::ptrdiff_t stride)
            : m_ptr{ ptr }
            , m_size{ size }
            , m_stride{ stride }
        {
        }

        U *m_ptr{ nullptr };
        std::size_t m_size{ 0 };
        std::ptrdiff_t m_stride{ 0 };

        friend class Matrix;
    };

    constexpr auto column(std::size_t i) { return ColumnView<T>{ m_data + i, m_rows, static_cast<std::ptrdiff_t>(m_cols) }; }
    constexpr auto column(std::size_t i) const { return ColumnView<const T>{ m_data + i, m_rows, static_cast<std::ptrdiff_t>(m_cols) }; }

    template<typename E>
    constexpr Matrix &operator+=(const MatrixExpression<E> &expr)
    {
        assert(expr.rows() == m_rows);
        assert(expr.cols() == m_cols);
        for (std::size_t row = 0; row < m_rows; ++row) {
            for (std::size_t col = 0; col < m_cols; ++col) {
                operator[](row, col) += expr[row, col];
            }
        }
        return *this;
    }

    template<typename E>
    constexpr Matrix &operator-=(const MatrixExpression<E> &expr)
    {
        assert(expr.rows() == m_rows);
        assert(expr.cols() == m_cols);
        for (std::size_t row = 0; row < m_rows; ++row) {
            for (std::size_t col = 0; col < m_cols; ++col) {
                operator[](row, col) -= expr[row, col];
            }
        }
        return *this;
    }

    template<typename E>
    constexpr Matrix &operator%=(const MatrixExpression<E> &expr)
    {
        assert(expr.rows() == m_rows);
        assert(expr.cols() == m_cols);
        for (std::size_t row = 0; row < m_rows; ++row) {
            for (std::size_t col = 0; col < m_cols; ++col) {
                operator[](row, col) *= expr[row, col];
            }
        }
        return *this;
    }

private:
    std::size_t m_rows;
    std::size_t m_cols;
    T *m_data;
};

using Matrixi = Matrix<int>;
using Matrixf = Matrix<float>;

template<typename LeftT, typename RightT, typename OpT>
class MatrixBinaryOp : public MatrixExpression<MatrixBinaryOp<LeftT, RightT, OpT>>
{
public:
    constexpr MatrixBinaryOp(const LeftT &lhs, const RightT &rhs, const OpT &op = OpT{ })
        : m_lhs{ lhs }
        , m_rhs{ rhs }
        , m_op{ op }
    {
        assert(m_lhs.rows() == m_rhs.rows());
        assert(m_lhs.cols() == m_rhs.cols());
    }

    constexpr auto operator[](std::size_t r, std::size_t c) const
    {
        return m_op(m_lhs[r, c], m_rhs[r, c]);
    }

    constexpr std::size_t rows() const
    {
        return m_lhs.rows();
    }

    constexpr std::size_t cols() const
    {
        return m_lhs.cols();
    }

protected:
    std::conditional_t<LeftT::IsLeaf, const LeftT &, LeftT> m_lhs;
    std::conditional_t<RightT::IsLeaf, const RightT &, RightT> m_rhs;
    std::decay_t<OpT> m_op;
};

template<typename LeftT, typename RightT>
using MatrixAdd = MatrixBinaryOp<LeftT, RightT, std::plus<>>;

template<typename LeftT, typename RightT>
using MatrixSub = MatrixBinaryOp<LeftT, RightT, std::minus<>>;

template<typename LeftT, typename RightT>
using MatrixElementwiseMul = MatrixBinaryOp<LeftT, RightT, std::multiplies<>>;

template<typename LeftT, typename RightT>
constexpr auto operator+(const MatrixExpression<LeftT> &lhs, const MatrixExpression<RightT> &rhs)
{
    return MatrixAdd{ static_cast<const LeftT &>(lhs), static_cast<const RightT &>(rhs) };
}

template<typename LeftT, typename RightT>
constexpr auto operator-(const MatrixExpression<LeftT> &lhs, const MatrixExpression<RightT> &rhs)
{
    return MatrixSub{ static_cast<const LeftT &>(lhs), static_cast<const RightT &>(rhs) };
}

template<typename LeftT, typename RightT>
constexpr auto operator%(const MatrixExpression<LeftT> &lhs, const MatrixExpression<RightT> &rhs)
{
    return MatrixElementwiseMul{ static_cast<const LeftT &>(lhs), static_cast<const RightT &>(rhs) };
}

template<typename LeftT, typename OpT>
class MatrixApply : public MatrixExpression<MatrixApply<LeftT, OpT>>
{
public:
    constexpr MatrixApply(const LeftT &lhs, const OpT &op = { })
        : m_lhs{ lhs }
        , m_op{ op }
    {
    }

    constexpr auto operator[](std::size_t r, std::size_t c) const
    {
        return m_op(m_lhs[r, c]);
    }

    constexpr std::size_t rows() const
    {
        return m_lhs.rows();
    }

    constexpr std::size_t cols() const
    {
        return m_lhs.cols();
    }

private:
    std::conditional_t<LeftT::IsLeaf, const LeftT &, LeftT> m_lhs;
    std::decay_t<OpT> m_op;
};

template<typename LeftT, typename RightT>
    requires std::is_arithmetic_v<RightT>
constexpr auto operator*(const MatrixExpression<LeftT> &lhs, RightT rhs)
{
    return MatrixApply{ static_cast<const LeftT &>(lhs), [rhs](auto value) { return value * rhs; } };
}

template<typename LeftT, typename RightT>
    requires std::is_arithmetic_v<LeftT>
constexpr auto operator*(LeftT lhs, const MatrixExpression<RightT> &rhs)
{
    return MatrixApply{ static_cast<const RightT &>(rhs), [lhs](auto value) { return lhs * value; } };
}

template<typename LeftT, typename RightT>
class MatrixMul : public MatrixExpression<MatrixMul<LeftT, RightT>>
{
public:
    constexpr MatrixMul(const LeftT &lhs, const RightT &rhs)
        : m_lhs{ lhs }
        , m_rhs{ rhs }
    {
        assert(m_lhs.cols() == m_rhs.rows());
    }

    constexpr auto operator[](std::size_t r, std::size_t c) const
    {
        using ElementT = std::common_type_t<std::decay_t<decltype(m_lhs[0, 0])>, std::decay_t<decltype(m_rhs[0, 0])>>;
        ElementT result{ 0 };
        for (std::size_t i = 0; i < m_lhs.cols(); ++i) {
            result += m_lhs[r, i] * m_rhs[i, c];
        }
        return result;
    }

    constexpr std::size_t rows() const
    {
        return m_lhs.rows();
    }

    constexpr std::size_t cols() const
    {
        return m_rhs.cols();
    }

private:
    std::conditional_t<LeftT::IsLeaf, const LeftT &, LeftT> m_lhs;
    std::conditional_t<RightT::IsLeaf, const RightT &, RightT> m_rhs;
};

template<typename LeftT, typename RightT>
constexpr auto operator*(const MatrixExpression<LeftT> &lhs, const MatrixExpression<RightT> &rhs)
{
    return MatrixMul{ static_cast<const LeftT &>(lhs), static_cast<const RightT &>(rhs) };
}

template<typename ExprT>
class MatrixTranspose : public MatrixExpression<MatrixTranspose<ExprT>>
{
public:
    constexpr MatrixTranspose(const ExprT &expr)
        : m_expr{ expr }
    {
    }

    constexpr auto operator[](std::size_t r, std::size_t c) const
    {
        return m_expr[c, r];
    }

    constexpr std::size_t rows() const
    {
        return m_expr.cols();
    }

    constexpr std::size_t cols() const
    {
        return m_expr.rows();
    }

private:
    std::conditional_t<ExprT::IsLeaf, const ExprT &, ExprT> m_expr;
};

template<typename ExprT>
constexpr auto transposed(const MatrixExpression<ExprT> &expr)
{
    return MatrixTranspose{ static_cast<const ExprT &>(expr) };
}
