#pragma once

#include "HeaderFile.h"

//从3.4版开始，Eigen的密集矩阵和数组提供了兼容STL的迭代器。如下所示，这使得它们与range-for循环和STL的算法自然兼容。

void test_STL(){
    {
        // Iterating over 1D arrays and vectors
        VectorXi v = VectorXi::Random(4);
        cout << "Here is the vector v:\n";
        for(auto x : v) cout << x << " ";
        cout << "\n";
    }

    {
        Array4i v = Array4i::Random().abs();
        cout << "Here is the initial vector v:\n" << v.transpose() << "\n";
        std::sort(v.begin(), v.end());
        cout << "Here is the sorted vector v:\n" << v.transpose() << "\n";
    }

    {
        // Iterating over coefficients of 2D arrays and matrices
        Matrix2i A = Matrix2i::Random();
        cout << "Here are the coeffs of the 2x2 matrix A:\n";
        for(auto x : A.reshaped())
        cout << x << " ";
        cout << "\n";
    }

    {
        // Iterating over rows or columns of 2D arrays and matrices
        ArrayXXi A = ArrayXXi::Random(4,4).abs();
        cout << "Here is the initial matrix A:\n" << A << "\n";
        for(auto row : A.rowwise())
        std::sort(row.begin(), row.end());
        cout << "Here is the sorted matrix A:\n" << A << "\n";

    }
}