#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <utility>

template<typename T>
struct Matrix {
public:
    constexpr Matrix(std::size_t rows, std::size_t cols)
        : m_rows(rows), m_cols(cols), m_data(new T[m_rows * m_cols]) { }

    constexpr Matrix(std::size_t rows, std::size_t cols, std::initializer_list<T> data)
        : Matrix{ rows, cols }
    {
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

    constexpr bool operator==(const Matrix& other) const
    {
        return m_rows == other.m_rows && m_cols == other.m_cols && std::equal(begin(), end(), other.begin());
    }

private:
    std::size_t m_rows;
    std::size_t m_cols;
    T *m_data;
};

using Matrixf = Matrix<float>;
