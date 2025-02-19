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
#include <string>
#include <vector>

#include "EbsdLib/Core/EbsdDataArray.hpp"
#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Core/OrientationTransformation.hpp"
#include "EbsdLib/Core/Quaternion.hpp"
#include "EbsdLib/EbsdLib.h"
#include "EbsdLib/Math/Matrix3X1.hpp"
#include "EbsdLib/Math/Matrix3X3.hpp"
#include "EbsdLib/Utilities/PoleFigureUtilities.h"

/*
 * @class LaueOps LaueOps.h OrientationLib/LaueOps/LaueOps.h
 * @brief
 */
class EbsdLib_EXPORT LaueOps
{
public:
  using Self = LaueOps;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  /**
   * @brief Returns the name of the class for LaueOps
   */
  virtual std::string getNameOfClass() const;
  /**
   * @brief Returns the name of the class for LaueOps
   */
  static std::string ClassName();

  virtual ~LaueOps();

  /**
   * @brief GetAllOrientationOps This method returns a vector of each type of LaueOps placed such that the
   * index into the vector is the value of the constant at EbsdLib::CrystalStructure::***
   * @return Vector of LaueOps subclasses.
   */
  static std::vector<LaueOps::Pointer> GetAllOrientationOps();

  /**
   * @brief GetOrientationOpsFromSpaceGroupNumber
   * @param sgNumber
   * @return
   */
  static Pointer GetOrientationOpsFromSpaceGroupNumber(size_t sgNumber);

  /**
   * @brief GetLaueNames Returns the names of the Laue Classes
   * @return
   */
  static std::vector<std::string> GetLaueNames();

  /**
   * @brief getODFSize Returns the number of elements in the ODF array
   * @return
   */
  virtual int getODFSize() const = 0;

  /**
   * @brief getNumSymmetry Returns the internal variables for symSize0, symSize1, symSize2
   * @return
   */
  virtual std::array<int32_t, 3> getNumSymmetry() const = 0;

  /**
   * @brief Returns the number of bins for an MDF Plot assuming 5 degree increments
   * @return
   */
  virtual int getMdfPlotBins() const = 0;

  /**
   * @brief getHasInversion Returns a bool whether the symmetry class is centro-symmetric
   * @return
   */
  virtual bool getHasInversion() const = 0;

  /**
   * @brief getMDFSize Returns the number of elements in the MDF Array
   * @return
   */
  virtual int getMDFSize() const = 0;

  /**
   * @brief getNumSymOps Returns the number of symmetry operators
   * @return
   */
  virtual int getNumSymOps() const = 0;

  /**
   * @brief getSymmetryName Returns the name of the symmetry
   * @return
   */
  virtual std::string getSymmetryName() const = 0;

  /**
   * @brief Returns the number of bins in each of the 3 dimensions
   * @return
   */
  virtual std::array<size_t, 3> getOdfNumBins() const = 0;

  /**
   * @brief calculateMisorientation Finds the misorientation between 2 quaternions and returns the result as an Axis Angle value
   * @param q1 Input Quaternion
   * @param q2 Input Quaternion
   * @return Axis Angle Representation
   */
  virtual OrientationD calculateMisorientation(const QuatD& q1, const QuatD& q2) const = 0;

  /**
   * @brief calculateMisorientation Finds the misorientation between 2 quaternions and returns the result as an Axis Angle value
   * @param q1 Input Quaternion
   * @param q2 Input Quaternion
   * @return Axis Angle Representation
   */
  virtual OrientationF calculateMisorientation(const QuatF& q1, const QuatF& q2) const = 0;

  /**
   * @brief getQuatSymOp Returns the symmetry operator at index i
   * @param i The index into the Symmetry operators array
   * @return The quaternion symmetry operator
   */
  virtual QuatD getQuatSymOp(int i) const = 0;

  /**
   * @brief getRodSymOp Returns a Rodrigues vector based on the symmetry operator at index i
   * @param i Index of the symmetry operator
   * @param r Pointer to store the Rodrigues vector into.
   */
  virtual void getRodSymOp(int i, double* r) const = 0;

  /**
   * @brief Retrieves a specific Symmetry Operator for a giving index
   * @param i The index from the Symmetry Operator Array to retrieve
   * @return void or a Matrix3X3 object.
   */
  virtual void getMatSymOp(int i, double g[3][3]) const = 0;
  virtual void getMatSymOp(int i, float g[3][3]) const = 0;
  virtual EbsdLib::Matrix3X3F getMatSymOpF(int i) const = 0;
  virtual EbsdLib::Matrix3X3D getMatSymOpD(int i) const = 0;

  /**
   * @brief getODFFZRod
   * @param rod
   * @return
   */
  virtual OrientationType getODFFZRod(const OrientationType& rod) const = 0;

  /**
   * @brief getMDFFZRod
   * @param rod
   * @return
   */
  virtual OrientationType getMDFFZRod(const OrientationType& rod) const = 0;

  virtual QuatD getNearestQuat(const QuatD& q1, const QuatD& q2) const = 0;
  virtual QuatF getNearestQuat(const QuatF& q1f, const QuatF& q2f) const = 0;

  /**
   * @brief getFZQuat Returns a Quaternioni that lies in the Fundemental Zone (FZ)
   * @param qr Input Quaternion
   * @return
   */
  virtual QuatD getFZQuat(const QuatD& qr) const;

  /**
   * @brief getMisoBin Returns the misorientation bin that the input Rodregues vector lies in.
   * @param rod
   * @return
   */
  virtual int getMisoBin(const OrientationType& rod) const = 0;

  virtual bool inUnitTriangle(double eta, double chi) const = 0;

  virtual OrientationType determineEulerAngles(double random[3], int choose) const = 0;

  virtual OrientationType randomizeEulerAngles(const OrientationType& euler) const = 0;

  virtual size_t getRandomSymmetryOperatorIndex(int numSymOps) const;

  virtual OrientationType determineRodriguesVector(double random[3], int choose) const = 0;

  virtual int getOdfBin(const OrientationType& rod) const = 0;

  virtual void getSchmidFactorAndSS(double load[3], double& schmidfactor, double angleComps[2], int& slipsys) const = 0;

  virtual void getSchmidFactorAndSS(double load[3], double plane[3], double direction[3], double& schmidfactor, double angleComps[2], int& slipsys) const = 0;

  virtual double getmPrime(const QuatD& q1, const QuatD& q2, double LD[3]) const = 0;

  virtual double getF1(const QuatD& q1, const QuatD& q2, double LD[3], bool maxSF) const = 0;

  virtual double getF1spt(const QuatD& q1, const QuatD& q2, double LD[3], bool maxSF) const = 0;

  virtual double getF7(const QuatD& q1, const QuatD& q2, double LD[3], bool maxSF) const = 0;

  virtual void generateSphereCoordsFromEulers(EbsdLib::FloatArrayType* eulers, EbsdLib::FloatArrayType* c1, EbsdLib::FloatArrayType* c2, EbsdLib::FloatArrayType* c3) const = 0;

  /**
   * @brief generateIPFColor Generates an RGB Color from a Euler Angle and Reference Direction
   * @param eulers Pointer to the 3 component Euler Angle
   * @param refDir Pointer to the 3 Component Reference Direction
   * @param rgb [output] The pointer to store the RGB value
   * @param convertDegrees Are the input angles in Degrees
   */
  virtual EbsdLib::Rgb generateIPFColor(double* eulers, double* refDir, bool convertDegrees) const = 0;

  /**
   * @brief generateIPFColor Generates an RGB Color from a Euler Angle and Reference Direction
   * @param e0 First component of the Euler Angle
   * @param e1 Second component of the Euler Angle
   * @param e2 Third component of the Euler Angle
   * @param dir0 First component of the Reference Direction
   * @param dir1 Second component of the Reference Direction
   * @param dir2 Third component of the Reference Direction
   * @param rgb [output] The pointer to store the RGB value
   * @param convertDegrees Are the input angles in Degrees
   */
  virtual EbsdLib::Rgb generateIPFColor(double e0, double e1, double e2, double dir0, double dir1, double dir2, bool convertDegrees) const = 0;

  /**
   * @brief generateRodriguesColor Generates an RGB Color from a Rodrigues Vector
   * @param r1 First component of the Rodrigues Vector
   * @param r2 Second component of the Rodrigues Vector
   * @param r3 Third component of the Rodrigues Vector
   * @param rgb [output] The pointer to store the RGB value
   */
  virtual EbsdLib::Rgb generateRodriguesColor(double r1, double r2, double r3) const = 0;

  /**
   * @brief generateMisorientationColor Generates a color based on the method developed by C. Schuh and S. Patala.
   * @param q A Quaternion representing the crystal direction
   * @param refDir A Quaternion representing the sample reference direction
   * @return A EbsdLib::Rgb value
   */
  virtual EbsdLib::Rgb generateMisorientationColor(const QuatD& q, const QuatD& refFrame) const;

  /**
   * @brief generatePoleFigure This method will generate a number of pole figures for this crystal symmetry and the Euler
   * angles that are passed in.
   * @param eulers The Euler Angles to generate the pole figure from.
   * @param imageSize The size in Pixels of the final RGB Image.
   * @param numColors The number of colors to use in the RGB Image. Less colors can give the effect of contouring.
   * @return A std::vector of EbsdLib::UInt8ArrayType pointers where each one represents a 2D RGB array that can be used to initialize
   * an image object from other libraries and written out to disk.
   */
  virtual std::vector<EbsdLib::UInt8ArrayType::Pointer> generatePoleFigure(PoleFigureConfiguration_t& config) const = 0;

  /**
   * @brief Returns the names for each of the three standard pole figures that are generated. For example
   *<001>, <011> and <111> for a cubic system
   */
  virtual std::array<std::string, 3> getDefaultPoleFigureNames() const = 0;

protected:
  LaueOps();

  /**
   * @brief calculateMisorientationInternal
   * @param quatsym The Symmetry Quarternions from the specific Laue class
   * @param numsym The number of Symmetry Quaternions
   * @param q1 Input Quaternion 1
   * @param q2 Input Quaternion 2
   * @return Returns Axis-Angle <XYZ>W form.
   */
  virtual OrientationD calculateMisorientationInternal(const std::vector<QuatD>& quatsym, const QuatD& q1, const QuatD& q2) const;

  OrientationType _calcRodNearestOrigin(const std::vector<OrientationD>& rodsym, const OrientationType& rod) const;

  QuatD _calcNearestQuat(const std::vector<QuatD>& quatsym, const QuatD& q1, const QuatD& q2) const;

  QuatD _calcQuatNearestOrigin(const std::vector<QuatD>& quatsym, const QuatD& qr) const;

  int _calcMisoBin(double dim[3], double bins[3], double step[3], const OrientationType& homochoric) const;
  void _calcDetermineHomochoricValues(double random[3], double init[3], double step[3], int32_t phi[3], double& r1, double& r2, double& r3) const;
  int _calcODFBin(double dim[3], double bins[3], double step[3], const OrientationType& homochoric) const;

public:
  LaueOps(const LaueOps&) = delete;            // Copy Constructor Not Implemented
  LaueOps(LaueOps&&) = delete;                 // Move Constructor Not Implemented
  LaueOps& operator=(const LaueOps&) = delete; // Copy Assignment Not Implemented
  LaueOps& operator=(LaueOps&&) = delete;      // Move Assignment Not Implemented
};
