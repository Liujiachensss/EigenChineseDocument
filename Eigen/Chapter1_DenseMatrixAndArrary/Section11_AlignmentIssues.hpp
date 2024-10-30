#ifndef ALIGNMENT_ISSUES_HPP
#define ALIGNMENT_ISSUES_HPP
#include "HeaderFile.h"
#include <vector>
#include <map>

namespace Chapter1_DenseMatrixAndArrary
{

namespace Section11_AlignmentIssues
{
// 这节比较抽象，知道咋用就好！
// TODO 继续翻译！

//+ Explanation of the assertion on unaligned arrays
/* 
! my_program: path/to/eigen/Eigen/src/Core/DenseStorage.h:44:
! Eigen::internal::matrix_array<T, Size, MatrixOptions, Align>::internal::matrix_array()
! [with T = double, int Size = 2, int MatrixOptions = 2, bool Align = true]:
! Assertion `(reinterpret_cast<size_t>(array) & (sizemask)) == 0 && "this assertion
! is explained here: http://eigen.tuxfamily.org/dox-devel/group__TopicUnalignedArrayAssert.html
!      READ THIS WEB PAGE !!! ****"' failed.

这一问题的已知原因有4个。如果您只使用较新的编译器（例如，GCC>=7、clang>=5、MSVC>=19.12）来针对[c++17]，那么您很幸运：
启用c++17应该就足够了（如果不行，请向我们报告）。否则，请继续阅读以了解这些问题并学习如何解决它们。

首先，你需要找出这个断言是在自己的代码的哪个部分触发的。乍一看，这个错误消息似乎没有帮助，因为它指向了Eigen库中的一个文件！
然而，由于你的程序崩溃了，如果你可以重现崩溃，你可以使用任何调试器获取一个堆栈跟踪。例如，如果你使用GCC，你可以按照以下方式使用GDB调试器：

$ gdb ./my_program          # Start GDB on your program
> run                       # Start running your program
...                         # Now reproduce the crash!
> bt                        # Obtain the backtrace
*/

//然后与下面的4种原因对号入座，修改代码
//+ case1: 结构体中具有Eigen对象成员
class Foo
{
  //...
  Eigen::Vector4d v;
  //...
};
//...
Foo *foo = new Foo;

// then you need to read this separate page: "Structures Having Eigen Members".

// Note that here, Eigen::Vector4d is only used as an example, more generally the issue arises for all "fixed-size vectorizable Eigen types".

//+ Fixed-size vectorizable Eigen objects

// 更一般而言，所有固定大小的可矢量化Eigen类型都会出现此问题
// 如果一个 Eigen 对象具有固定大小，并且该大小是 16 字节的整数倍，则该对象被称为“固定大小可向量化”对象。
// Eigen::Vector2d
// Eigen::Vector4d
// Eigen::Vector4f
// Eigen::Matrix2d
// Eigen::Matrix2f
// Eigen::Matrix4d
// Eigen::Matrix4f
// Eigen::Affine3d
// Eigen::Affine3f
// Eigen::Quaterniond
// Eigen::Quaternionf

/*
解释
首先，“固定大小”应该明确：如果Eigen对象的行数和列数在编译时是固定的，那么该对象就是固定大小的。
! 这里讨论的是类似于std::array的Matrix，但是array也有可能被分配在堆上啊！
例如，Matrix3f是固定大小的，而MatrixXf不是（非固定大小的对应物是动态大小）。

固定大小的 Eigen 对象的系数数组是一个简单的“静态数组”，它不是动态分配的。例如，Matrix4f 对象背后的数据只是“float array[16]”。

固定大小的对象通常非常小，这意味着我们希望在运行时不产生任何开销 —— 无论是在内存使用还是速度方面。

现在，矢量化操作使用128位数据包（例如SSE、AltiVec、NEON）、256位数据包（例如AVX）或512位数据包（例如AVX512）。
此外，出于性能考虑，这些数据包应以与数据包大小相同的对齐方式进行读取和写入，即16字节、32字节和64字节。

因此，固定大小的Eigen对象可以进行向量化的最好方法是，它们的大小是16字节（或更大）的倍数。
Eigen将为这些对象请求16字节（或更多）的对齐，并从那时起依赖这些对象的对齐来实现最大效率。
*/

//+ "Structures Having Eigen Members"

/*执行总结

如果你定义了一个具有固定大小可向量化的Eigen类型成员的结构，你必须确保调用operator new为其分配适当对齐的缓冲区。
如果您仅在[C++17]模式下编译（例如，GCC>=7，clang>=5，MSVC>=19.12），则所有操作都由编译器处理，您可以停止阅读。

否则，您必须重载其“operator new”函数，以便它生成正确对齐的指针（例如，对Vector4d和AVX对齐32字节）。
幸运的是，Eigen为您提供了一个名为EIGEN_MAKE_ALIGNED_OPERATOR_NEW的宏，可以为您完成此操作。
*/
class Foo2
{
  //...
  Eigen::Vector2d v;
  //...
};
//...
Foo2 *foo2 = new Foo2; 
//! 这里怎么是new的Foo

// 换句话说：你有一个类，它的成员是一个固定大小的可向量化的特征对象，然后你动态地创建这个类的对象。

// 这样的代码应该如何修改？
// 很简单，你只需要把一个EIGEN_MAKE_ALIGNED_OPERATOR_NEW宏放在类的公共部分，像这样：

class Foo3
{
        Eigen::Vector4d v;
        public : EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
Foo3 *foo = new Foo3;
/*
这个宏使得new Foo总是返回一个对齐的指针。

! 在[c++17]中，这个宏是空的。

如果这种方法过于侵入，请参阅其他解决方案。*/


/* 
++ 为什么需要这样？
Eigen::Vector4d包含4个双精度，即256位。这正是AVX寄存器的大小，这使得在这个向量上使用AVX进行各种操作成为可能。
但是AVX指令（至少是Eigen使用的那些指令，它们是最快的）需要256位对齐。否则就会出现分段错误。
出于这个原因，Eigen自己需要256位对齐Eigen::Vector4d，通过做两件事：
    * Eigen::Vector4d的数组（4双精度）需要256位对齐。这是通过alignas关键字完成的。
    * Eigen重载Eigen::Vector4d的new操作符，因此它将始终返回256位对齐指针。（在[c++17]中删除）
    
因此，通常情况下，您不必担心任何事情，Eigen为您处理操作符new的对齐…

…除了一种情况。当你有一个像上面那样的类Foo，并且你像上面那样动态地分配一个新的Foo，那么，因为Foo没有对齐的 operator new，返回的指针Foo不一定是256位对齐的。

成员v的对齐属性相对于类Foo的开始。如果foo指针没有对齐，那么foo->v也不会对齐！

解决方案是让类Foo有一个对齐的操作符new，如前一节所示。

这个解释也适用于SSE/NEON/MSA/Altivec/VSX目标，它们需要16字节对齐，而AVX512需要64字节对齐，用于固定大小的对象（例如，Eigen::Matrix4d）。

++ 那么我应该把所有特征类型的成员放在类的开头吗？
这不是必需的。由于Eigen负责声明适当的对齐，所有需要它的成员都自动相对于类对齐。所以像这样的代码工作得很好：*/

class Foo4
{
  double x;
  Eigen::Vector4d v;
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

/*也就是说，像往常一样，建议对成员进行排序，这样对齐就不会浪费内存。在上面的例子中，对于AVX，编译器必须在x和v之间保留24个空字节。*/

/*
++ 那么动态大小的矩阵和向量呢？
动态大小的矩阵和向量，如Eigen::VectorXd，会动态分配它们自己的系数数组，因此它们会自动处理要求绝对对齐的问题。
! 所以他们不会引起这个问题。这里讨论的问题仅适用于"固定大小的可矢量矩阵和向量"。

++ 这是eigen的bug吗？
不，这不是我们的bug。它更像是c++语言规范的一个固有问题，在c++17中通过称为为过度对齐的数据分配动态内存的特性解决了这个问题。
https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0035r4.html

++如果我想有条件地执行此操作（取决于模板参数）该怎么办？
对于这种情况，我们提供宏EIGEN_MAKE_ALIGNED_OPERATOR_NEW_IF（NeedsToAlign）。如果NeedsToAlign为true，它将生成像EIGEN_MAKE_ALIGNED_OPERATOR_NEW这样的对齐操作符。
如果NeedsToAlign为false，它将生成具有默认对齐方式的操作符。在[c++17]中，这个宏是空的。*/

template<int n> class Foo5
{
  typedef Eigen::Matrix<float,n,1> Vector;
  enum { NeedsToAlign = (sizeof(Vector)%16)==0 };
  //...
  Vector v;
  //...
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW_IF(NeedsToAlign)
};

Foo5<4> *foo4 = new Foo<4>; // foo4 is guaranteed to be 128bit-aligned
Foo5<3> *foo3 = new Foo<3>; // foo3 has only the system default alignment guarantee

// ++ 其他的解决办法
//如果将EIGEN_MAKE_ALIGNED_OPERATOR_NEW宏放在任何地方都太过干扰，那么至少存在两种其他解决方案。

// +++ 禁用对齐
//首先是禁用固定大小成员的对齐要求：
class Foo6
{
//   ...
  Eigen::Matrix<double,4,1,typename Eigen::DontAlign> v;
//   ...
};

// 这个v与对齐的Eigen::Vector4d完全兼容。这实际上只会使加载/存储到v的成本更高（通常会稍微高一些，但这取决于硬件）。

// +++ 私人结构
// 第二种方法是将固定大小的对象存储到一个私有结构中，该私有结构将在构造主对象时动态分配：
struct Foo_d
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  Vector4d v;
//   ...
};
 
 
struct Foo7 {
  Foo7() { init_d(); }
  ~Foo7() { delete d; }
  void bar()
  {
    // use d->v instead of v
    // ...
  }
private:
  void init_d() { d = new Foo_d; }
  Foo_d* d;
};
// 这里的明显优势是类Foo在对齐问题上保持不变。缺点是无论如何都需要额外的堆分配。


//+ case2：STL容器或手动内存分配

///如果您将STL容器（例如std :: vector，std :: map等）
//与Eigen对象或包含Eigen对象的类一起使用，
std::vector<Eigen::Matrix2f> my_vector;
struct my_class { 
  //... 
  Eigen::Matrix2f m; 
  //... 
};
std::map<int, my_class> my_map;

// 那么您需要阅读本页面的“在STL容器中使用Eigen”部分。
// 需要注意的是，这里仅以Eigen::Matrix2d为例，更普遍的情况是，所有固定大小的可向量化Eigen类型和包含此类Eigen对象的结构都会出现此类问题。
// 任何绕过“new”操作符来分配内存的类/函数都会出现同样的问题，也就是说，先执行自定义内存分配，然后调用“placement new”操作符。
// 例如，std::make_shared或std::allocate_shared通常就是这种情况，解决办法是在STL容器解决方案中详细介绍的使用对齐分配器。

//++ 在STL容器中使用Eigen
//+++ 执行摘要
//! 如果你只在使用较新版本的编译器（例如 GCC>=7、clang>=5 和 MSVC>=19.12）的情况下以 [c++17] 模式编译，那么编译器会处理所有事情，你可以停止阅读了。

// 否则，在固定大小的可向量化 Eigen 类型或包含此类型的类上使用 STL 容器时，需要使用一个过界分配器。
// 也就是说，一个能够分配具有 16、32 甚至 64 字节对齐缓冲区的分配器。Eigen 确实提供了一个可用的分配器：aligned_allocator。

//在C++11之前，如果您想使用std::vector容器，则还需要包含<Eigen/StdVector>头文件。

//这些问题只出现在固定大小的可向量化的 Eigen 类型和包含此类 Eigen 对象的结构中。对于其他 Eigen 类型（如 Vector3f 或 MatrixXd），在使用 STL 容器时无需特别注意。

//+++ 使用对齐分配器
// STL 容器有一个可选的模板参数，即分配器类型。当在固定大小的可向量化 Eigen 类型上使用 STL 容器时，您需要告诉容器使用一个始终在 16 字节对齐（或更高）位置分配内存的分配器。
// 幸运的是，Eigen 确实提供了这样的分配器 Eigen::aligned_allocator。

// 例如，应该使用：
std::map<int, Eigen::Vector4d, std::less<int>, Eigen::aligned_allocator<std::pair<const int, Eigen::Vector4d> > > mp;
// 而不是 
std::map<int, Eigen::Vector4d> mp_;
//这是一个使用Eigen库的4维向量数组的std::map类型，其中键是int类型，值是Eigen::Vector4d类型的数组。
//请注意，第三个参数 `std::less<int>` 只是默认值，但我们必须包含它，因为我们需要指定第四个参数，即分配器类型。

//+++ std::vector 案例
//本节仅适用于 C++98/03 用户。[C++11 （或更高版本）] 用户可以跳过此处。

// 因此，在C++98/03中，由于标准库中的一个bug（后面解释），std::vector的情况更加复杂。
// 为了解决这个问题，我们必须为Eigen::aligned_allocator类型对std::vector进行特例化。
// 在实践中，您必须使用Eigen::aligned_allocator（而不是其他类型的分配器），并包含<Eigen/StdVector>头文件。

// 下面是一个例子：
#include<Eigen/StdVector>
std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> my_vector;

// 解释：`std::vector<Eigen::Vector4d>`的`resize()`方法需要一个`value_type`参数（默认为`value_type()`）。
// 因此，一些`Eigen::Vector4d`对象将被按值传递，这会丢弃任何对齐修饰符，因此可以在未对齐的位置创建`Eigen::Vector4d`。
// 为了避免这种情况，我们看到的唯一解决方案是为`std::vector`进行特殊化，使其在对`Eigen::Vector4d`进行微小修改的情况下工作，以便正确处理这种情况。

// 另一种选择是为Eigen类型专门实现std::vector。这种方法的优点是不需要在整个代码中都使用Eigen::aligned_allocator来声明std::vector。
// 缺点是需要在使用std::vector<Vector2d>等Eigen类型之前定义该特殊化版本。
// 否则，如果不知道该特殊化版本的存在，编译器将使用默认的std::allocator来编译该特定实例，程序很可能会崩溃。如下：
#include<Eigen/StdVector>
/* ... */
EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(Matrix2d)
std::vector<Eigen::Vector2d>


//+ case3：通过值传递Eigen对象
//如果您代码中的某些函数正在通过值传递Eigen对象，例如这样，
//void func(Eigen::Vector4d v);
//那么您需要阅读以下单独的页面：将Eigen对象按值传递给函数。
//请注意，此处Eigen::Vector4d仅用作示例，更一般而言，所有固定大小的可矢量化Eigen类型都会出现此问题

//++ 将Eigen对象按值传递给函数
/*在C++中，通过值传递对象几乎总是一个非常糟糕的主意，因为这意味着无用的复制，应该通过引用传递它们。

在Eigen中，这一点尤为重要：将固定大小的可优化Eigen对象按值传递不仅效率低下，还可能导致程序非法或崩溃！
//! 原因在于，这些Eigen对象具有不被按值传递时尊重的对齐修饰符。

例如，像这样的函数，其中 v 是按值传递的：
void my_function(Eigen::Vector2d v)
需要按照以下方式重写，将v作为常引用传递：

void my_function(const Eigen::Vector2d& v)；

同样，如果你有一个类，其中包含一个Eigen对象：

struct Foo
{
  Eigen::Vector2d v;
};

void my_function(Foo v);
这个函数也需要像这样重写：
void my_function(const Foo& v);
注意，另一方面，返回值为对象的函数则不存在问题。*/


//+ case4：编译器对堆栈对齐做出错误假设（例如Windows上的GCC）
//这是在Windows上使用GCC（例如MinGW或TDM-GCC）的人们的必读内容。
//如果在声明这样的局部变量的无辜函数中有此断言失败，请执行以下操作：
void foo()
{
  Eigen::Quaternionf q;
  //...
}
//那么您需要阅读以下单独的页面：编译器对堆栈对齐做出了错误的假设。
//请注意，此处Eigen::Quaternionf仅用作示例，更一般而言，所有固定大小的可矢量化Eigen类型都会出现此问题。

// ++ 编译器对堆栈对齐做出了错误的假设
/*看来这是GCC的一个已在GCC 4.5中得到修复的bug。如果您遇到了此问题，请升级到GCC 4.5并向我们报告，以便我们更新此页面。
到目前为止，我们仅在Windows上的GCC上遇到过此问题，例如MinGW和TDM-GCC。

通常情况下，像这样的函数中，

void foo()
{
  Eigen::Quaternionf q;
  //...
}
GCC假定栈是16字节对齐的，因此对象q将在一个16字节对齐的位置创建。因此，它不需要特别注意显式对齐对象q，就像Eigen要求的那样。

问题是，在某些特定情况下，在Windows上这个假设可能是错误的，因为栈只保证有4字节对齐。
确实，尽管GCC会确保在main函数中对栈进行对齐，并尽其所能保持栈对齐，但当函数从另一个线程或由使用其他编译器编译的二进制文件中调用时，栈对齐可能会被破坏。
这会导致对象'q'在未对齐的位置创建，使您的程序在未对齐数组上出现断言错误并崩溃。到目前为止，我们已经找到了以下三种解决方案。

//+++ 局部解法
局部解法是指为以下函数添加此属性：

使用 `__attribute__((force_align_arg_pointer))` 关键字可以强制将函数的参数指针对齐。

下面是一个使用该关键字的示例函数：
```c
__attribute__((force_align_arg_pointer)) void foo() {
  Eigen::Quaternionf q;
}
```
在这个示例中，`foo()` 函数的参数指针会被强制对齐，即使它们不是严格必要的。这可能会提高代码的性能，但可能会增加代码的复杂性和可维护性。{
Eigen::Quaternionf q; 表示一个使用 Eigen 库的四元数类型（Quaternionf）的变量 q。//...
}
阅读GCC文档以了解这是什么功能。当然，此操作仅适用于Windows上的GCC，因此为了提高代码的可移植性，您需要将此功能封装在一个宏中，并在其他平台上将其设为空。
这种解决方案的优点是可以精确地选择哪些函数可能存在栈对齐问题。当然，其缺点是必须为每个这样的函数单独进行处理，因此您可能更倾向于以下两种全局解决方案中的一种。

//+++ 全局解决方案

全局解决方案是编辑您的项目，以便在使用GCC的Windows上编译时，将此选项传递给GCC：

-min-coming-stack-boundary=2
解释：这告诉GCC栈只需要对2^2（即4字节）进行对齐，这样GCC就知道在必要时必须特别注意遵守16字节对齐的固定大小可矢量化Eigen类型的要求。

另一个全局性解决方案是将此选项传递给gcc：
-mstackrealign 与为所有函数添加 force_align_arg_pointer 属性具有相同的效果。

//!这些全局性解决方案易于使用，但请注意，它们可能会降低程序的运行速度，因为每个函数都需要额外的前置/后置指令。
*/


//+ 这个断言（最开头说的那个）的一般解释是：
// 固定大小的可向量化 Eigen 对象必须在正确对齐的位置上创建，否则针对它们的 SIMD 指令将发生崩溃。
// 例如，针对 SSE/NEON/MSA/Altivec/VSX 目标需要 16 字节对齐，而针对 AVX 和 AVX512 目标则可能需要分别进行 32 字节和 64 字节对齐。
// Eigen通常会为您处理这些对齐问题，通过为它们设置一个对齐属性，并重载它们的“new”操作符来实现。
//然而，有一些特殊情况可能会使这些对齐设置被覆盖：这些可能是导致该断言的可能原因。

//+ 我不在乎最优向量化，那我该如何去掉那些东西呢？
// 有三种可能的解决方法：
// 1. 使用“DontAlign”选项来处理矩阵、数组、四元数等对象，这样Eigen就不会尝试过度对齐它们，也不会假设任何特殊的对齐方式。
// 缺点是，您将为它们支付未对齐加载/存储的代价，但在现代CPU上，开销要么为零，要么微不足道。上文有一个示例。
// 2. 将EIGEN_MAX_STATIC_ALIGN_BYTES设置为0。这将禁用所有16字节（或更大）的静态对齐代码，同时保持16字节（或更大）的堆栈对齐。
// 这将通过EIGEN_UNALIGNED_VECTORIZE控制的未对齐存储来对固定大小的对象（如Matrix4d）进行向量化，同时保持动态大小的对象（如MatrixXd）的向量化不变。
// 在64字节系统上，您也可以将它设置为16，以禁用32和64字节的过度对齐。但请注意，这会破坏静态对齐的ABI兼容性。
// 3. 或者同时定义EIGEN_DONT_VECTORIZE和EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT。
// 这将保留16字节（或更大）的对齐代码，从而保持ABI兼容性，但完全禁用向量化。

// 如果你想知道为什么定义EIGEN_DONT_VECTORIZE本身并不能禁用16字节（或更高）对齐和断言，这里是解释：
// 这不会禁用断言，因为否则，在启用向量化之前运行良好的代码可能会突然在启用向量化时崩溃。
// 它不会禁用16字节（或更大）对齐，因为这将意味着向量化和非向量化代码的二进制接口（ABI）不兼容。
// 这种ABI兼容性非常重要，即使只是开发内部应用程序，例如，可能希望在同一应用程序中同时提供向量化路径和非向量化路径。


//+ 我该如何检查我的代码在对齐问题方面是否安全？

// 不幸的是，在 C++ 中没有可能在编译时检测出上述任何缺点（尽管静态分析器变得越来越强大，可以检测出其中的一些缺点）。
// 即使在运行时，我们所能做的也只是捕获无效的未对齐分配，并触发本页开头提到的显式断言。
// 因此，如果您的程序在给定的系统上以给定的编译标志运行良好，那么这并不能保证您的代码是安全的。
// 例如，在大多数 64 位系统上缓冲区都以 16 字节边界对齐，因此，如果您未启用 AVX 指令集，则您的代码将正常运行。
// 另一方面，相同的代码可能在迁移到更奇特的平台或启用默认需要 32 字节对齐的 AVX 指令时引发断言。

// 虽然情况并不乐观，但如果你的代码有良好的单元测试覆盖，那么你可以通过将它链接到一个自定义的malloc库，
// 该库只返回8字节对齐的缓冲区，来检查其对齐安全性。这样一来，所有的对齐问题都应该暴露出来。
// 为此，你必须在编译程序时将EIGEN_MALLOC_ALREADY_ALIGNED设置为0。


// 4   我不在乎最佳矢量化，如何摆脱这些东西？
// 三种可能性：
// 使用Matrix，Array，Quaternion等对象的DontAlign选项会给您带来麻烦。
// 这样，Eigen不会尝试对齐它们，因此不会采取任何特殊对齐方式。
// 不利的一面是，您将为它们支付未对齐的加载/存储的成本，但是在现代CPU上，开销为null或边际的。
// 定义EIGEN_DONT_ALIGN_STATICALLY。这将禁用所有16​​字节（或以上）的静态对齐代码，
// 同时保持16字节（或以上）的堆对齐。这具有通过未对齐的存储区（由EIGEN_UNALIGNED_VECTORIZE控制）对固定大小的对象（如Matrix4d）进行矢量化的效果，同时保持动态大小的对象（如MatrixXd）的矢量化不变。
//但是请注意，这会破坏ABI与静态对齐方式默认行为的兼容性。
// 或同时定义EIGEN_DONT_VECTORIZE和EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT。
// 这样可以保留16字节的对齐代码，从而保留ABI兼容性，但完全禁用向量化。
// 如果您想知道为什么定义EIGEN_DONT_VECTORIZE本身并不能禁用16字节对齐和断言，则说明如下：
// 它不会禁用断言，因为如果不执行矢量化，则正常运行的代码将在启用矢量化时突然崩溃。
// 它不会禁用16字节对齐，因为这将意味着矢量化和非矢量化的代码不相互兼容ABI。
// 即使对于仅开发内部应用程序的人，这种ABI兼容性也非常重要，
// 例如，可能希望在同一应用程序中同时具有矢量化路径和非矢量化路径。

} // namespace Section11_AlignmentIssues

} // namespace Chapter1_DenseMatrixAndArrary
#endif