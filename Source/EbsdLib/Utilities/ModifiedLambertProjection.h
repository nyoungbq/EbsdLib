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
#include "EbsdLib/Utilities/PoleFigureUtilities.h"

#ifdef DATA_ARRAY_ENABLE_HDF5_IO
#include "H5Support/H5SupportTypeDefs.h"
#endif

/**
 * @class ModifiedLambertProjection ModifiedLambertProjection.h DREAM3DLib/Common/ModifiedLambertProjection.h
 * @brief  This class holds a pair of Modified Lambert Projection images. Based off the paper
 * Rosca, D. (2010) New uniform grids on the sphere. Astron Astrophys A63, 520–525.
 * @date July 30, 2013
 * @version 1.0
 */
class EbsdLib_EXPORT ModifiedLambertProjection
{
public:
  using Self = ModifiedLambertProjection;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static Pointer New();

  /**
   * @brief Returns the name of the class for ModifiedLambertProjection
   */
  std::string getNameOfClass() const;
  /**
   * @brief Returns the name of the class for ModifiedLambertProjection
   */
  static std::string ClassName();

  virtual ~ModifiedLambertProjection();

  enum Square
  {
    NorthSquare = 0,
    SouthSquare = 1
  };

  /**
   * @brief CreateProjectionFromXYZCoords This static method creates the north and south squares based on the XYZ coordinates
   * in the 'coords' parameter. The XYZ coordinates are on the unit sphere but are true cartesian coordinates and NOT
   * spherical coordinates.
   * @param coords The XYZ cartesian coords that are all on the Unit Sphere (Radius = 1)
   * @param dimension The Dimension of the modified lambert projections images
   * @param resolution The Spacing of the modified lambert projections
   * @param sphereRadius The radius of the sphere from where the coordinates are coming from.
   * @return
   */
  static Pointer LambertBallToSquare(EbsdLib::FloatArrayType* coords, int dimension, float sphereRadius);

  /**
   * @brief Getter property for Dimension
   * @return Value of Dimension
   */
  int getDimension() const;

  /**
   * @brief Getter property for StepSize
   * @return Value of StepSize
   */
  float getStepSize() const;

  /**
   * @brief Getter property for SphereRadius
   * @return Value of SphereRadius
   */
  float getSphereRadius() const;

  /**
   * @brief Getter property for NorthSquare
   * @return Value of NorthSquare
   */
  EbsdLib::DoubleArrayType::Pointer getNorthSquare() const;

  /**
   * @brief Getter property for SouthSquare
   * @return Value of SouthSquare
   */
  EbsdLib::DoubleArrayType::Pointer getSouthSquare() const;

  /**
   * @brief initializeSquares
   * @param dims
   * @param resolution
   * @param sphereRadius
   */
  void initializeSquares(int dims, float sphereRadius);

#ifdef DATA_ARRAY_ENABLE_HDF5_IO
  virtual int writeHDF5Data(hid_t groupId);
  virtual int readHDF5Data(hid_t groupId);
#endif

  /**
   * @brief addValue
   * @param square
   * @param sqCoord
   * @param value
   */
  void addInterpolatedValues(Square square, float* sqCoord, double value);

  /**
   * @brief addValue
   * @param square
   * @param index
   * @param value
   */
  void addValue(Square square, int index, double value);

  /**
   * @brief This function sets the value of a bin in the lambert projection
   * @param square The North or South Squares
   * @param index The index into the array
   * @param value The value to set
   */
  void setValue(Square square, int index, double value);

  /**
   * @brief getValue
   * @param square
   * @param index
   * @return
   */
  double getValue(Square square, int index);

  /**
   * @brief getInterpolatedValue
   * @param square
   * @param sqCoord
   * @return
   */
  double getInterpolatedValue(Square square, const float* sqCoord) const;

  /**
   * @brief getSquareCoord
   * @param xyz The input XYZ coordinate on the unit sphere.
   * @param sqCoord [output] The XY coordinate in the Modified Lambert Square
   * @return If the point was in the north or south squares
   */
  bool getSquareCoord(const float* xyz, float* sqCoord) const;

  /**
   * @brief getSquareIndex
   * @param sqCoord
   * @return
   */
  int getSquareIndex(float* sqCoord);

  /**
   * @brief This function normalizes the squares by taking the value of each square and dividing by the sum of all the
   * values in all the squares.
   */
  void normalizeSquares();

  /**
   * @brief normalizeSquaresToMRD This function first normalizes the squares then multiplies each value by the total number of squares.
   */
  void normalizeSquaresToMRD();

  /**
   * @brief createStereographicProjection
   * @param stereoGraphicProjectionDims
   */
  EbsdLib::DoubleArrayType::Pointer createStereographicProjection(int dim);

  void createStereographicProjection(int dim, EbsdLib::DoubleArrayType& stereoIntensity);

  /**
   * @brief Creates a circular Projection
   * @param dim
   * @return stereoIntensity
   */
  std::vector<float> createCircularProjection(int dim);

protected:
  ModifiedLambertProjection();

private:
  int m_Dimension;
  float m_StepSize; // The length of an individual grid square
  float m_SphereRadius;

  // These are some precalculated values based on the values above.
  float m_HalfDimension;
  float m_HalfDimensionTimesStepSize;
  float m_MaxCoord;
  float m_MinCoord;

  EbsdLib::DoubleArrayType::Pointer m_NorthSquare;
  EbsdLib::DoubleArrayType::Pointer m_SouthSquare;

public:
  ModifiedLambertProjection(const ModifiedLambertProjection&) = delete;            // Copy Constructor Not Implemented
  ModifiedLambertProjection(ModifiedLambertProjection&&) = delete;                 // Move Constructor Not Implemented
  ModifiedLambertProjection& operator=(const ModifiedLambertProjection&) = delete; // Copy Assignment Not Implemented
  ModifiedLambertProjection& operator=(ModifiedLambertProjection&&) = delete;      // Move Assignment Not Implemented
};
