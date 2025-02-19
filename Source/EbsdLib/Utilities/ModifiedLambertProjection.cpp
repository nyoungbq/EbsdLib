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

#include "ModifiedLambertProjection.h"

#include <array>

#include "EbsdLib/Core/EbsdMacros.h"
#include "EbsdLib/Math/EbsdLibMath.h"

#define WRITE_LAMBERT_SQUARE_COORD_VTK 0

namespace
{
double calcInterpolatedValue(const ModifiedLambertProjection& self, const std::array<float, 3>& xyz)
{
  std::array<float, 2> sqCoord{};
  if(self.getSquareCoord(xyz.data(), sqCoord.data()))
  {
    // get Value from North square
    return self.getInterpolatedValue(ModifiedLambertProjection::Square::NorthSquare, sqCoord.data());
  }
  else
  {
    // get Value from South square
    return self.getInterpolatedValue(ModifiedLambertProjection::Square::SouthSquare, sqCoord.data());
  }
};
} // namespace

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ModifiedLambertProjection::ModifiedLambertProjection()
: m_Dimension(0)
, m_StepSize(0.0f)
, m_SphereRadius(1.0f)
, m_MaxCoord(0.0)
, m_MinCoord(0.0)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ModifiedLambertProjection::~ModifiedLambertProjection() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ModifiedLambertProjection::Pointer ModifiedLambertProjection::LambertBallToSquare(EbsdLib::FloatArrayType* coords, int dimension, float sphereRadius)
{

  size_t npoints = coords->getNumberOfTuples();
  bool nhCheck = false;
  float sqCoord[2];
  // int sqIndex = 0;
  ModifiedLambertProjection::Pointer squareProj = ModifiedLambertProjection::New();
  squareProj->initializeSquares(dimension, sphereRadius);

#if WRITE_LAMBERT_SQUARE_COORD_VTK
  std::string ss;
  std::string filename("/tmp/");
  filename.append("ModifiedLambert_Square_Coords_").append(coords->getName()).append(".vtk");
  FILE* f = nullptr;
  f = fopen(filename.c_str(), "wb");
  if(nullptr == f)
  {
    ss.str("");
    std::string ss = QObject::tr("Could not open vtk viz file %1 for writing. Please check access permissions and the path to the output location exists").arg(filename);
    return squareProj;
  }

  // Write the correct header
  fprintf(f, "# vtk DataFile Version 2.0\n");
  fprintf(f, "data set from DREAM3D\n");
  fprintf(f, "ASCII");
  fprintf(f, "\n");

  fprintf(f, "DATASET UNSTRUCTURED_GRID\nPOINTS %lu float\n", coords->getNumberOfTuples());
#endif

  for(size_t i = 0; i < npoints; ++i)
  {
    sqCoord[0] = 0.0;
    sqCoord[1] = 0.0;
    // get coordinates in square projection of crystal normal parallel to boundary normal
    nhCheck = squareProj->getSquareCoord(coords->getPointer(i * 3), sqCoord);
#if WRITE_LAMBERT_SQUARE_COORD_VTK
    fprintf(f, "%f %f 0\n", sqCoord[0], sqCoord[1]);
#endif

    // Based on the XY coordinate, get the pointer index that the value corresponds to in the proper square
    //    sqIndex = squareProj->getSquareIndex(sqCoord);
    if(nhCheck)
    {
      // north increment by 1
      //      squareProj->addValue(ModifiedLambertProjection::NorthSquare, sqIndex, 1.0);
      squareProj->addInterpolatedValues(ModifiedLambertProjection::NorthSquare, sqCoord, 1.0);
    }
    else
    {
      // south increment by 1
      //      squareProj->addValue(ModifiedLambertProjection::SouthSquare, sqIndex, 1.0);
      squareProj->addInterpolatedValues(ModifiedLambertProjection::SouthSquare, sqCoord, 1.0);
    }
  }
#if WRITE_LAMBERT_SQUARE_COORD_VTK
  fclose(f);
#endif

  return squareProj;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::initializeSquares(int dims, float sphereRadius)
{
  m_Dimension = dims;
  m_SphereRadius = sphereRadius;
  // We want half the sphere area for each square because each square represents a hemisphere.
  float halfSphereArea = 4.0f * EbsdLib::Constants::k_PiF * sphereRadius * sphereRadius / 2.0f;
  // The length of a side of the square is the square root of the area
  float squareEdge = sqrt(halfSphereArea);

  m_StepSize = squareEdge / static_cast<float>(m_Dimension);

  m_MaxCoord = squareEdge / 2.0f;
  m_MinCoord = -squareEdge / 2.0f;
  m_HalfDimension = static_cast<float>(m_Dimension) / 2.0f;
  m_HalfDimensionTimesStepSize = m_HalfDimension * m_StepSize;

  std::vector<size_t> tDims(2, m_Dimension);
  std::vector<size_t> cDims(1, 1);
  m_NorthSquare = EbsdLib::DoubleArrayType::CreateArray(tDims, cDims, "ModifiedLambert_NorthSquare", true);
  m_NorthSquare->initializeWithZeros();
  m_SouthSquare = EbsdLib::DoubleArrayType::CreateArray(tDims, cDims, "ModifiedLambert_SouthSquare", true);
  m_SouthSquare->initializeWithZeros();
}
#ifdef DATA_ARRAY_ENABLE_HDF5_IO
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ModifiedLambertProjection::writeHDF5Data(hid_t groupId)
{
  int err = -1;

  std::vector<size_t> dims = {static_cast<size_t>(m_Dimension), static_cast<size_t>(m_Dimension), 1};
  err = m_NorthSquare->writeH5Data(groupId, dims);
  std::cout << "Err: " << err << std::endl;
  err = m_SouthSquare->writeH5Data(groupId, dims);
  std::cout << "Err: " << err << std::endl;

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ModifiedLambertProjection::readHDF5Data(hid_t groupId)
{
  int err = -1;
  return err;
}

#endif
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::addInterpolatedValues(Square square, float* sqCoord, double value)
{
  int abin1 = 0, bbin1 = 0;
  int abin2 = 0, bbin2 = 0;
  int abin3 = 0, bbin3 = 0;
  int abin4 = 0, bbin4 = 0;
  int abinSign, bbinSign;
  float modX = (sqCoord[0] + m_HalfDimensionTimesStepSize) / m_StepSize;
  float modY = (sqCoord[1] + m_HalfDimensionTimesStepSize) / m_StepSize;
  int abin = (int)modX;
  int bbin = (int)modY;
  modX -= abin;
  modY -= bbin;
  modX -= 0.5;
  modY -= 0.5;
  if(modX == 0.0)
  {
    abinSign = 1;
  }
  else
  {
    abinSign = static_cast<int>(modX / std::fabs(modX));
  }
  if(modY == 0.0)
  {
    bbinSign = 1;
  }
  else
  {
    bbinSign = static_cast<int>(modY / std::fabs(modY));
  }
  abin1 = abin;
  bbin1 = bbin;
  abin2 = abin + abinSign;
  bbin2 = bbin;
  if(abin2 < 0 || abin2 > m_Dimension - 1)
  {
    abin2 = abin2 - (abinSign * m_Dimension), bbin2 = m_Dimension - bbin2 - 1;
  }
  abin3 = abin;
  bbin3 = bbin + bbinSign;
  if(bbin3 < 0 || bbin3 > m_Dimension - 1)
  {
    abin3 = m_Dimension - abin3 - 1, bbin3 = bbin3 - (bbinSign * m_Dimension);
  }
  abin4 = abin + abinSign;
  bbin4 = bbin + bbinSign;
  if((abin4 < 0 || abin4 > m_Dimension - 1) && (bbin4 >= 0 && bbin4 <= m_Dimension - 1))
  {
    abin4 = abin4 - (abinSign * m_Dimension), bbin4 = m_Dimension - bbin4 - 1;
  }
  else if((abin4 >= 0 && abin4 <= m_Dimension - 1) && (bbin4 < 0 || bbin4 > m_Dimension - 1))
  {
    abin4 = m_Dimension - abin4 - 1, bbin4 = bbin4 - (bbinSign * m_Dimension);
  }
  else if((abin4 < 0 || abin4 > m_Dimension - 1) && (bbin4 < 0 || bbin4 > m_Dimension - 1))
  {
    abin4 = abin4 - (abinSign * m_Dimension), bbin4 = bbin4 - (bbinSign * m_Dimension);
  }
  modX = fabs(modX);
  modY = fabs(modY);

  int index1 = bbin1 * m_Dimension + abin1;
  int index2 = bbin2 * m_Dimension + abin2;
  int index3 = bbin3 * m_Dimension + abin3;
  int index4 = bbin4 * m_Dimension + abin4;
  if(square == NorthSquare)
  {
    double v1 = m_NorthSquare->getValue(index1) + value * (1.0 - modX) * (1.0 - modY);
    double v2 = m_NorthSquare->getValue(index2) + value * (modX) * (1.0 - modY);
    double v3 = m_NorthSquare->getValue(index3) + value * (1.0 - modX) * (modY);
    double v4 = m_NorthSquare->getValue(index4) + value * (modX) * (modY);
    m_NorthSquare->setValue(index1, v1);
    m_NorthSquare->setValue(index2, v2);
    m_NorthSquare->setValue(index3, v3);
    m_NorthSquare->setValue(index4, v4);
  }
  else
  {
    double v1 = m_SouthSquare->getValue(index1) + value * (1.0 - modX) * (1.0 - modY);
    double v2 = m_SouthSquare->getValue(index2) + value * (modX) * (1.0 - modY);
    double v3 = m_SouthSquare->getValue(index3) + value * (1.0 - modX) * (modY);
    double v4 = m_SouthSquare->getValue(index4) + value * (modX) * (modY);
    m_SouthSquare->setValue(index1, v1);
    m_SouthSquare->setValue(index2, v2);
    m_SouthSquare->setValue(index3, v3);
    m_SouthSquare->setValue(index4, v4);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::addValue(Square square, int index, double value)
{
  if(square == NorthSquare)
  {
    double v = m_NorthSquare->getValue(index) + value;
    m_NorthSquare->setValue(index, v);
  }
  else
  {
    double v = m_SouthSquare->getValue(index) + value;
    m_SouthSquare->setValue(index, v);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::setValue(Square square, int index, double value)
{
  if(square == NorthSquare)
  {
    m_NorthSquare->setValue(index, value);
  }
  else
  {
    m_SouthSquare->setValue(index, value);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double ModifiedLambertProjection::getValue(Square square, int index)
{
  if(square == NorthSquare)
  {
    return m_NorthSquare->getValue(index);
  }

  return m_SouthSquare->getValue(index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double ModifiedLambertProjection::getInterpolatedValue(Square square, const float* sqCoord) const
{
  // float sqCoord[2] = { sqCoord0[0] - 0.5*m_StepSize, sqCoord0[1] - 0.5*m_StepSize};
  int abin1, bbin1;
  int abin2, bbin2;
  int abin3, bbin3;
  int abin4, bbin4;
  int abinSign, bbinSign;
  float modX = (sqCoord[0] + m_HalfDimensionTimesStepSize) / m_StepSize;
  float modY = (sqCoord[1] + m_HalfDimensionTimesStepSize) / m_StepSize;
  int abin = (int)modX;
  int bbin = (int)modY;
  modX -= abin;
  modY -= bbin;
  modX -= 0.5;
  modY -= 0.5;
  if(modX == 0.0)
  {
    abinSign = 1;
  }
  else
  {
    abinSign = static_cast<int>(modX / std::fabs(modX));
  }
  if(modY == 0.0)
  {
    bbinSign = 1;
  }
  else
  {
    bbinSign = static_cast<int>(modY / std::fabs(modY));
  }
  abin1 = abin;
  bbin1 = bbin;
  abin2 = abin + abinSign;
  bbin2 = bbin;
  if(abin2 < 0 || abin2 > m_Dimension - 1)
  {
    abin2 = abin2 - (abinSign * m_Dimension), bbin2 = m_Dimension - bbin2 - 1;
  }
  abin3 = abin;
  bbin3 = bbin + bbinSign;
  if(bbin3 < 0 || bbin3 > m_Dimension - 1)
  {
    abin3 = m_Dimension - abin3 - 1, bbin3 = bbin3 - (bbinSign * m_Dimension);
  }
  abin4 = abin + abinSign;
  bbin4 = bbin + bbinSign;
  if((abin4 < 0 || abin4 > m_Dimension - 1) && (bbin4 >= 0 && bbin4 <= m_Dimension - 1))
  {
    abin4 = abin4 - (abinSign * m_Dimension), bbin4 = m_Dimension - bbin4 - 1;
  }
  else if((abin4 >= 0 && abin4 <= m_Dimension - 1) && (bbin4 < 0 || bbin4 > m_Dimension - 1))
  {
    abin4 = m_Dimension - abin4 - 1, bbin4 = bbin4 - (bbinSign * m_Dimension);
  }
  else if((abin4 < 0 || abin4 > m_Dimension - 1) && (bbin4 < 0 || bbin4 > m_Dimension - 1))
  {
    abin4 = abin4 - (abinSign * m_Dimension), bbin4 = bbin4 - (bbinSign * m_Dimension);
  }
  modX = fabs(modX);
  modY = fabs(modY);
  if(square == NorthSquare)
  {
    float intensity1 = static_cast<float>(m_NorthSquare->getValue((abin1) + (bbin1 * m_Dimension)));
    float intensity2 = static_cast<float>(m_NorthSquare->getValue((abin2) + (bbin2 * m_Dimension)));
    float intensity3 = static_cast<float>(m_NorthSquare->getValue((abin3) + (bbin3 * m_Dimension)));
    float intensity4 = static_cast<float>(m_NorthSquare->getValue((abin4) + (bbin4 * m_Dimension)));
    float interpolatedIntensity = ((intensity1 * (1 - modX) * (1 - modY)) + (intensity2 * (modX) * (1 - modY)) + (intensity3 * (1 - modX) * (modY)) + (intensity4 * (modX) * (modY)));
    return interpolatedIntensity;
  }

  float intensity1 = static_cast<float>(m_SouthSquare->getValue((abin1) + (bbin1 * m_Dimension)));
  float intensity2 = static_cast<float>(m_SouthSquare->getValue((abin2) + (bbin2 * m_Dimension)));
  float intensity3 = static_cast<float>(m_SouthSquare->getValue((abin3) + (bbin3 * m_Dimension)));
  float intensity4 = static_cast<float>(m_SouthSquare->getValue((abin4) + (bbin4 * m_Dimension)));
  float interpolatedIntensity = ((intensity1 * (1 - modX) * (1 - modY)) + (intensity2 * (modX) * (1 - modY)) + (intensity3 * (1 - modX) * (modY)) + (intensity4 * (modX) * (modY)));
  return interpolatedIntensity;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ModifiedLambertProjection::getSquareCoord(const float* xyz, float* sqCoord) const
{
  bool nhCheck = false;
  float adjust = 1.0;
  if(xyz[2] >= 0.0)
  {
    adjust = -1.0;
    nhCheck = true;
  }
  if(xyz[0] == 0 && xyz[1] == 0)
  {
    sqCoord[0] = 0.0;
    sqCoord[1] = 0.0;
    return nhCheck;
  }
  if(std::fabs(xyz[0]) >= std::fabs(xyz[1]))
  {
    sqCoord[0] = static_cast<float>((xyz[0] / std::fabs(xyz[0])) * std::sqrt(2.0 * m_SphereRadius * (m_SphereRadius + (xyz[2] * adjust))) * EbsdLib::Constants::k_HalfOfSqrtPiD);
    sqCoord[1] =
        static_cast<float>((xyz[0] / std::fabs(xyz[0])) * std::sqrt(2.0 * m_SphereRadius * (m_SphereRadius + (xyz[2] * adjust))) * ((EbsdLib::Constants::k_2OverSqrtPiD)*std::atan(xyz[1] / xyz[0])));
  }
  else
  {
    sqCoord[0] =
        static_cast<float>((xyz[1] / std::fabs(xyz[1])) * std::sqrt(2.0 * m_SphereRadius * (m_SphereRadius + (xyz[2] * adjust))) * ((EbsdLib::Constants::k_2OverSqrtPiD)*std::atan(xyz[0] / xyz[1])));
    sqCoord[1] = static_cast<float>((xyz[1] / std::fabs(xyz[1])) * std::sqrt(2.0 * m_SphereRadius * (m_SphereRadius + (xyz[2] * adjust))) * (EbsdLib::Constants::k_HalfOfSqrtPiD));
  }

  if(sqCoord[0] >= m_MaxCoord)
  {
    sqCoord[0] = m_MaxCoord - 0.0001f;
  }
  if(sqCoord[1] >= m_MaxCoord)
  {
    sqCoord[1] = m_MaxCoord - 0.0001f;
  }
  return nhCheck;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ModifiedLambertProjection::getSquareIndex(float* sqCoord)
{
  int x = (int)((sqCoord[0] + m_MaxCoord) / m_StepSize);
  if(x >= m_Dimension)
  {
    x = m_Dimension - 1;
  }
  if(x < 0)
  {
    x = 0;
  }
  int y = (int)((sqCoord[1] + m_MaxCoord) / m_StepSize);
  if(y >= m_Dimension)
  {
    y = m_Dimension - 1;
  }
  if(y < 0)
  {
    y = 0;
  }
  int index = y * m_Dimension + x;
  EBSD_INDEX_OUT_OF_RANGE(index < m_Dimension * m_Dimension);
  return index;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::normalizeSquares()
{

  size_t npoints = m_NorthSquare->getNumberOfTuples();
  double nTotal = 0;
  double sTotal = 0;

  double* north = m_NorthSquare->getPointer(0);
  double* south = m_SouthSquare->getPointer(0);

  // Get the Sum of all the bins
  for(size_t i = 0; i < npoints; ++i)
  {
    nTotal = nTotal + north[i];
    sTotal = sTotal + south[i];
  }
  double oneOverNTotal = 1.0 / nTotal;
  double oneOverSTotal = 1.0 / sTotal;

  // Divide each bin by the total of all the bins for that Hemisphere
  for(size_t i = 0; i < npoints; ++i)
  {
    north[i] = (north[i] * oneOverNTotal);
    south[i] = (south[i] * oneOverSTotal);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::normalizeSquaresToMRD()
{
  // First Normalize the squares
  normalizeSquares();
  size_t npoints = m_NorthSquare->getNumberOfTuples();
  double* north = m_NorthSquare->getPointer(0);
  double* south = m_SouthSquare->getPointer(0);
  int dimSqrd = m_Dimension * m_Dimension;

  // Multiply Each Bin by the total number of bins
  for(size_t i = 0; i < npoints; ++i)
  {
    north[i] = north[i] * dimSqrd;
    south[i] = south[i] * dimSqrd;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ModifiedLambertProjection::createStereographicProjection(int dim, EbsdLib::DoubleArrayType& stereoIntensity)
{
  int xpoints = dim;
  int ypoints = dim;

  int xpointshalf = xpoints / 2;
  int ypointshalf = ypoints / 2;

  float xres = 2.0f / static_cast<float>(xpoints);
  float yres = 2.0f / static_cast<float>(ypoints);

  stereoIntensity.initializeWithZeros();

  for(int64_t y = 0; y < ypoints; y++)
  {
    for(int64_t x = 0; x < xpoints; x++)
    {
      // get (x,y) for stereographic projection pixel
      float xtmp = static_cast<float>(x - xpointshalf) * xres + (xres * 0.5f);
      float ytmp = static_cast<float>(y - ypointshalf) * yres + (yres * 0.5f);
      int index = static_cast<int>(y * xpoints + x);
      if((xtmp * xtmp + ytmp * ytmp) <= 1.0)
      {
        std::array<float, 3> xyz{};
        // project xy from stereo projection to the unit spehere
        xyz[2] = -((xtmp * xtmp + ytmp * ytmp) - 1) / ((xtmp * xtmp + ytmp * ytmp) + 1);
        xyz[0] = xtmp * (1 + xyz[2]);
        xyz[1] = ytmp * (1 + xyz[2]);

        stereoIntensity[index] += calcInterpolatedValue(*this, xyz);

        for(auto& value : xyz)
        {
          value *= -1.0f;
        }

        stereoIntensity[index] += calcInterpolatedValue(*this, xyz);

        stereoIntensity[index] *= 0.5;
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EbsdLib::DoubleArrayType::Pointer ModifiedLambertProjection::createStereographicProjection(int dim)
{
  std::vector<size_t> tDims(2, dim);
  std::vector<size_t> cDims(1, 1);
  EbsdLib::DoubleArrayType::Pointer stereoIntensity = EbsdLib::DoubleArrayType::CreateArray(tDims, cDims, "ModifiedLambertProjection_StereographicProjection", true);
  createStereographicProjection(dim, *stereoIntensity);
  return stereoIntensity;
}

// -----------------------------------------------------------------------------
std::vector<float> ModifiedLambertProjection::createCircularProjection(int dim)
{
  std::vector<float> stereoIntensity(dim * dim, 0.0f);
  int xpoints = dim;
  int ypoints = dim;

  int xpointshalf = xpoints / 2;
  int ypointshalf = ypoints / 2;

  float unitRadius = std::sqrt(2.0f);
  float span = unitRadius - (-unitRadius);

  float xres = span / static_cast<float>(xpoints);
  float yres = span / static_cast<float>(ypoints);

  for(int64_t y = 0; y < ypoints; y++)
  {
    for(int64_t x = 0; x < xpoints; x++)
    {
      // get (x,y) for stereographic projection pixel
      float xtmp = static_cast<float>(x - xpointshalf) * xres + (xres * 0.5f);
      float ytmp = static_cast<float>(y - ypointshalf) * yres + (yres * 0.5f);
      size_t index = static_cast<size_t>(y * xpoints + x);
      if((xtmp * xtmp + ytmp * ytmp) <= unitRadius * unitRadius)
      {
        // project xy from stereo projection to the unit sphere
        float q = xtmp * xtmp + ytmp * ytmp;
        float t = std::sqrt(1.0f - (q / 4.0f));

        std::array<float, 3> xyz{xtmp * t, ytmp * t, (q / 2.0f) - 1.0f};

        stereoIntensity[index] += static_cast<float>(calcInterpolatedValue(*this, xyz));

        for(auto& value : xyz)
        {
          value *= -1.0f;
        }

        stereoIntensity[index] += static_cast<float>(calcInterpolatedValue(*this, xyz));

        stereoIntensity[index] *= 0.5f;
      }
    }
  }
  return stereoIntensity;
}

// -----------------------------------------------------------------------------
ModifiedLambertProjection::Pointer ModifiedLambertProjection::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
ModifiedLambertProjection::Pointer ModifiedLambertProjection::New()
{
  Pointer sharedPtr(new(ModifiedLambertProjection));
  return sharedPtr;
}

// -----------------------------------------------------------------------------
std::string ModifiedLambertProjection::getNameOfClass() const
{
  return std::string("ModifiedLambertProjection");
}

// -----------------------------------------------------------------------------
std::string ModifiedLambertProjection::ClassName()
{
  return std::string("ModifiedLambertProjection");
}

// -----------------------------------------------------------------------------
int ModifiedLambertProjection::getDimension() const
{
  return m_Dimension;
}

// -----------------------------------------------------------------------------
float ModifiedLambertProjection::getStepSize() const
{
  return m_StepSize;
}

// -----------------------------------------------------------------------------
float ModifiedLambertProjection::getSphereRadius() const
{
  return m_SphereRadius;
}

// -----------------------------------------------------------------------------
EbsdLib::DoubleArrayType::Pointer ModifiedLambertProjection::getNorthSquare() const
{
  return m_NorthSquare;
}

// -----------------------------------------------------------------------------
EbsdLib::DoubleArrayType::Pointer ModifiedLambertProjection::getSouthSquare() const
{
  return m_SouthSquare;
}
