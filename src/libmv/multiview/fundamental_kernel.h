// Copyright (c) 2009 libmv authors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// TODO(keir): This code is plain unfinished! Doesn't even compile!

#ifndef LIBMV_MULTIVIEW_FUNDAMENTAL_KERNEL_H_
#define LIBMV_MULTIVIEW_FUNDAMENTAL_KERNEL_H_

#include "libmv/base/vector.h"
#include "libmv/multiview/two_view_kernel.h"
#include "libmv/numeric/numeric.h"
#include "libmv/logging/logging.h"

namespace libmv {
namespace fundamental {
namespace kernel {

// TODO(keir): Templatize error functions to work with autodiff (only F).
struct SampsonError {
  static double Error(const Mat3 &F, const Vec2 &x1, const Vec2 &x2) {
    Vec3 x(x1(0), x1(1), 1.0);
    Vec3 y(x2(0), x2(1), 1.0);
    // See page 287 equation (11.9) of HZ.
    Vec3 F_x = F * x;
    Vec3 Ft_y = F.transpose() * y;
    return Square(y.dot(F_x)) / (  F_x.start<2>().squaredNorm()
                                + Ft_y.start<2>().squaredNorm());
  }
};

struct SymmetricEpipolarDistanceError {
  static double Error(const Mat3 &F, const Vec2 &x1, const Vec2 &x2) {
    Vec3 x(x1(0), x1(1), 1.0);
    Vec3 y(x2(0), x2(1), 1.0);
    // See page 288 equation (11.10) of HZ.
    Vec3 F_x = F * x;
    Vec3 Ft_y = F.transpose() * y;
    return Square(y.dot(F_x)) * ( 1  / F_x.start<2>().squaredNorm()
                                + 1 / Ft_y.start<2>().squaredNorm())
      / 4.0;  // The divide by 4 is to make this match the sampson distance.
  }
};

/**
 * Seven-point algorithm for solving for the fundamental matrix from point
 * correspondences. See page 281 in HZ, though oddly they use a different
 * equation: \f$det(\alpha F_1 + (1-\alpha)F_2) = 0\f$. Since \f$F_1\f$ and
 * \f$F2\f$ are projective, there's no need to balance the relative scale.
 * Instead, here, the simpler equation is solved: \f$det(F_1 + \alpha F_2) =
 * 0\f$.
 *
 * \see http://www.cs.unc.edu/~marc/tutorial/node55.html
 */
struct SevenPointSolver {
  enum { MINIMUM_SAMPLES = 7 };
  static void Solve(const Mat &x1, const Mat &x2, vector<Mat3> *F);
};

struct EightPointSolver {
  enum { MINIMUM_SAMPLES = 8 };
  static void Solve(const Mat &x1, const Mat &x2, vector<Mat3> *Fs);
};

struct Unnormalizer {
  static void Unnormalize(const Mat3 &T1, const Mat3 &T2, Mat3 *H) {
    *H = T2.transpose() * (*H) * T1;
  }
};

typedef two_view::kernel::Kernel<SevenPointSolver, SampsonError, Mat3>
  SevenPointKernel;

typedef two_view::kernel::Kernel<EightPointSolver, SampsonError, Mat3>
  EightPointKernel;

typedef two_view::kernel::Kernel<
    two_view::kernel::NormalizedSolver<SevenPointSolver, Unnormalizer>,
    SampsonError,
    Mat3>
  NormalizedSevenPointKernel;

typedef two_view::kernel::Kernel<
    two_view::kernel::NormalizedSolver<EightPointSolver, Unnormalizer>,
    SampsonError,
    Mat3>
  NormalizedEightPointKernel;

// Set the default kernel to normalized 7 point, because it is the fastest (in
// a robust estimation context) and most robust of the above kernels.
typedef NormalizedSevenPointKernel Kernel;

// TODO(keir): Convert this to a solver that enforces the essential
// constrsaints; in particular det(F) = 0 and the two nonzero singular values
// are equal.
typedef two_view::kernel::Kernel<EightPointSolver,
                                 SampsonError,
                                 Mat3>
  EssentialKernel;

}  // namespace kernel
}  // namespace fundamental
}  // namespace libmv

#endif  // LIBMV_MULTIVIEW_FUNDAMENTAL_KERNEL_H_
