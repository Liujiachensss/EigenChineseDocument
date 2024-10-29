#ifndef ALIASING_HPP
#define ALIASING_HPP
#include "HeaderFile.h"

namespace Chapter1_DenseMatrixAndArrary
{

namespace Section9_Aliasing
{
// 在Eigen中，混叠(aliasing)是指在赋值操作符的左侧和右侧出现相同矩阵(或数组或向量)的赋值语句。
// 像mat = 2 * mat；或者mat = mat.transpose() 表现出混叠。
// 第一个示例中的混叠是无害的，但第二个示例中的混叠会导致意想不到的结果。这个页面解释了什么是混叠，什么时候它是有害的，以及如何做。

// 有害混叠
// 使用.eval()方法，可以解决混叠问题。具体的说，.eval()方法生成临时对象，然后再执行赋值操作
// 如果Eigen中假定该操作是混叠的，那么它会自动使用.eval()方法，而不需要我们显示调用

// 无害混叠是我们不需要评估，它对元素计算无害。这包括标量乘法和矩阵或数组加法。

// 将两个矩阵相乘时，Eigen会假定发生混叠(注意，Eigen3.3以后的版本中需要区分目标矩阵大小是否改变了)
// 如果您知道没有混叠，则可以使用noalias()。

// 在所有其他情况下，Eigen假定不存在混叠问题，因此如果实际上发生混叠，则会给出错误的结果。
// 为防止这种情况，您必须使用eval()或xxxInPlace()函数之一。

void Aliasing()
{
        LOG();
        //在Eigen中，混叠(aliasing)是指相同的矩阵(或数组或向量)出现在赋值运算符的左侧和右侧的赋值语句。
        // 例如mat = 2 * mat(虽混叠但无害);或mat = mat.transpose();(有害的混叠)。

        MatrixXi mat(3, 3);
        mat << 1, 2, 3, 
               4, 5, 6, 
               7, 8, 9;
        cout << "Here is the matrix mat:\n"
             << mat << "\n\n";
        // This assignment shows the aliasing problem
        mat.bottomRightCorner(2, 2) = mat.topLeftCorner(2, 2);
        cout << "After the assignment, mat = \n"
             << mat << "\n\n";

        // Output  is:
        // Here is the matrix mat:
        // 1 2 3
        // 4 5 6
        // 7 8 9
        // After the assignment, mat =
        // 1 2 3
        // 4 1 2
        // 7 4 1

        // 理解混叠的关键是了解惰性求值

        // 输出结果不是人们所期望的，问题是产生了有害混叠
        //* mat.bottomRightCorner(2,2) = mat.topLeftCorner(2,2);
        // 该赋值具有混叠aliasing)：元素mat(1,1)既出现在mat.bottomRightCorner(2,2)分配左侧的块中mat.topLeftCorner(2,2)，又出现在右侧的块中。
        // 分配后，右下角的(2,2)项应具有mat(1,1)分配前的值5。但是，输出显示mat(2,2)实际上为1。
        // 问题在于Eigen使用了惰性求值。结果类似于
        // mat(1,1)= mat(0,0);
        // mat(1,2)= mat(0,1);
        // mat(2,1)= mat(1,0);
        // mat(2,2)= mat(1,1);
        // 因此，mat(2,2)分配了新值mat(1,1)而不是旧值。
        // 可以通过调用eval()解决此问题(注:eval()负责生成临时对象而避免混叠)
        // 尝试缩小矩阵时，混叠也会发生。
        // 例如，表达式vec = vec.head(n)和mat = mat.block(i,j,r,c)具有混叠。

        //通常，在编译时无法检测到混叠：如果mat在第一个示例中稍大一点，则块将不会重叠，也不会出现混叠问题。
        //但是Eigen确实会在运行时检测到一些混叠实例。Matrix和向量算术中提到了以下显示别名的示例：

        // 注意：这段代码会报错～～～～～～

        Matrix2i a;
        a << 1, 2, 3, 4;
        cout << "Here is the matrix a:\n"
             << a << "\n\n";
        a = a.transpose().eval(); // !!! do NOT do this without eval() !!!
        cout << "and the result of the aliasing effect:\n"
             << a << "\n\n";

        // 输出显示混叠(alising)问题。
        // 但是，默认情况下，Eigen使用运行时断言来检测到此情况并退出，并显示如下消息
        // void Eigen::DenseBase<Derived>::checkTransposeAliasing(const OtherDerived&) const
        // [with OtherDerived = Eigen::Transpose<Eigen::Matrix<int, 2, 2, 0, 2, 2> >, Derived = Eigen::Matrix<int, 2, 2, 0, 2, 2>]:
        // Assertion `(!internal::check_transpose_aliasing_selector<Scalar,internal::blas_traits<Derived>::IsTransposed,OtherDerived>::run(internal::extract_data(derived()), other))
        // && "aliasing detected during transposition, use transposeInPlace() or evaluate the rhs into a temporary using .eval()"' failed.

        //! 用户可以通过定义EIGEN_NO_DEBUG宏来关闭Eigen的运行时断言
}

void ResolvingAliasingIssues()
{
        LOG();
        //解决方法：Eigen必须将右侧完全看作一个临时矩阵/数组，然后将其分配给左侧。
        //函数**eval()**正是这样做的,作用为生成一个临时对象
        MatrixXi mat(3, 3);
        mat << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        cout << "Here is the matrix mat:\n"
             << mat << "\n\n";
        // The eval() solves the aliasing problem
        mat.bottomRightCorner(2, 2) = mat.topLeftCorner(2, 2).eval();
        cout << "After the assignment, mat = \n"
             << mat << "\n\n";

        //相同的解决方案也适用于第二示例中，与转置：只需更换线a = a.transpose();用a = a.transpose().eval();。但是，在这种常见情况下，有更好的解决方案。
        //Eigen提供了专用函数transposeInPlace()，该函数通过其转置来替换矩阵。如下所示：
        MatrixXf a(2, 3);
        a << 1, 2, 3, 4, 5, 6;
        cout << "Here is the initial matrix a:\n"
             << a << "\n\n";
        a.transposeInPlace();
        cout << "and after being transposed:\n"
             << a << "\n\n";
             
        //如果xxxInPlace()函数可用，则最好使用它，因为它可以更清楚地指示您正在做什么。
        //这也可以让Eigen更积极地进行优化。这些是提供的一些xxxInPlace()函数：
        //* Original function	               In-place function
        //* MatrixBase::adjoint()	          MatrixBase::adjointInPlace()
        //* DenseBase::reverse()	          DenseBase::reverseInPlace()
        //* LDLT::solve()	               LDLT::solveInPlace()
        //* LLT::solve()	                    LLT::solveInPlace()
        //* TriangularView::solve()	     TriangularView::solveInPlace()
        //* DenseBase::transpose()	          DenseBase::transposeInPlace()

        //在特殊情况下，矩阵或向量使用类似的表达式缩小vec = vec.head(n)，则可以使用conservativeResize()。
}

void AliasingAndComponentWiseOperations()
{
        LOG();
        // 如果同一矩阵或数组同时出现在赋值运算符的左侧和右侧，则可能很危险，因此通常有必要显示地评估右侧
        // 但是，应用基于元素的操作(例如矩阵加法，标量乘法和数组乘法)是安全的。
        // 以下示例仅具有基于组件的操作。因此，即使相同的矩阵出现在赋值符号的两侧，也不需要eval()。
        MatrixXf mat(2, 2);
        mat << 1, 2, 4, 7;
        cout << "Here is the matrix mat:\n"
             << mat << "\n\n"
             << "\n\n";
        mat = 2 * mat;
        cout << "After 'mat = 2 * mat', mat = \n"
             << mat << "\n\n"
             << "\n\n";
        mat = mat - MatrixXf::Identity(2, 2); //  identity matrix 单位矩阵
        cout << "After the subtraction, it becomes\n"
             << mat << "\n\n"
             << "\n\n";
        ArrayXXf arr = mat;
        arr = arr.square();
        cout << "After squaring, it becomes\n"
             << arr << "\n\n"
             << "\n\n";
        // Combining all operations in one statement:
        mat << 1, 2, 4, 7;
        mat = (2 * mat - MatrixXf::Identity(2, 2)).array().square();
        cout << "Doing everything at once yields\n"
             << mat << "\n\n"
             << "\n\n";
        // Output is:
        // Here is the matrix mat:
        // 1 2
        // 4 7

        // After 'mat = 2 * mat', mat =
        //  2  4
        //  8 14

        // After the subtraction, it becomes
        //  1  4
        //  8 13

        // After squaring, it becomes
        //   1  16
        //  64 169

        // Doing everything at once yields
        //   1  16
        //  64 169
        //通常，如果表达式右侧的(i，j)条目仅取决于左侧矩阵或数组的(i，j)条目
        //而不依赖于其他任何表达式，则赋值是安全的。在这种情况下，不必显示地评估右侧(.evl())。
}

void AliasingAndMatrixMultiplication()
{
        LOG();

        //在目标矩阵**未调整大小**的情况下，矩阵乘法是Eigen中唯一假定默认情况下为混叠的。
        // 若假定混叠，则会使用eval()生成临时对象，所以是安全的。
        //因此，如果matA是平方矩阵，则该语句matA = matA * matA是安全的。
        //Eigen中的所有其他操作都假定没有混叠问题，这是因为结果被分配给了不同的矩阵，或者因为它是逐个元素的操作。
        {
                MatrixXf matA(2, 2);
                matA << 2, 0, 0, 2;
                matA = matA * matA;
                cout << matA << "\n\n";
        }
        /*
        ! 然而，这会带来一定的代价。
        在执行表达式matA = matA * matA时，Eigen会在临时矩阵中计算乘积，然后将结果赋值给matA。这没问题。
        但是，当乘积赋值给另一个矩阵（例如matB = matA * matA）时，Eigen也会这样做。
        在这种情况下，直接将乘积赋值给matB会更高效，而不是先将乘积赋值给临时矩阵，然后再将该矩阵复制到matB。

        用户可以使用 noalias() 函数来表示没有发生数组重叠，如下所示：matB.noalias() = matA * matA。
        这使得 Eigen 可以直接将矩阵乘积 matA * matA 计算到 matB 中。
        */
        {
                MatrixXf matA(2, 2), matB(2, 2);
                matA << 2, 0, 0, 2;
                // Simple but not quite as efficient
                matB = matA * matA;
                cout << matB << "\n\n";
                // More complicated but also more efficient
                matB.noalias() = matA * matA;
                cout << matB << "\n\n";
                // Output is:
                //4 0
                // 0 4

                // 4 0
                // 0 4
        }

        {
                //当然，不应该在实际上发生混叠时使用noalias()，如果这样做，则**可能**会得到错误的结果：
                //报错吗？我的平台上没报错
                MatrixXf matA(2, 2);
                matA << 2, 0, 0, 2;
                matA.noalias() = matA * matA;
                cout << matA << "\n\n";
                // Output is:
                //4 0
                // 0 4
        }

        {
                //此外，从Eigen 3.3 开始，
                //!如果调整了目标矩阵的大小(注意，上文中的操作假定目标矩阵大小不变)
                //并且未将乘积直接分配给目标，则不假定混叠。因此，以下示例也是错误的：
                /*此外，从Eigen 3.3开始，如果目标矩阵被缩放，并且乘积不直接赋值给目标，则不会假设出现混叠现象。因此，以下示例也是错误的：*/
                MatrixXf A(2, 2), B(3, 2);
                B << 2, 0, 
                     0, 3, 
                     1, 1;
                A << 2, 0, 
                     0, -2;
                A = (B * A).cwiseAbs();  // 由于不假定混叠，所以需要我们显示评价，否则可能出错
                // cwiseAbs()对元素做绝对值操作
                cout << A << "\n\n";
                //报错吗？我的平台上没报错，可能会有偶发错误
                // Output                                
                // 4 0
                // 0 6
                // 2 2
        }

        {
                //对于任何混叠问题，您都可以通过在赋值之前对表达式显式调用eval()来解决它：
                MatrixXf A(2, 2), B(3, 2);
                B << 2, 0, 0, 3, 1, 1;
                A << 2, 0, 0, -2;
                A = (B * A).eval().cwiseAbs();
                cout << A << "\n\n";

                // Output is
                // 4 0
                // 0 6
                // 2 2
        }
}

} // namespace Section9_Aliasing
} // namespace Chapter1_DenseMatrixAndArrary


/*
+ 总结
当相同的矩阵或数组系数同时出现在赋值运算符的左右两侧时，就会出现混叠现象。
* 系数级运算（包括标量乘法和矩阵或数组加法）不会产生混叠现象；
* 但在进行矩阵乘法时，Eigen会假定存在混叠现象。如果您知道不存在混叠现象，则可以使用noalias()函数。
* 在其他情况下，Eigen会假定不存在混叠问题，如果确实存在混叠现象，则会给出错误的结果。为了避免这种情况，您需要使用eval()函数或xxxInPlace()函数中的一个。
*/


#endif