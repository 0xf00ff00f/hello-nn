#pragma once

#include <algorithm>
#include <utility>
#include <cassert>
#include <numeric>

template<typename T>
struct Matrix {
public:
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
        : m_rows(std::exchange(other.m_rows, 0)), m_cols(std::exchange(other.m_cols, 0)), m_data(std::exchange(other.m_data, nullptr)) { }

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

    constexpr T &operator[](std::size_t i, std::size_t j) { return m_data[i * m_cols + j]; }
    constexpr const T &operator[](std::size_t i, std::size_t j) const
    {
        return const_cast<Matrix *>(this)->m_data[i * m_cols + j];
    }

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

    constexpr Matrix operator*(const Matrix &other) const
    {
        assert(m_cols == other.m_rows);
        Matrix result{ m_rows, other.m_cols };
        for (std::size_t i = 0; i < m_rows; ++i) {
            for (std::size_t j = 0; j < other.m_cols; ++j) {
                const auto r = row(i);
                const auto c = other.column(j);
                assert(r.size() == c.size());
                result[i, j] = std::inner_product(r.begin(), r.end(), c.begin(), T{ 0 });
            }
        }
        return result;
    }

private:
    std::size_t m_rows;
    std::size_t m_cols;
    T *m_data;
};

using Matrixi = Matrix<int>;
using Matrixf = Matrix<float>;
