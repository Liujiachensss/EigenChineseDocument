#pragma once

#include "HeaderFile.h"


/*
切片和索引
密集矩阵和数组操作
本页介绍了使用operator()来对行或者列做索引提供的众多可能性。此 API 已在 Eigen 3.4 中引入。它支持 block API的所有功能, 以及更多。
特别是, 它支持切片, 包括采用一组行、列或元素, 这些行、列或元素在矩阵中均匀分布或从索引数组中索引。

+ 概述
上述所有操作都通过通用 DenseBase::operator()(const RowIndices&, const ColIndices&) 方法处理。每个参数可以是:
    *为单个行或列编制索引的整数, 包括符号索引。
    *符号 Eigen::all 按升序表示整个相应行或列的集合。
    *由 Eigen::seq、Eigen::seqN 或 Eigen::placeholders::lastN 函数构造的 ArithmeticSequence 类。
    *任何整数的一维向量/数组, 包括 Eigen的 vector/array、表达式、std::vector、std::array 以及C数组 int[N]

更一般地说, 它可以接受以具备以下两个公开成员函数的任何对象：
    * typename IntegralType operator[](<integral type>) const;
    * typename IntegralType size() const;
    * 其中 IntegralType 类型表示与 Eigen::Index 兼容的任何整数类型(比如 std::ptrdiff_t)。

+ 基本切片
采用一组在矩阵或向量中均匀分布的行、列或元素是通过 Eigen::seq 或 Eigen::seqN 函数实现的, 其中“seq”代表算术序列。他们的签名总结如下:

*    功能	                            描述	                                 例 {1,2,3,4,5...} 的输出 
*    seq(firstIdx, lastIdx)             表示从 到 的整数序列firstIdxlastIdx	      seq(2,5) <=> {2,3,4,5}
*    seq(firstIdx, lastIdx, incr)       与上同，但步进的增量是incr，而不是1	       seq(2,8,2) <=> {2,4,6,8}
*    seqN(firstIdx, size)               表示从firstIdx开始的size个数              seqN(2,5) <=> {2,3,4,5,6}
*    seqN(firstIdx, size, incr)         与上同，但步进的增量是incr，而不是1	       seqN(2,3,3) <=> {2,5,8}

还可以在Eigen::last符号的帮助下定义firstdx和lastdx参数，
该符号表示通过operator()将等差数列传递给底层矩阵/向量后的最后一行、最后一列或最后一个元素的索引。
下面是二维数组/矩阵a和一维数组/向量v的一些示例。

*   意图	                              code	                            等效的block-API
*   从i行n列开始的左下角	                A(seq(i, last), seqN(0, n))      A.bottomLeftCorner(A.rows()-i, n)
*   从(i, j)开始的块，具有m行和n列	        A(seqN(i, m), seqN(j, n))         A.block(i, j, m, n)
*   块从(i0, j0) 开始到(i1, j1)结束	       A(seq(i0, i1),  seq(j0, j1)       A.block(i0, j0, i1-i0+1, j1-j0+1)
*   A的偶数列	                           A(all,  seq(0, last, 2))
*   A前奇数行n	                           A(seqN(1, n, 2),  all) 
*   最后一列	                           A(all, last-1)                   A.col(A.cols()-2)
*   最中间的行	                           A(last/2, all))                  A.row((A.rows()-1)/2)
*   v从i开始到最后的所有元素	            v(seq(i, last))                  v.tail(v.size()-i)
*   v的最后n个元素	                       v(seq(last+1-n, last))           v.tail(n)

如上例所示, 引用最后 n 个元素(或行/列)编写起来有点麻烦。对于非默认增量, 这将变得更加棘手且容易出错。
函数提供了一种更优雅的方法来引用最后 n 个元素(或行/列)。
*   Eigen::placeholders::lastN(size) 和 Eigen::placeholders::lastN(size, incr)：

*   意图	               code	                          等效的block-API 
*   v 的最后元素n	        v(lastN(n))                    v.tail(n)
*   A 的右下角m*n	        v(lastN(m), lastN(n))          A.bottomRightCorner(m, n)
*   A 的右下角m*n	        v(lastN(m),  lastN(n))         A.bottomRightCorner(m, n)
*   最后n列每列跨过3列	     A(all,  lastN(n, 3))

+ 编译时大小和增量
在性能方面, Eigen 和编译器可以利用编译时大小和增量。
为此, 您可以使用 Eigen::fix<val> 使用编译时参数。这样的编译时值可以与 Eigen::last 符号结合使用:
*   v(seq(last-fix<7>, last-fix<2>))

在此示例中, Eigen 在编译时知道返回的表达式有 6 个元素。它相当于:
*   v(seqN(last-7, fix<6>))

我们可以重新访问 A example 的偶数列, 如下所示:
*   A(all, seq(0, last, fix<2>))

+ 倒序
行/列索引也可以使用负增量按降序枚举。例如, 从第20-10列，每两列取1列:
*   A(all, seq(20, 10, fix<-2>))

从最后一行开始的n行:
*   A(seqN(last, n, fix<-1>), all)
您还可以使用 ArithmeticSequence::reverse() 方法反转其顺序。因此, 前面的示例也可以写成:
*   A(lastN(n).reverse(), all)

+ 索引数组
泛型operator()还可以将任意行或列索引列表作为输入，存储为ArrayXi，std::vector<int>，std::array<int,N>等等

*/

void Section5_SlicingAndIndexing(){
    {
        std::vector<int> ind{4,2,5,5,3};
        MatrixXi A = MatrixXi::Random(4,6);
        cout << "Initial matrix A:\n" << A << "\n\n";
        cout << "A(all,ind):\n" << A(Eigen::placeholders::all,ind) << "\n\n";
        /*
        行取所有行，列按ind来，假如输入：
        7   9  -5  -3   3 -10
        -2  -6   1   0   5  -5
        6  -3   0   9  -8  -8
        6   6   3   9   2   6
        则输出：
          3  -5 -10 -10  -3
          5   1  -5  -5   0
         -8   0  -8  -8   9
          2   3   6   6   9
        */
    }

    // 或：
    {   
        MatrixXi A = MatrixXi::Random(4,6);
        cout << "Initial matrix A:\n" << A << "\n\n";
        cout << "A(all,{4,2,5,5,3}):\n" << A(Eigen::placeholders::all, {4,2,5,5,3}) << "\n\n";
    }

    // 也可以有：
    {
        ArrayXi ind(5); ind<<4,2,5,5,3;
        MatrixXi A = MatrixXi::Random(4,6);
        cout << "Initial matrix A:\n" << A << "\n\n";
        cout << "A(all,ind-1):\n" << A(Eigen::placeholders::all, ind-1) << "\n\n";
    }
}

// 当传递一个具有编译时大小的对象，如Array4i、std::array<int,N>或静态数组时，返回的表达式也具有编译时尺寸。

/*
+ 自定义索引列表
更一般的operator()可以接受任何 ind 的 T 类型的输入：
Index s = ind.size(); or Index s = size(ind);
Index i;
i = ind[i];
*/
//! 其实就是只要类型有公开的size()和operator[]即可
struct pad {
  Index size() const { return out_size; }
  Index operator[] (Index i) const { return std::max<Index>(0, i - (out_size - in_size)); }
  Index in_size, out_size;
};

void Section5_SlicingAndIndexing2(){    
    Matrix3i A;
    A.reshaped() = VectorXi::LinSpaced(9, 1, 9);
    cout << "Initial matrix A:\n" << A << "\n\n";
    MatrixXi B(5, 5);
    B = A(pad{3, 5}, pad{3, 5});
    cout << "A(pad{3, N}, pad{3, N}):\n" << B << "\n\n";
}