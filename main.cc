#include <iostream>
#include <print>

#include "nn.h"

template<typename T>
void print(const Matrix<T> &matrix)
{
    for (std::size_t i = 0; i < matrix.rows(); ++i) {
        for (std::size_t j = 0; j < matrix.cols(); ++j) {
            std::cout << matrix[i, j] << ' ';
        }
        std::cout << '\n';
    }
}

int main()
{
    NeuralNetwork<float> nn{ 2, 3, 1, 0.1 };

    for (std::size_t epoch = 0; epoch < 50'000; ++epoch) {
        const auto a = epoch & 1;
        const auto b = (epoch >> 1) & 1;
        Matrixf input{ 2, 1, { static_cast<float>(a), static_cast<float>(b) } };
        Matrixf target{ 1, 1, { static_cast<float>(a ^ b) } };
        nn.train(input, target);
    }

    for (std::size_t i = 0; i < 4; ++i) {
        const auto a = i & 1;
        const auto b = (i >> 1) & 1;
        Matrixf input{ 2, 1, { static_cast<float>(a), static_cast<float>(b) } };
        Matrixf output = nn.feedForward(input);
        std::println("{} ^ {} = {}", a, b, output[0, 0]);
    }
}
