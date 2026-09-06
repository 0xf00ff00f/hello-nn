#pragma once

#include <cmath>
#include <random>

#include "matrix.h"

template<typename T>
constexpr auto sigmoid(T x)
{
    return T{ 1 } / (T{ 1 } + std::exp(-x));
}

template<typename T>
constexpr auto sigmoidDerivative(T x)
{
    return x * (T{ 1 } - x);
}

template<typename ExprT, typename F>
class MatrixApply : public MatrixExpression<MatrixApply<ExprT, F>>
{
public:
    constexpr MatrixApply(const ExprT &expr, const F &fn)
        : m_expr{ expr }
        , m_fn{ fn }
    {
    }

    constexpr auto operator[](std::size_t r, std::size_t c) const
    {
        return m_fn(m_expr[r, c]);
    }

    constexpr std::size_t rows() const
    {
        return m_expr.rows();
    }

    constexpr std::size_t cols() const
    {
        return m_expr.cols();
    }

private:
    std::conditional_t<ExprT::IsLeaf, const ExprT &, ExprT> m_expr;
    const F &m_fn;
};

template<typename ExprT>
constexpr auto applySigmoid(const MatrixExpression<ExprT> &expr)
{
    return MatrixApply{ static_cast<const ExprT &>(expr), sigmoid<decltype(expr[0, 0])> };
}

template<typename ExprT>
constexpr auto applySigmoidDerivative(const MatrixExpression<ExprT> &expr)
{
    return MatrixApply{ static_cast<const ExprT &>(expr), sigmoidDerivative<decltype(expr[0, 0])> };
}

template<typename T>
class NeuralNetwork
{
public:
    using MatrixT = Matrix<T>;

    NeuralNetwork(std::size_t input, std::size_t hidden, std::size_t output, T learningRate)
        : m_input{ input }
        , m_hidden{ hidden }
        , m_output{ output }
        , m_weightsIH{ hidden, input }
        , m_weightsHO{ output, hidden }
        , m_biasH{ hidden, 1 }
        , m_biasO{ output, 1 }
        , m_learningRate{ learningRate }
    {
        auto initializeRandom = [](MatrixT &m) {
            std::random_device rd;
            std::mt19937 gen{ rd() };
            std::uniform_real_distribution<T> dist(T{ -1 }, T{ 1 });
            std::ranges::generate(m, [&] { return dist(gen); });
        };
        initializeRandom(m_weightsIH);
        initializeRandom(m_weightsHO);
        initializeRandom(m_biasH);
        initializeRandom(m_biasO);
    }

    MatrixT feedForward(const Matrixf &input) const
    {
        MatrixT hidden = applySigmoid(m_weightsIH * input + m_biasH);
        MatrixT output = applySigmoid(m_weightsHO * hidden + m_biasO);
        return output;
    }

    void train(const MatrixT &input, const MatrixT &target)
    {
        // forward pass

        MatrixT hiddenOutputs = applySigmoid(m_weightsIH * input + m_biasH);
        MatrixT finalOutputs = applySigmoid(m_weightsHO * hiddenOutputs + m_biasO);

        // backward pass

        MatrixT outputErrors = target - finalOutputs;
        MatrixT outputGradients = (applySigmoidDerivative(finalOutputs) % outputErrors) * m_learningRate;
        MatrixT deltaWeightsHO = outputGradients * transposed(hiddenOutputs);
        m_weightsHO += deltaWeightsHO;
        m_biasO += outputGradients;

        MatrixT hiddenErrors = transposed(m_weightsHO) * outputErrors;
        MatrixT hiddenGradients = (applySigmoidDerivative(hiddenOutputs) % hiddenErrors) * m_learningRate;
        MatrixT deltaWeightsIH = hiddenGradients * transposed(input);
        m_weightsIH += deltaWeightsIH;
        m_biasH += hiddenGradients;
    }

private:
    std::size_t m_input;
    std::size_t m_hidden;
    std::size_t m_output;
    MatrixT m_weightsIH;
    MatrixT m_weightsHO;
    MatrixT m_biasH;
    MatrixT m_biasO;
    float m_learningRate;
};
