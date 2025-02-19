/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <limits>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Eigen>

#include "EbsdLib/Core/OrientationRepresentation.h"
#include "EbsdLib/Core/OrientationTransformation.hpp"
#include "EbsdLib/Core/Quaternion.hpp"
#include "EbsdLib/EbsdLib.h"
#include "EbsdLib/Math/EbsdMatrixMath.h"
#include "EbsdLib/Math/Matrix3X1.hpp"
#include "EbsdLib/Math/Matrix3X3.hpp"

#include "UnitTestSupport.hpp"

#include "EbsdLib/Test/EbsdLibTestFileLocations.h"

constexpr float DREAM3D_PASSIVE = 1.0F;
constexpr float DREAM3D_ACTIVE = -1.0F;

class QuaternionTest
{
public:
  QuaternionTest() = default;
  virtual ~QuaternionTest() = default;

  EBSD_GET_NAME_OF_CLASS_DECL(QuaternionTest)

  // -----------------------------------------------------------------------------
  void RemoveTestFiles()
  {
#if REMOVE_TEST_FILES
// fs::remove();
#endif
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestEbsdMatrixMath()
  {
    using VectorMapType = Eigen::Map<Eigen::Vector3f>;

    {
      std::array<float, 3> dir = {1.0f, 2.0f, 3.0f};

      VectorMapType array(dir.data());
      array.normalize();
      array = array * -1.0f;
    }

    {
      std::array<float, 3> dir = {1.0f, 2.0f, 3.0f};
      EbsdMatrixMath::Normalize3x1(dir.data());
      EbsdMatrixMath::Multiply3x1withConstant(dir.data(), -1.0f);
    }
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void TestQuaternion()
  {
    QuatF p(1.0f, 0.0f, 0.0f, 1.0f);
    QuatF q(0.0f, 1.0f, 0.0f, 2.0f);
    QuatF out(0.0f, 0.0f, 0.0f, 0.0f);
    QuatF out2(10.0f, 20.0f, 30.0f, 40.0f);

    out2.negate();
    DREAM3D_REQUIRE_EQUAL(out2.x(), -10.0)
    DREAM3D_REQUIRE_EQUAL(out2.y(), -20.0)
    DREAM3D_REQUIRE_EQUAL(out2.z(), -30.0)
    DREAM3D_REQUIRE_EQUAL(out2.w(), -40.0)

    out = p;
    DREAM3D_REQUIRE_EQUAL(p.x(), out.x())
    DREAM3D_REQUIRE_EQUAL(p.y(), out.y())
    DREAM3D_REQUIRE_EQUAL(p.z(), out.z())
    DREAM3D_REQUIRE_EQUAL(p.w(), out.w())

    out = QuatF::identity();
    DREAM3D_REQUIRE_EQUAL(out.x(), 0.0)
    DREAM3D_REQUIRE_EQUAL(out.y(), 0.0)
    DREAM3D_REQUIRE_EQUAL(out.z(), 0.0)
    DREAM3D_REQUIRE_EQUAL(out.w(), 1.0)

    out = QuatF(-10.5f, -1.5f, -30.66f, -40.987f);
    out.elementWiseAbs();
    DREAM3D_REQUIRE_EQUAL(out.x(), 10.5f)
    DREAM3D_REQUIRE_EQUAL(out.y(), 1.5f)
    DREAM3D_REQUIRE_EQUAL(out.z(), 30.66f)
    DREAM3D_REQUIRE_EQUAL(out.w(), 40.987f)

    out = QuatF(10.0f, 20.0f, 30.0f, 40.0f);
    out.scalarMultiply(-1.0f);
    DREAM3D_REQUIRE_EQUAL(out.x(), -10.0)
    DREAM3D_REQUIRE_EQUAL(out.y(), -20.0)
    DREAM3D_REQUIRE_EQUAL(out.z(), -30.0)
    DREAM3D_REQUIRE_EQUAL(out.w(), -40.0)

    out.scalarDivide(-1.0f).scalarAdd(50.0f);
    DREAM3D_REQUIRE_EQUAL(out.x(), 60.0)
    DREAM3D_REQUIRE_EQUAL(out.y(), 70.0)
    DREAM3D_REQUIRE_EQUAL(out.z(), 80.0)
    DREAM3D_REQUIRE_EQUAL(out.w(), 90.0)

    out.elementWiseAssign(5.0f);
    DREAM3D_REQUIRE_EQUAL(out.x(), 5.0)
    DREAM3D_REQUIRE_EQUAL(out.y(), 5.0)
    DREAM3D_REQUIRE_EQUAL(out.z(), 5.0)
    DREAM3D_REQUIRE_EQUAL(out.w(), 5.0)

    out.negate();
    out = p + q;
    out = p - q;

    // Conjugate Tests where conjugate of a Quaternion is q*
    // (q*)* = q

    p.x() = 1.0f;
    p.y() = 2.0f;
    p.z() = 3.0f;
    p.w() = 1.0f;

    p = p.conjugate();
    DREAM3D_REQUIRE_EQUAL(p.x(), -1.0)
    DREAM3D_REQUIRE_EQUAL(p.y(), -2.0)
    DREAM3D_REQUIRE_EQUAL(p.z(), -3.0)
    DREAM3D_REQUIRE_EQUAL(p.w(), 1.0)
    p = p.conjugate();
    DREAM3D_REQUIRE_EQUAL(p.x(), 1.0)
    DREAM3D_REQUIRE_EQUAL(p.y(), 2.0)
    DREAM3D_REQUIRE_EQUAL(p.z(), 3.0)
    DREAM3D_REQUIRE_EQUAL(p.w(), 1.0)

    // (pq)* = q*p*
    q.x() = 1.0f;
    q.y() = 0.0f;
    q.z() = 1.0f;
    q.w() = 1.0f;
    out = p * q;
    out = out.conjugate();
    p = p.conjugate();
    q = q.conjugate();
    out2 = q * p;
    DREAM3D_REQUIRE_EQUAL(out.x(), out2.x())
    DREAM3D_REQUIRE_EQUAL(out.y(), out2.y())
    DREAM3D_REQUIRE_EQUAL(out.z(), out2.z())
    DREAM3D_REQUIRE_EQUAL(out.w(), out2.w())

    //(p+q)* = p*+q*
    p.x() = 1.0f;
    p.y() = 2.0f;
    p.z() = 3.0f;
    p.w() = 1.0f;
    q.x() = 1.0f;
    q.y() = 0.0f;
    q.z() = 1.0f;
    q.w() = 1.0f;
    out = p + q;
    out = out.conjugate();
    p = p.conjugate();
    q = q.conjugate();
    out2 = p + q;

    DREAM3D_REQUIRE_EQUAL(out.x(), out2.x())
    DREAM3D_REQUIRE_EQUAL(out.y(), out2.y())
    DREAM3D_REQUIRE_EQUAL(out.z(), out2.z())
    DREAM3D_REQUIRE_EQUAL(out.w(), out2.w())

    // Multiplication Test
    // pq != qp
    p.x() = 1.0f;
    p.y() = 0.0f;
    p.z() = 0.0f;
    p.w() = 1.0f;

    q.x() = 0.0f;
    q.y() = 1.0f;
    q.z() = 0.0f;
    q.w() = 2.0f;

    out = p * q;
    DREAM3D_REQUIRE_EQUAL(out.x(), 2.0)
    DREAM3D_REQUIRE_EQUAL(out.y(), 1.0)
    DREAM3D_REQUIRE_EQUAL(out.z(), 1.0)
    DREAM3D_REQUIRE_EQUAL(out.w(), 2.0)

    out = q * p;
    DREAM3D_REQUIRE_EQUAL(out.x(), 2.0)
    DREAM3D_REQUIRE_EQUAL(out.y(), 1.0)
    DREAM3D_REQUIRE_EQUAL(out.z(), -1.0)
    DREAM3D_REQUIRE_EQUAL(out.w(), 2.0)

    // Norm Test
    // N(q*) = N(q)
    p.x() = 1.0f;
    p.y() = 0.0f;
    p.z() = 0.0f;
    p.w() = 1.0f;
    q.x() = 0.0f;
    q.y() = 1.0f;
    q.z() = 0.0f;
    q.w() = 2.0f;
    float norm = p.norm();
    p = p.conjugate();
    float cnorm = p.norm();
    DREAM3D_REQUIRE_EQUAL(norm, cnorm)

    // Length and Unit Quaternion Tests
    p.x() = 2.0f;
    p.y() = 2.0f;
    p.z() = 2.0f;
    p.w() = 2.0f;
    float length = p.length();
    DREAM3D_REQUIRE_EQUAL(length, 4.0);
    p = p.unitQuaternion();
    DREAM3D_REQUIRE_EQUAL(p.x(), 0.5)
    DREAM3D_REQUIRE_EQUAL(p.y(), 0.5)
    DREAM3D_REQUIRE_EQUAL(p.z(), 0.5)
    DREAM3D_REQUIRE_EQUAL(p.w(), 0.5)

    p.x() = 1.0;
    p.y() = 2.0;
    p.z() = 3.0;
    p.w() = 0.0;
    std::array<float, 3> vec = {0.0f, 0.0f, 0.0f};
    std::array<float, 3> ovec = {0.0f, 0.0f, 0.0f};

    vec = p.getMisorientationVector();
    bool pass = EbsdLibMath::closeEnough(vec[0], 3.14159274f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true);

    pass = EbsdLibMath::closeEnough(vec[1], 6.28318548f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    pass = EbsdLibMath::closeEnough(vec[2], 9.42477798f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    // Test point r=[100] rotated 120Deg @ [111] axis
    vec = {1.0F, 0.0F, 0.0F};
    // The quaternion Representation for 120@[111]
    q = QuatF(0.5f, 0.5f, 0.5f, 0.5f);

    // Passive Rotation
    ovec = q.multiplyByVector(vec.data());

    pass = EbsdLibMath::closeEnough(ovec[0], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true);

    pass = EbsdLibMath::closeEnough(ovec[1], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    pass = EbsdLibMath::closeEnough(ovec[2], 1.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    // Active Rotation
    q = q.conjugate();
    ovec = q.multiplyByVector(vec.data());

    pass = EbsdLibMath::closeEnough(ovec[0], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true);

    pass = EbsdLibMath::closeEnough(ovec[1], 1.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    pass = EbsdLibMath::closeEnough(ovec[2], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    // 90@[100]
    q.x() = 0.7071067811865475f;
    q.y() = 0.0f;
    q.z() = 0.0f;
    q.w() = 0.7071067811865476f;

    vec[0] = 0.0;
    vec[1] = 1.0;
    vec[2] = 0.0;
    // Passive Rotation
    ovec = q.multiplyByVector(vec.data());

    pass = EbsdLibMath::closeEnough(ovec[0], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true);

    pass = EbsdLibMath::closeEnough(ovec[1], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    pass = EbsdLibMath::closeEnough(ovec[2], -1.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    // active Rotation
    q = q.conjugate();
    ovec = q.multiplyByVector(vec.data());

    pass = EbsdLibMath::closeEnough(ovec[0], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true);

    pass = EbsdLibMath::closeEnough(ovec[1], 0.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    pass = EbsdLibMath::closeEnough(ovec[2], 1.0f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    // Rotate a vector by a quaternion
    Quaternion<float> quat(0.32732683f, -0.54554468f, 0.76376259f, 0.10910894f);

    vec = {0.26726124f, -0.53452247f, 0.80178368f};
    ovec = quat.rotateVector(vec.data(), static_cast<int>(DREAM3D_PASSIVE));

    pass = EbsdLibMath::closeEnough(ovec[0], 0.381802f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true);

    pass = EbsdLibMath::closeEnough(ovec[1], -0.572703f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    pass = EbsdLibMath::closeEnough(ovec[2], 0.725423f, 1.0E-4f);
    DREAM3D_REQUIRE_EQUAL(pass, true)

    std::cout << ovec[0] << ", " << ovec[1] << ", " << ovec[2] << std::endl;
  }

  void Matrix3x3Test()
  {
    EbsdLib::Matrix3X1D vec0 = {1.0, 0.0, 0.0};
    EbsdLib::Matrix3X1D vec1 = {0.0, 1.0, 0.0};
    EbsdLib::Matrix3X1D vec2 = {-1.0, 0.0, 0.0};
    EbsdLib::Matrix3X1D vec3 = {1.0, 1.0, 0.0};

    double cosTheta = vec0.cosTheta(vec1);
    DREAM3D_REQUIRE_EQUAL(cosTheta, 0.0)
    cosTheta = vec0.cosTheta(vec2);
    DREAM3D_REQUIRE_EQUAL(cosTheta, -1.0)
    cosTheta = vec0.cosTheta(vec0);
    DREAM3D_REQUIRE_EQUAL(cosTheta, 1.0)

    cosTheta = vec0.cosTheta(vec3);
    bool pass = EbsdLibMath::closeEnough(cosTheta, 0.70710678118654746, 1.0E-4);
    DREAM3D_REQUIRE_EQUAL(pass, true);
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void operator()()
  {
    std::cout << "<===== Start " << getNameOfClass() << std::endl;
    int err = EXIT_SUCCESS;
    DREAM3D_REGISTER_TEST(TestEbsdMatrixMath())
    DREAM3D_REGISTER_TEST(TestQuaternion())
    DREAM3D_REGISTER_TEST(RemoveTestFiles())
    DREAM3D_REGISTER_TEST(Matrix3x3Test())
  }

public:
  QuaternionTest(const QuaternionTest&) = delete;            // Copy Constructor Not Implemented
  QuaternionTest(QuaternionTest&&) = delete;                 // Move Constructor Not Implemented
  QuaternionTest& operator=(const QuaternionTest&) = delete; // Copy Assignment Not Implemented
  QuaternionTest& operator=(QuaternionTest&&) = delete;      // Move Assignment Not Implemented
};
