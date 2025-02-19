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
#pragma once

#include <memory>

#include "EbsdLib/Core/EbsdDataArray.hpp"
#include "EbsdLib/EbsdLib.h"
#include "EbsdLib/LaueOps/LaueOps.h"
#include "EbsdLib/Math/Matrix3X1.hpp"
#include "EbsdLib/Math/Matrix3X3.hpp"

/**
 * @class TriclinicOps TriclinicOps.h DREAM3DLib/Common/LaueOps/TriclinicOps.h
 * @brief
 *
 *
 * @date May 5, 2011
 * @version 1.0
 */
class EbsdLib_EXPORT TriclinicOps : public LaueOps
{
public:
  using Self = TriclinicOps;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  /**
   * @brief Returns the name of the class for TriclinicOps
   */
  virtual std::string getNameOfClass() const override;

  /**
   * @brief Returns the name of the class for TriclinicOps
   */
  static std::string ClassName();

  static Pointer New();

  TriclinicOps();

  virtual ~TriclinicOps() override;

  /**
   * @brief getHasInversion Returns if this Laue class has inversion
   * @return
   */
  bool getHasInversion() const override;

    /**
   * @brief getODFSize Returns the number of ODF bins
   * @return
   */
  int getODFSize() const override;

/**
   * @brief getNumSymmetry Returns the internal variables for symSize0, symSize1, symSize2
   * @return
   */
 std::array<int32_t, 3> getNumSymmetry() const override;

  /**
   * @brief getMDFSize Returns the number of MDF bins
   * @return
   */
  int getMDFSize() const override;

  /**
   * @brief Returns the number of bins for an MDF Plot assuming 5 degree increments
   * @return
   */
  int getMdfPlotBins() const override;

  /**
   * @brief getNumSymOps Returns the number of symmetry operators
   * @return
   */
  int getNumSymOps() const override;

  /**
   * @brief getSymmetryName Returns the name of the Laue class
   * @return
   */
  std::string getSymmetryName() const override;

  /**
   * @brief Returns the number of bins in each of the 3 dimensions
   * @return
   */
  std::array<size_t, 3> getOdfNumBins() const override;

  /**
   * @brief calculateMisorientation Finds the misorientation between 2 quaternions and returns the result as an Axis Angle value
   * @param q1 Input Quaternion
   * @param q2 Input Quaternion
   * @return Axis Angle Representation
   */
  virtual OrientationD calculateMisorientation(const QuatD& q1, const QuatD& q2) const override;

  /**
   * @brief calculateMisorientation Finds the misorientation between 2 quaternions and returns the result as an Axis Angle value
   * @param q1 Input Quaternion
   * @param q2 Input Quaternion
   * @return Axis Angle Representation
   */
  virtual OrientationF calculateMisorientation(const QuatF& q1, const QuatF& q2) const override;

  QuatD getQuatSymOp(int i) const override;
  void getRodSymOp(int i, double* r) const override;

  /**
   * @brief Retrieves a specific Symmetry Operator for a giving index
   * @param i The index from the Symmetry Operator Array to retrieve
   * @return void or a Matrix3X3 object.
   */
  void getMatSymOp(int i, double g[3][3]) const override;
  void getMatSymOp(int i, float g[3][3]) const override;
  EbsdLib::Matrix3X3F getMatSymOpF(int i) const override;
  EbsdLib::Matrix3X3D getMatSymOpD(int i) const override;

  OrientationType getODFFZRod(const OrientationType& rod) const override;
  OrientationType getMDFFZRod(const OrientationType& rod) const override;

  QuatD getNearestQuat(const QuatD& q1, const QuatD& q2) const override;
  QuatF getNearestQuat(const QuatF& q1f, const QuatF& q2f) const override;

  int getMisoBin(const OrientationType& rod) const override;
  bool inUnitTriangle(double eta, double chi) const override;
  OrientationType determineEulerAngles(double random[3], int choose) const override;
  OrientationType randomizeEulerAngles(const OrientationType& euler) const override;
  OrientationType determineRodriguesVector(double random[3], int choose) const override;
  int getOdfBin(const OrientationType& rod) const override;
  void getSchmidFactorAndSS(double load[3], double& schmidfactor, double angleComps[2], int& slipsys) const override;
  void getSchmidFactorAndSS(double load[3], double plane[3], double direction[3], double& schmidfactor, double angleComps[2], int& slipsys) const override;
  double getmPrime(const QuatD& q1, const QuatD& q2, double LD[3]) const override;
  double getF1(const QuatD& q1, const QuatD& q2, double LD[3], bool maxSF) const override;
  double getF1spt(const QuatD& q1, const QuatD& q2, double LD[3], bool maxSF) const override;
  double getF7(const QuatD& q1, const QuatD& q2, double LD[3], bool maxSF) const override;

  void generateSphereCoordsFromEulers(EbsdLib::FloatArrayType* eulers, EbsdLib::FloatArrayType* c1, EbsdLib::FloatArrayType* c2, EbsdLib::FloatArrayType* c3) const override;

  /**
   * @brief generateIPFColor Generates an RGB Color from a Euler Angle and Reference Direction
   * @param eulers Pointer to the 3 component Euler Angle
   * @param refDir Pointer to the 3 Component Reference Direction
   * @param convertDegrees Are the input angles in Degrees
   * @return Returns the ARGB Quadruplet EbsdLib::Rgb
   */
  EbsdLib::Rgb generateIPFColor(double* eulers, double* refDir, bool convertDegrees) const override;

  /**
   * @brief generateIPFColor Generates an RGB Color from a Euler Angle and Reference Direction
   * @param e0 First component of the Euler Angle
   * @param e1 Second component of the Euler Angle
   * @param e2 Third component of the Euler Angle
   * @param dir0 First component of the Reference Direction
   * @param dir1 Second component of the Reference Direction
   * @param dir2 Third component of the Reference Direction
   * @param convertDegrees Are the input angles in Degrees
   * @return Returns the ARGB Quadruplet EbsdLib::Rgb
   */
  EbsdLib::Rgb generateIPFColor(double e0, double e1, double phi2, double dir0, double dir1, double dir2, bool convertDegrees) const override;

  /**
   * @brief generateRodriguesColor Generates an RGB Color from a Rodrigues Vector
   * @param r1 First component of the Rodrigues Vector
   * @param r2 Second component of the Rodrigues Vector
   * @param r3 Third component of the Rodrigues Vector
   * @return Returns the ARGB Quadruplet EbsdLib::Rgb
   */
  EbsdLib::Rgb generateRodriguesColor(double r1, double r2, double r3) const override;


  /**
   * @brief generatePoleFigure This method will generate a number of pole figures for this crystal symmetry and the Euler
   * angles that are passed in.
   * @param eulers The Euler Angles to generate the pole figure from.
   * @param imageSize The size in Pixels of the final RGB Image.
   * @param numColors The number of colors to use in the RGB Image. Less colors can give the effect of contouring.
   * @return A std::vector of EbsdLib::UInt8ArrayType pointers where each one represents a 2D RGB array that can be used to initialize
   * an image object from other libraries and written out to disk.
   */
  std::vector<EbsdLib::UInt8ArrayType::Pointer> generatePoleFigure(PoleFigureConfiguration_t& config) const override;

  /**
   * @brief Returns the names for each of the three standard pole figures that are generated. For example
   *<001>, <011> and <111> for a cubic system
   */
 std::array<std::string, 3> getDefaultPoleFigureNames() const override;

  /**
   * @brief generateStandardTriangle Generates an RGBA array that is a color "Standard" IPF Triangle Legend used for IPF Color Maps.
   * @return
   */
  EbsdLib::UInt8ArrayType::Pointer generateIPFTriangleLegend(int imageDim) const;

protected:
public:
  TriclinicOps(const TriclinicOps&) = delete;            // Copy Constructor Not Implemented
  TriclinicOps(TriclinicOps&&) = delete;                 // Move Constructor Not Implemented
  TriclinicOps& operator=(const TriclinicOps&) = delete; // Copy Assignment Not Implemented
  TriclinicOps& operator=(TriclinicOps&&) = delete;      // Move Assignment Not Implemented
};
