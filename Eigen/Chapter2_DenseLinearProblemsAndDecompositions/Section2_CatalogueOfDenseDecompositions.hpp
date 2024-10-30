#ifndef CATALOGUE_OF_DENSE_DECOMPOSITIONS_HPP
#define CATALOGUE_OF_DENSE_DECOMPOSITIONS_HPP
#include "HeaderFile.h"
namespace Chapter2_DenseLinearProblemsAndDecompositions
{

namespace Section2_CatalogueOfDenseDecompositions
{
//+ 此页面显示了Eigen提供的稠密矩阵分解的目录。

// 分解	            矩阵要求  速度	 算法的可靠性和准确性	可计算矩阵秩	允许计算（除了线性求解） Eigen提供的线性求解器	Eigen实施的成熟度   最佳化

// 这个分解类似于没有行交换的A=LU
// PartialPivLU	    可逆的	  快速	 取决于条件数	        --	            --	                    是	                优秀的	          Blocking，隐式MT

//这个分解类似于PA=LU
// FullPivLU	    --	      慢	久经考验	            是	            --	                    是	                优秀的	           --

// 一般的矩阵QR分解要求特征向量线性无关，不知道为啥这个QR没要求
// HouseholderQR	--	     快速	取决于条件数	        --	            正交化	                 是	                优秀的	            Blocking

//列主元QR分解？
// ColPivHouseholderQR	--	 快速	好	                    是	            正交化	                是	                优秀的	            --

//全主元QR分解
// FullPivHouseholderQR	--	 慢	    久经考验	            是	            正交化	                是	                平均	            --

//
// CompleteOrthogonalDecomposition	快  好                 是	            正交化	                是	                优秀的	            --  

// LLT	            正定	 非常快	  取决于条件数	        --	                --	                是	                优秀的	            Blocking

// LDLT	            正或负半定1	非常快	   好    	        --	                --	                是	                优秀的	            Soon: blocking


//+ 奇异值和特征值分解

// BDCSVD（分而治之）	--	最快的SVD算法之一	优秀的	        是	         奇异值/向量，最小二乘	   是（并且最小二乘）	优秀的	            blocked  双对角化

// JacobiSVD（双面）	--	慢（但对于小型矩阵则快）已证明3	    是	            奇异值/向量，最小二乘	是的（并且最小二乘）	优秀的	        R-SVD

// 自伴特征算法	
// SelfAdjointEigenSolver	自伴	快速平均2	好	            是	        特征值/向量	            --	                    优秀的	        2x2和3x3的解析解

// 复特征值求解器
// ComplexEigenSolver		平方	慢-非常慢2	取决于条件数	是	        特征值/向量	            --	                    一般	            --

// 特征解算器
// EigenSolver		正方形和实数	平均-慢  	取决于条件数	是	            特征值/向量	            --	                 一般	             --

// 广义自我伴随特征求解
// GeneralizedSelfAdjointEigenSolver	方阵	快速-一般2	取决于条件数	 --	广义特征值/向量	        --	                好	                --

// 辅助分解

// RealSchur	       正方形和实数	一般-慢2	取决于条件数	是	            --	                    --	                一般	             --

// ComplexSchur	        正方形	慢-非常慢2	取决于条件数	    是	            --	                    --	                一般	             --

// Tridiagonalization	   自伴	快速	    好	                --	            --	                    --	                好	            Soon: blocking

// HessenbergDecomposition	正方形	平均	好	                --	            --	                    --	                好	            Soon: blocking

// 笔记：
// 1: LDLT算法存在两种变体。Eigen的版本会生成一个纯对角的D矩阵，因此它无法处理非定矩阵，而Lapack的版本会生成一个块对角的D矩阵。
// 2： 特征值、奇异值分解和Schur分解都依赖于迭代算法。它们的收敛速度取决于特征值的分离程度。
// 3： 我们的JacobiSVD是双向的，对于方阵来说可以保证精确度和优化的精度。对于非方阵，我们需要先使用QR预处理器。默认选择ColPivHouseholderQR已经非常可靠，但如果您需要证明，请使用FullPivHouseholderQR。

//+ 术语：
//* 自共轭（Selfadjoint）：
//     对于实矩阵，“自共轭”与“对称”是同义词。对于一个复矩阵来说，“自共轭”与“赫尔米特”是同义词。更一般地，一个矩阵A是自共轭的，当且仅当它等于它的共轭转置A∗。共轭转置也被称为共轭转置。
//* 正定/负定（Positive/negative definite）：
//     一个自共轭矩阵A是正定的，当且仅当对于任何非零向量v，都有v∗Av>0。同样地，它是负定的，当且仅当对于任何非零向量v，都有v∗Av<0。
//* 半正定/半负定（Positive/negative semidefinite）：
//     一个自共轭矩阵A是正定半定的，当且仅当对于任何非零向量v，都有v∗Av≥0。同样地，它是负定半定的，当且仅当对于任何非零向量v，都有v∗Av≤0。

//* 分块（Blocking）：意味着算法可以按块工作，从而为大型矩阵保证良好的性能扩展。
//* 隐式多线程（Implicit Multi Threading，MT）：意味着算法可以通过OpenMP利用多核处理器。“隐式”意味着算法本身不是并行化的，而是依赖于并行化的矩阵乘法子程序。
//* 显式多线程（Explicit Multi Threading，MT）：意味着算法通过OpenMP显式地并行化以利用多核处理器。
//* 元展开（Meta-unroller）：意味着对于非常小的固定尺寸矩阵，算法会自动和显式地展开。} // namespace Section2_CatalogueOfDenseDecompositions
} // namespace Chapter2_DenseLinearProblemsAndDecompositions

#endif