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

#include "TrigonalOps.h"

#ifdef EbsdLib_USE_PARALLEL_ALGORITHMS
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>
#include <tbb/task.h>
#include <tbb/task_group.h>
#endif

// Include this FIRST because there is a needed define for some compiles
// to expose some of the constants needed below
#include "EbsdLib/Core/EbsdMacros.h"
#include "EbsdLib/Core/Orientation.hpp"
#include "EbsdLib/Math/EbsdLibMath.h"
#include "EbsdLib/Utilities/ColorTable.h"
#include "EbsdLib/Utilities/ComputeStereographicProjection.h"
#include "EbsdLib/Utilities/PoleFigureUtilities.h"

namespace TrigonalHigh
{
static const std::array<size_t, 3> OdfNumBins = {36, 36, 24}; // Represents a 5Deg bin

static const std::array<double, 3> OdfDimInitValue = {std::pow((0.75 * (EbsdLib::Constants::k_PiOver2D - std::sin(EbsdLib::Constants::k_PiOver2D))), (1.0 / 3.0)),
                                                      std::pow((0.75 * (EbsdLib::Constants::k_PiOver2D - std::sin(EbsdLib::Constants::k_PiOver2D))), (1.0 / 3.0)),
                                                      std::pow((0.75 * (EbsdLib::Constants::k_PiOver3D - std::sin(EbsdLib::Constants::k_PiOver3D))), (1.0 / 3.0))};
static const std::array<double, 3> OdfDimStepValue = {OdfDimInitValue[0] / static_cast<double>(OdfNumBins[0] / 2), OdfDimInitValue[1] / static_cast<double>(OdfNumBins[1] / 2),
                                                      OdfDimInitValue[2] / static_cast<double>(OdfNumBins[2] / 2)};

static const int symSize0 = 2;
static const int symSize1 = 2;
static const int symSize2 = 2;

static const int k_OdfSize = 31104;
static const int k_MdfSize = 31104;
static const int k_SymOpsCount = 6;
static const int k_NumMdfBins = 12;

static const std::vector<QuatD> QuatSym = {QuatD(0.000000000, 0.000000000, 0.000000000, 1.000000000), QuatD(0.000000000, 0.000000000, 0.866025400, 0.500000000),
                                           QuatD(0.000000000, 0.000000000, 0.866025400, -0.50000000), QuatD(1.000000000, 0.000000000, 0.000000000, 0.000000000),
                                           QuatD(-0.500000000, 0.86602540, 0.000000000, 0.000000000), QuatD(-0.500000000, -0.866025400, 0.000000000, 0.000000000)};

static const std::vector<OrientationD> RodSym = {
    {0.0, 0.0, 0.0}, {0.0, 0.0, 1.73205}, {0.0, 0.0, -1.73205}, {8660254000000.0, 5000000000000.0, 0.0}, {0.0, 1000000000000.0, 0.0}, {-8660254000000.0, 5000000000000.0, 0.0}};

static const double MatSym[k_SymOpsCount][3][3] = {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},

                                                   {{-0.5, EbsdLib::Constants::k_Root3Over2D, 0.0}, {-EbsdLib::Constants::k_Root3Over2D, -0.5, 0.0}, {0.0, 0.0, 1.0}},

                                                   {{-0.5, -EbsdLib::Constants::k_Root3Over2D, 0.0}, {EbsdLib::Constants::k_Root3Over2D, -0.5, 0.0}, {0.0, 0.0, 1.0}},

                                                   {{0.5, EbsdLib::Constants::k_Root3Over2D, 0.0}, {EbsdLib::Constants::k_Root3Over2D, -0.5, 0.0}, {0.0, 0.0, -1.0}},

                                                   {{-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, -1.0}},

                                                   {{0.5, -EbsdLib::Constants::k_Root3Over2D, 0.0}, {-EbsdLib::Constants::k_Root3Over2D, -0.5, 0.0}, {0.0, 0.0, -1.0}}};

} // namespace TrigonalHigh

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
TrigonalOps::TrigonalOps() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
TrigonalOps::~TrigonalOps() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool TrigonalOps::getHasInversion() const
{
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TrigonalOps::getODFSize() const
{
  return TrigonalHigh::k_OdfSize;
}

// -----------------------------------------------------------------------------
std::array<int32_t, 3> TrigonalOps::getNumSymmetry() const
{
  return {TrigonalHigh::symSize0, TrigonalHigh::symSize1, TrigonalHigh::symSize2};
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TrigonalOps::getMDFSize() const
{
  return TrigonalHigh::k_MdfSize;
}

// -----------------------------------------------------------------------------
int TrigonalOps::getMdfPlotBins() const
{
  return TrigonalHigh::k_NumMdfBins;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TrigonalOps::getNumSymOps() const
{
  return TrigonalHigh::k_SymOpsCount;
}

// -----------------------------------------------------------------------------
std::array<size_t, 3> TrigonalOps::getOdfNumBins() const
{
  return TrigonalHigh::OdfNumBins;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::string TrigonalOps::getSymmetryName() const
{
  return "Trigonal -3m";
  ;
}

OrientationD TrigonalOps::calculateMisorientation(const QuatD& q1, const QuatD& q2) const
{
  return calculateMisorientationInternal(TrigonalHigh::QuatSym, q1, q2);
}

// -----------------------------------------------------------------------------
OrientationF TrigonalOps::calculateMisorientation(const QuatF& q1f, const QuatF& q2f) const

{
  QuatD q1 = q1f.to<double>();
  QuatD q2 = q2f.to<double>();
  OrientationD axisAngle = calculateMisorientationInternal(TrigonalHigh::QuatSym, q1, q2);
  return axisAngle;
}

// -----------------------------------------------------------------------------
QuatD TrigonalOps::getQuatSymOp(int32_t i) const
{
  return TrigonalHigh::QuatSym[i];
}

void TrigonalOps::getRodSymOp(int i, double* r) const
{
  r[0] = TrigonalHigh::RodSym[i][0];
  r[1] = TrigonalHigh::RodSym[i][1];
  r[2] = TrigonalHigh::RodSym[i][2];
}

EbsdLib::Matrix3X3D TrigonalOps::getMatSymOpD(int i) const
{
  return {TrigonalHigh::MatSym[i][0][0], TrigonalHigh::MatSym[i][0][1], TrigonalHigh::MatSym[i][0][2], TrigonalHigh::MatSym[i][1][0], TrigonalHigh::MatSym[i][1][1],
          TrigonalHigh::MatSym[i][1][2], TrigonalHigh::MatSym[i][2][0], TrigonalHigh::MatSym[i][2][1], TrigonalHigh::MatSym[i][2][2]};
}

EbsdLib::Matrix3X3F TrigonalOps::getMatSymOpF(int i) const
{
  return {static_cast<float>(TrigonalHigh::MatSym[i][0][0]), static_cast<float>(TrigonalHigh::MatSym[i][0][1]), static_cast<float>(TrigonalHigh::MatSym[i][0][2]),
          static_cast<float>(TrigonalHigh::MatSym[i][1][0]), static_cast<float>(TrigonalHigh::MatSym[i][1][1]), static_cast<float>(TrigonalHigh::MatSym[i][1][2]),
          static_cast<float>(TrigonalHigh::MatSym[i][2][0]), static_cast<float>(TrigonalHigh::MatSym[i][2][1]), static_cast<float>(TrigonalHigh::MatSym[i][2][2])};
}

void TrigonalOps::getMatSymOp(int i, double g[3][3]) const
{
  g[0][0] = TrigonalHigh::MatSym[i][0][0];
  g[0][1] = TrigonalHigh::MatSym[i][0][1];
  g[0][2] = TrigonalHigh::MatSym[i][0][2];
  g[1][0] = TrigonalHigh::MatSym[i][1][0];
  g[1][1] = TrigonalHigh::MatSym[i][1][1];
  g[1][2] = TrigonalHigh::MatSym[i][1][2];
  g[2][0] = TrigonalHigh::MatSym[i][2][0];
  g[2][1] = TrigonalHigh::MatSym[i][2][1];
  g[2][2] = TrigonalHigh::MatSym[i][2][2];
}

void TrigonalOps::getMatSymOp(int i, float g[3][3]) const
{
  g[0][0] = static_cast<float>(TrigonalHigh::MatSym[i][0][0]);
  g[0][1] = static_cast<float>(TrigonalHigh::MatSym[i][0][1]);
  g[0][2] = static_cast<float>(TrigonalHigh::MatSym[i][0][2]);
  g[1][0] = static_cast<float>(TrigonalHigh::MatSym[i][1][0]);
  g[1][1] = static_cast<float>(TrigonalHigh::MatSym[i][1][1]);
  g[1][2] = static_cast<float>(TrigonalHigh::MatSym[i][1][2]);
  g[2][0] = static_cast<float>(TrigonalHigh::MatSym[i][2][0]);
  g[2][1] = static_cast<float>(TrigonalHigh::MatSym[i][2][1]);
  g[2][2] = static_cast<float>(TrigonalHigh::MatSym[i][2][2]);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType TrigonalOps::getODFFZRod(const OrientationType& rod) const
{
  return _calcRodNearestOrigin(TrigonalHigh::RodSym, rod);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType TrigonalOps::getMDFFZRod(const OrientationType& inRod) const
{
  double w = 0.0, n1 = 0.0, n2 = 0.0, n3 = 0.0;
  double FZn1 = 0.0, FZn2 = 0.0, FZn3 = 0.0, FZw = 0.0;
  double n1n2mag = 0.0f;

  OrientationType rod = _calcRodNearestOrigin(TrigonalHigh::RodSym, inRod);

  OrientationType ax = OrientationTransformation::ro2ax<OrientationType, OrientationType>(rod);

  n1 = ax[0];
  n2 = ax[1], n3 = ax[2], w = ax[3];

  float denom = static_cast<float>(std::sqrt((n1 * n1 + n2 * n2 + n3 * n3)));
  n1 = n1 / denom;
  n2 = n2 / denom;
  n3 = n3 / denom;
  if(n3 < 0)
  {
    n1 = -n1, n2 = -n2, n3 = -n3;
  }
  float angle = static_cast<float>(180.0 * std::atan2(n2, n1) * EbsdLib::Constants::k_1OverPiD);
  if(angle < 0)
  {
    angle = angle + 360.0f;
  }
  FZn1 = n1;
  FZn2 = n2;
  FZn3 = n3;
  if(angle > 60.0f)
  {
    n1n2mag = std::sqrt(n1 * n1 + n2 * n2);
    if(int(angle / 60) % 2 == 0)
    {
      FZw = angle - (60.0f * int(angle / 60.0f));
      FZw = FZw * EbsdLib::Constants::k_PiOver180D;
      FZn1 = n1n2mag * std::cos(FZw);
      FZn2 = n1n2mag * std::sin(FZw);
    }
    else
    {
      FZw = angle - (60.0f * int(angle / 60.0f));
      FZw = 60.0f - FZw;
      FZw = FZw * EbsdLib::Constants::k_PiOver180D;
      FZn1 = n1n2mag * std::cos(FZw);
      FZn2 = n1n2mag * std::sin(FZw);
    }
  }

  return OrientationTransformation::ax2ro<OrientationType, OrientationType>(OrientationType(FZn1, FZn2, FZn3, FZw));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QuatD TrigonalOps::getNearestQuat(const QuatD& q1, const QuatD& q2) const
{
  return _calcNearestQuat(TrigonalHigh::QuatSym, q1, q2);
}
QuatF TrigonalOps::getNearestQuat(const QuatF& q1f, const QuatF& q2f) const
{
  return _calcNearestQuat(TrigonalHigh::QuatSym, q1f.to<double>(), q2f.to<double>()).to<float>();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int TrigonalOps::getMisoBin(const OrientationType& rod) const
{
  double dim[3];
  double bins[3];
  double step[3];

  OrientationType ho = OrientationTransformation::ro2ho<OrientationType, OrientationType>(rod);

  dim[0] = TrigonalHigh::OdfDimInitValue[0];
  dim[1] = TrigonalHigh::OdfDimInitValue[1];
  dim[2] = TrigonalHigh::OdfDimInitValue[2];
  step[0] = TrigonalHigh::OdfDimStepValue[0];
  step[1] = TrigonalHigh::OdfDimStepValue[1];
  step[2] = TrigonalHigh::OdfDimStepValue[2];
  bins[0] = static_cast<double>(TrigonalHigh::OdfNumBins[0]);
  bins[1] = static_cast<double>(TrigonalHigh::OdfNumBins[1]);
  bins[2] = static_cast<double>(TrigonalHigh::OdfNumBins[2]);

  return _calcMisoBin(dim, bins, step, ho);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType TrigonalOps::determineEulerAngles(double random[3], int choose) const
{
  double init[3];
  double step[3];
  int32_t phi[3];
  double h1, h2, h3;

  init[0] = TrigonalHigh::OdfDimInitValue[0];
  init[1] = TrigonalHigh::OdfDimInitValue[1];
  init[2] = TrigonalHigh::OdfDimInitValue[2];
  step[0] = TrigonalHigh::OdfDimStepValue[0];
  step[1] = TrigonalHigh::OdfDimStepValue[1];
  step[2] = TrigonalHigh::OdfDimStepValue[2];
  phi[0] = static_cast<int32_t>(choose % TrigonalHigh::OdfNumBins[0]);
  phi[1] = static_cast<int32_t>((choose / TrigonalHigh::OdfNumBins[0]) % TrigonalHigh::OdfNumBins[1]);
  phi[2] = static_cast<int32_t>(choose / (TrigonalHigh::OdfNumBins[0] * TrigonalHigh::OdfNumBins[1]));

  _calcDetermineHomochoricValues(random, init, step, phi, h1, h2, h3);

  OrientationType ho(h1, h2, h3);
  OrientationType ro = OrientationTransformation::ho2ro<OrientationType, OrientationType>(ho);
  ro = getODFFZRod(ro);
  OrientationType eu = OrientationTransformation::ro2eu<OrientationType, OrientationType>(ro);
  return eu;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType TrigonalOps::randomizeEulerAngles(const OrientationType& synea) const
{
  size_t symOp = getRandomSymmetryOperatorIndex(TrigonalHigh::k_SymOpsCount);
  QuatD quat = OrientationTransformation::eu2qu<OrientationType, QuatD>(synea);
  QuatD qc = TrigonalHigh::QuatSym[symOp] * quat;
  return OrientationTransformation::qu2eu<QuatD, OrientationType>(qc);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType TrigonalOps::determineRodriguesVector(double random[3], int choose) const
{
  double init[3];
  double step[3];
  int32_t phi[3];
  double h1, h2, h3;

  init[0] = TrigonalHigh::OdfDimInitValue[0];
  init[1] = TrigonalHigh::OdfDimInitValue[1];
  init[2] = TrigonalHigh::OdfDimInitValue[2];
  step[0] = TrigonalHigh::OdfDimStepValue[0];
  step[1] = TrigonalHigh::OdfDimStepValue[1];
  step[2] = TrigonalHigh::OdfDimStepValue[2];
  phi[0] = static_cast<int32_t>(choose % TrigonalHigh::OdfNumBins[0]);
  phi[1] = static_cast<int32_t>((choose / TrigonalHigh::OdfNumBins[0]) % TrigonalHigh::OdfNumBins[1]);
  phi[2] = static_cast<int32_t>(choose / (TrigonalHigh::OdfNumBins[0] * TrigonalHigh::OdfNumBins[1]));

  _calcDetermineHomochoricValues(random, init, step, phi, h1, h2, h3);
  OrientationType ho(h1, h2, h3);
  OrientationType ro = OrientationTransformation::ho2ro<OrientationType, OrientationType>(ho);
  ro = getMDFFZRod(ro);
  return ro;
}

int TrigonalOps::getOdfBin(const OrientationType& rod) const
{
  double dim[3];
  double bins[3];
  double step[3];

  OrientationType ho = OrientationTransformation::ro2ho<OrientationType, OrientationType>(rod);

  dim[0] = TrigonalHigh::OdfDimInitValue[0];
  dim[1] = TrigonalHigh::OdfDimInitValue[1];
  dim[2] = TrigonalHigh::OdfDimInitValue[2];
  step[0] = TrigonalHigh::OdfDimStepValue[0];
  step[1] = TrigonalHigh::OdfDimStepValue[1];
  step[2] = TrigonalHigh::OdfDimStepValue[2];
  bins[0] = static_cast<double>(TrigonalHigh::OdfNumBins[0]);
  bins[1] = static_cast<double>(TrigonalHigh::OdfNumBins[1]);
  bins[2] = static_cast<double>(TrigonalHigh::OdfNumBins[2]);

  return _calcODFBin(dim, bins, step, ho);
}

void TrigonalOps::getSchmidFactorAndSS(double load[3], double& schmidfactor, double angleComps[2], int& slipsys) const
{
  schmidfactor = 0;
  slipsys = 0;
}

void TrigonalOps::getSchmidFactorAndSS(double load[3], double plane[3], double direction[3], double& schmidfactor, double angleComps[2], int& slipsys) const
{
  schmidfactor = 0;
  slipsys = 0;
  angleComps[0] = 0;
  angleComps[1] = 0;

  // compute mags
  double loadMag = sqrt(load[0] * load[0] + load[1] * load[1] + load[2] * load[2]);
  double planeMag = sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
  double directionMag = sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
  planeMag *= loadMag;
  directionMag *= loadMag;

  // loop over symmetry operators finding highest schmid factor
  for(int i = 0; i < TrigonalHigh::k_SymOpsCount; i++)
  {
    // compute slip system
    double slipPlane[3] = {0};
    slipPlane[2] = TrigonalHigh::MatSym[i][2][0] * plane[0] + TrigonalHigh::MatSym[i][2][1] * plane[1] + TrigonalHigh::MatSym[i][2][2] * plane[2];

    // dont consider negative z planes (to avoid duplicates)
    if(slipPlane[2] >= 0)
    {
      slipPlane[0] = TrigonalHigh::MatSym[i][0][0] * plane[0] + TrigonalHigh::MatSym[i][0][1] * plane[1] + TrigonalHigh::MatSym[i][0][2] * plane[2];
      slipPlane[1] = TrigonalHigh::MatSym[i][1][0] * plane[0] + TrigonalHigh::MatSym[i][1][1] * plane[1] + TrigonalHigh::MatSym[i][1][2] * plane[2];

      double slipDirection[3] = {0};
      slipDirection[0] = TrigonalHigh::MatSym[i][0][0] * direction[0] + TrigonalHigh::MatSym[i][0][1] * direction[1] + TrigonalHigh::MatSym[i][0][2] * direction[2];
      slipDirection[1] = TrigonalHigh::MatSym[i][1][0] * direction[0] + TrigonalHigh::MatSym[i][1][1] * direction[1] + TrigonalHigh::MatSym[i][1][2] * direction[2];
      slipDirection[2] = TrigonalHigh::MatSym[i][2][0] * direction[0] + TrigonalHigh::MatSym[i][2][1] * direction[1] + TrigonalHigh::MatSym[i][2][2] * direction[2];

      double cosPhi = fabs(load[0] * slipPlane[0] + load[1] * slipPlane[1] + load[2] * slipPlane[2]) / planeMag;
      double cosLambda = fabs(load[0] * slipDirection[0] + load[1] * slipDirection[1] + load[2] * slipDirection[2]) / directionMag;

      double schmid = cosPhi * cosLambda;
      if(schmid > schmidfactor)
      {
        schmidfactor = schmid;
        slipsys = i;
        angleComps[0] = acos(cosPhi);
        angleComps[1] = acos(cosLambda);
      }
    }
  }
}

double TrigonalOps::getmPrime(const QuatD& q1, const QuatD& q2, double LD[3]) const
{
  return 0.0;
}

double TrigonalOps::getF1(const QuatD& q1, const QuatD& q2, double LD[3], bool maxS) const
{
  return 0.0;
}

double TrigonalOps::getF1spt(const QuatD& q1, const QuatD& q2, double LD[3], bool maxS) const
{
  return 0.0;
}

double TrigonalOps::getF7(const QuatD& q1, const QuatD& q2, double LD[3], bool maxS) const
{
  return 0.0;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

namespace TrigonalHigh
{
class GenerateSphereCoordsImpl
{
  EbsdLib::FloatArrayType* m_Eulers;
  EbsdLib::FloatArrayType* m_xyz001;
  EbsdLib::FloatArrayType* m_xyz011;
  EbsdLib::FloatArrayType* m_xyz111;

public:
  GenerateSphereCoordsImpl(EbsdLib::FloatArrayType* eulerAngles, EbsdLib::FloatArrayType* xyz001Coords, EbsdLib::FloatArrayType* xyz011Coords, EbsdLib::FloatArrayType* xyz111Coords)
  : m_Eulers(eulerAngles)
  , m_xyz001(xyz001Coords)
  , m_xyz011(xyz011Coords)
  , m_xyz111(xyz111Coords)
  {
  }
  virtual ~GenerateSphereCoordsImpl() = default;

  void generate(size_t start, size_t end) const
  {
    EbsdLib::Matrix3X3D gTranspose;
    EbsdLib::Matrix3X1D direction(0.0, 0.0, 0.0);

    // Geneate all the Coordinates
    for(size_t i = start; i < end; ++i)
    {
      OrientationType eu(m_Eulers->getValue(i * 3), m_Eulers->getValue(i * 3 + 1), m_Eulers->getValue(i * 3 + 2));
      EbsdLib::Matrix3X3D g(OrientationTransformation::eu2om<OrientationType, OrientationType>(eu).data());

      gTranspose = g.transpose();

      // -----------------------------------------------------------------------------
      // 001 Family
      direction[0] = 0.0;
      direction[1] = 0.0;
      direction[2] = 1.0;
      (gTranspose * direction).copyInto<float>(m_xyz001->getPointer(i * 6));
      std::transform(m_xyz001->getPointer(i * 6), m_xyz001->getPointer(i * 6 + 3),
                     m_xyz001->getPointer(i * 6 + 3),             // write to the next triplet in memory
                     [](float value) { return value *= -1.0F; }); // Multiply each value by -1.0

      // -----------------------------------------------------------------------------
      // 011 Family
      direction[0] = 0;
      direction[1] = -1.0;
      direction[2] = 0.0;
      (gTranspose * direction).copyInto<float>(m_xyz011->getPointer(i * 6));
      std::transform(m_xyz011->getPointer(i * 6), m_xyz011->getPointer(i * 6 + 3),
                     m_xyz011->getPointer(i * 6 + 3),             // write to the next triplet in memory
                     [](float value) { return value *= -1.0F; }); // Multiply each value by -1.0

      // -----------------------------------------------------------------------------
      // 111 Family
      direction[0] = EbsdLib::Constants::k_Root3Over2D;
      direction[1] = -0.5;
      direction[2] = 0;
      (gTranspose * direction).copyInto<float>(m_xyz111->getPointer(i * 6));
      std::transform(m_xyz111->getPointer(i * 6), m_xyz111->getPointer(i * 6 + 3),
                     m_xyz111->getPointer(i * 6 + 3),             // write to the next triplet in memory
                     [](float value) { return value *= -1.0F; }); // Multiply each value by -1.0
    }
  }

#ifdef EbsdLib_USE_PARALLEL_ALGORITHMS
  void operator()(const tbb::blocked_range<size_t>& r) const
  {
    generate(r.begin(), r.end());
  }
#endif
};
} // namespace TrigonalHigh

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TrigonalOps::generateSphereCoordsFromEulers(EbsdLib::FloatArrayType* eulers, EbsdLib::FloatArrayType* xyz001, EbsdLib::FloatArrayType* xyz011, EbsdLib::FloatArrayType* xyz111) const
{
  size_t nOrientations = eulers->getNumberOfTuples();

  // Sanity Check the size of the arrays
  if(xyz001->getNumberOfTuples() < nOrientations * TrigonalHigh::symSize0)
  {
    xyz001->resizeTuples(nOrientations * TrigonalHigh::symSize0 * 3);
  }
  if(xyz011->getNumberOfTuples() < nOrientations * TrigonalHigh::symSize1)
  {
    xyz011->resizeTuples(nOrientations * TrigonalHigh::symSize1 * 3);
  }
  if(xyz111->getNumberOfTuples() < nOrientations * TrigonalHigh::symSize2)
  {
    xyz111->resizeTuples(nOrientations * TrigonalHigh::symSize2 * 3);
  }

#ifdef EbsdLib_USE_PARALLEL_ALGORITHMS
  bool doParallel = true;
  if(doParallel)
  {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, nOrientations), TrigonalHigh::GenerateSphereCoordsImpl(eulers, xyz001, xyz011, xyz111), tbb::auto_partitioner());
  }
  else
#endif
  {
    TrigonalHigh::GenerateSphereCoordsImpl serial(eulers, xyz001, xyz011, xyz111);
    serial.generate(0, nOrientations);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool TrigonalOps::inUnitTriangle(double eta, double chi) const
{
  return !(eta < (-90.0 * EbsdLib::Constants::k_PiOver180D) || eta > (-30.0 * EbsdLib::Constants::k_PiOver180D) || chi < 0 || chi > (90.0 * EbsdLib::Constants::k_PiOver180D));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EbsdLib::Rgb TrigonalOps::generateIPFColor(double* eulers, double* refDir, bool convertDegrees) const
{
  return generateIPFColor(eulers[0], eulers[1], eulers[2], refDir[0], refDir[1], refDir[2], convertDegrees);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EbsdLib::Rgb TrigonalOps::generateIPFColor(double phi1, double phi, double phi2, double refDir0, double refDir1, double refDir2, bool degToRad) const
{
  if(degToRad)
  {
    phi1 = phi1 * EbsdLib::Constants::k_DegToRadD;
    phi = phi * EbsdLib::Constants::k_DegToRadD;
    phi2 = phi2 * EbsdLib::Constants::k_DegToRadD;
  }

  EbsdLib::Matrix3X1D refDirection = {refDir0, refDir1, refDir2};
  double chi = 0.0f, eta = 0.0f;
  double _rgb[3] = {0.0, 0.0, 0.0};

  OrientationType eu(phi1, phi, phi2);
  OrientationType om(9); // Reusable for the loop
  QuatD q1 = OrientationTransformation::eu2qu<OrientationType, QuatD>(eu);

  for(int j = 0; j < TrigonalHigh::k_SymOpsCount; j++)
  {
    QuatD qu = getQuatSymOp(j) * q1;
    EbsdLib::Matrix3X3D g(OrientationTransformation::qu2om<QuatD, OrientationType>(qu).data());
    EbsdLib::Matrix3X1D p = (g * refDirection).normalize();

    if(!getHasInversion() && p[2] < 0)
    {
      continue;
    }
    if(getHasInversion() && p[2] < 0)
    {
      p[0] = -p[0], p[1] = -p[1], p[2] = -p[2];
    }
    chi = std::acos(p[2]);
    eta = std::atan2(p[1], p[0]);
    if(!inUnitTriangle(eta, chi))
    {
      continue;
    }

    break;
  }

  double etaMin = -90.0;
  double etaMax = -30.0;
  double chiMax = 90.0;
  double etaDeg = eta * EbsdLib::Constants::k_180OverPiD;
  double chiDeg = chi * EbsdLib::Constants::k_180OverPiD;

  _rgb[0] = 1.0 - chiDeg / chiMax;
  _rgb[2] = fabs(etaDeg - etaMin) / (etaMax - etaMin);
  _rgb[1] = 1 - _rgb[2];
  _rgb[1] *= chiDeg / chiMax;
  _rgb[2] *= chiDeg / chiMax;
  _rgb[0] = sqrt(_rgb[0]);
  _rgb[1] = sqrt(_rgb[1]);
  _rgb[2] = sqrt(_rgb[2]);

  double max = _rgb[0];
  if(_rgb[1] > max)
  {
    max = _rgb[1];
  }
  if(_rgb[2] > max)
  {
    max = _rgb[2];
  }

  _rgb[0] = _rgb[0] / max;
  _rgb[1] = _rgb[1] / max;
  _rgb[2] = _rgb[2] / max;

  return EbsdLib::RgbColor::dRgb(static_cast<int32_t>(_rgb[0] * 255), static_cast<int32_t>(_rgb[1] * 255), static_cast<int32_t>(_rgb[2] * 255), 255);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EbsdLib::Rgb TrigonalOps::generateRodriguesColor(double r1, double r2, double r3) const
{
  double range1 = 2.0f * TrigonalHigh::OdfDimInitValue[0];
  double range2 = 2.0f * TrigonalHigh::OdfDimInitValue[1];
  double range3 = 2.0f * TrigonalHigh::OdfDimInitValue[2];
  double max1 = range1 / 2.0f;
  double max2 = range2 / 2.0f;
  double max3 = range3 / 2.0f;
  double red = (r1 + max1) / range1;
  double green = (r2 + max2) / range2;
  double blue = (r3 + max3) / range3;

  return EbsdLib::RgbColor::dRgb(static_cast<int32_t>(red * 255), static_cast<int32_t>(green * 255), static_cast<int32_t>(blue * 255), 255);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::array<std::string, 3> TrigonalOps::getDefaultPoleFigureNames() const
{
return {"<0001>", "<0-110>", "<1-100>"};
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<EbsdLib::UInt8ArrayType::Pointer> TrigonalOps::generatePoleFigure(PoleFigureConfiguration_t& config) const
{
  std::array<std::string, 3>labels = getDefaultPoleFigureNames();
  std::string label0 = labels[0];
  std::string label1 = labels[1];
  std::string label2 = labels[2];

  if(!config.labels.empty())
  {
    label0 = config.labels.at(0);
  }
  if(config.labels.size() > 1)
  {
    label1 = config.labels.at(1);
  }
  if(config.labels.size() > 2)
  {
    label2 = config.labels.at(2);
  }

  size_t numOrientations = config.eulers->getNumberOfTuples();

  // Create an Array to hold the XYZ Coordinates which are the coords on the sphere.
  // this is size for CUBIC ONLY, <001> Family
  std::vector<size_t> dims(1, 3);
  EbsdLib::FloatArrayType::Pointer xyz001 = EbsdLib::FloatArrayType::CreateArray(numOrientations * TrigonalHigh::symSize0, dims, label0 + std::string("xyzCoords"), true);
  // this is size for CUBIC ONLY, <011> Family
  EbsdLib::FloatArrayType::Pointer xyz011 = EbsdLib::FloatArrayType::CreateArray(numOrientations * TrigonalHigh::symSize1, dims, label1 + std::string("xyzCoords"), true);
  // this is size for CUBIC ONLY, <111> Family
  EbsdLib::FloatArrayType::Pointer xyz111 = EbsdLib::FloatArrayType::CreateArray(numOrientations * TrigonalHigh::symSize2, dims, label2 + std::string("xyzCoords"), true);

  config.sphereRadius = 1.0f;

  // Generate the coords on the sphere **** Parallelized
  generateSphereCoordsFromEulers(config.eulers, xyz001.get(), xyz011.get(), xyz111.get());

  // These arrays hold the "intensity" images which eventually get converted to an actual Color RGB image
  // Generate the modified Lambert projection images (Squares, 2 of them, 1 for northern hemisphere, 1 for southern hemisphere
  EbsdLib::DoubleArrayType::Pointer intensity001 = EbsdLib::DoubleArrayType::CreateArray(config.imageDim * config.imageDim, label0 + "_Intensity_Image", true);
  EbsdLib::DoubleArrayType::Pointer intensity011 = EbsdLib::DoubleArrayType::CreateArray(config.imageDim * config.imageDim, label1 + "_Intensity_Image", true);
  EbsdLib::DoubleArrayType::Pointer intensity111 = EbsdLib::DoubleArrayType::CreateArray(config.imageDim * config.imageDim, label2 + "_Intensity_Image", true);
#ifdef EbsdLib_USE_PARALLEL_ALGORITHMS
  bool doParallel = true;

  if(doParallel)
  {
    std::shared_ptr<tbb::task_group> g(new tbb::task_group);
    g->run(ComputeStereographicProjection(xyz001.get(), &config, intensity001.get()));
    g->run(ComputeStereographicProjection(xyz011.get(), &config, intensity011.get()));
    g->run(ComputeStereographicProjection(xyz111.get(), &config, intensity111.get()));
    g->wait(); // Wait for all the threads to complete before moving on.
  }
  else
#endif
  {
    ComputeStereographicProjection m001(xyz001.get(), &config, intensity001.get());
    m001();
    ComputeStereographicProjection m011(xyz011.get(), &config, intensity011.get());
    m011();
    ComputeStereographicProjection m111(xyz111.get(), &config, intensity111.get());
    m111();
  }

  // Find the Max and Min values based on ALL 3 arrays so we can color scale them all the same
  double max = std::numeric_limits<double>::min();
  double min = std::numeric_limits<double>::max();

  double* dPtr = intensity001->getPointer(0);
  size_t count = intensity001->getNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if(dPtr[i] > max)
    {
      max = dPtr[i];
    }
    if(dPtr[i] < min)
    {
      min = dPtr[i];
    }
  }

  dPtr = intensity011->getPointer(0);
  count = intensity011->getNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if(dPtr[i] > max)
    {
      max = dPtr[i];
    }
    if(dPtr[i] < min)
    {
      min = dPtr[i];
    }
  }

  dPtr = intensity111->getPointer(0);
  count = intensity111->getNumberOfTuples();
  for(size_t i = 0; i < count; ++i)
  {
    if(dPtr[i] > max)
    {
      max = dPtr[i];
    }
    if(dPtr[i] < min)
    {
      min = dPtr[i];
    }
  }

  config.minScale = min;
  config.maxScale = max;

  dims[0] = 4;
  EbsdLib::UInt8ArrayType::Pointer image001 = EbsdLib::UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, dims, label0, true);
  EbsdLib::UInt8ArrayType::Pointer image011 = EbsdLib::UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, dims, label1, true);
  EbsdLib::UInt8ArrayType::Pointer image111 = EbsdLib::UInt8ArrayType::CreateArray(config.imageDim * config.imageDim, dims, label2, true);

  std::vector<EbsdLib::UInt8ArrayType::Pointer> poleFigures(3);
  if(config.order.size() == 3)
  {
    poleFigures[config.order[0]] = image001;
    poleFigures[config.order[1]] = image011;
    poleFigures[config.order[2]] = image111;
  }
  else
  {
    poleFigures[0] = image001;
    poleFigures[1] = image011;
    poleFigures[2] = image111;
  }

#ifdef EbsdLib_USE_PARALLEL_ALGORITHMS

  if(doParallel)
  {
    std::shared_ptr<tbb::task_group> g(new tbb::task_group);
    g->run(GeneratePoleFigureRgbaImageImpl(intensity001.get(), &config, image001.get()));
    g->run(GeneratePoleFigureRgbaImageImpl(intensity011.get(), &config, image011.get()));
    g->run(GeneratePoleFigureRgbaImageImpl(intensity111.get(), &config, image111.get()));
    g->wait(); // Wait for all the threads to complete before moving on.
  }
  else
#endif
  {
    GeneratePoleFigureRgbaImageImpl m001(intensity001.get(), &config, image001.get());
    m001();
    GeneratePoleFigureRgbaImageImpl m011(intensity011.get(), &config, image011.get());
    m011();
    GeneratePoleFigureRgbaImageImpl m111(intensity111.get(), &config, image111.get());
    m111();
  }

  return poleFigures;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EbsdLib::UInt8ArrayType::Pointer TrigonalOps::generateIPFTriangleLegend(int imageDim) const
{

  std::vector<size_t> dims(1, 4);
  EbsdLib::UInt8ArrayType::Pointer image = EbsdLib::UInt8ArrayType::CreateArray(imageDim * imageDim, dims, getSymmetryName() + " Triangle Legend", true);
  uint32_t* pixelPtr = reinterpret_cast<uint32_t*>(image->getPointer(0));

  double xInc = 1.0f / static_cast<double>(imageDim);
  double yInc = 1.0f / static_cast<double>(imageDim);
  double rad = 1.0f;

  double x = 0.0f;
  double y = 0.0f;
  double a = 0.0f;
  double b = 0.0f;
  double c = 0.0f;

  double val = 0.0f;
  double x1 = 0.0f;
  double y1 = 0.0f;
  double z1 = 0.0f;
  double denom = 0.0f;

  // Find the slope of the bounding line.
  static const double m = std::sin(30.0 * EbsdLib::Constants::k_PiOver180D) / std::cos(30.0 * EbsdLib::Constants::k_PiOver180D);

  EbsdLib::Rgb color;
  size_t idx = 0;
  size_t yScanLineIndex = 0; // We use this to control where the data is drawn. Otherwise the image will come out flipped vertically
  // Loop over every pixel in the image and project up to the sphere to get the angle and then figure out the RGB from
  // there.
  for(int32_t yIndex = 0; yIndex < imageDim; ++yIndex)
  {

    for(int32_t xIndex = 0; xIndex < imageDim; ++xIndex)
    {
      idx = (imageDim * yScanLineIndex) + xIndex;

      x = xIndex * xInc;
      y = yIndex * yInc;

      double sumSquares = (x * x) + (y * y);
      if(sumSquares > 1.0f || x > y / m) // Outside unit circle
      {
        color = 0xFFFFFFFF;
      }
      else if(sumSquares > (rad - 2 * xInc) && sumSquares < (rad + 2 * xInc)) // Black Border line
      {
        color = 0xFF000000;
      }
      else if(fabs(x - y / m) < 0.005)
      {
        color = 0xFF000000;
      }
      else if(xIndex == 0 || yIndex == 0)
      {
        color = 0xFF000000;
      }
      else
      {
        a = (x * x + y * y + 1);
        b = (2 * x * x + 2 * y * y);
        c = (x * x + y * y - 1);

        val = (-b + std::sqrt(b * b - 4.0 * a * c)) / (2.0 * a);
        x1 = (1 + val) * x;
        y1 = (1 + val) * y;
        z1 = val;
        denom = (x1 * x1) + (y1 * y1) + (z1 * z1);
        denom = std::sqrt(denom);
        x1 = x1 / denom;
        y1 = y1 / denom;
        z1 = z1 / denom;

        color = generateIPFColor(0.0, 0.0, 0.0, x1, y1, z1, false);
      }

      pixelPtr[idx] = color;
    }
    yScanLineIndex++;
  }
  return image;
}

// -----------------------------------------------------------------------------
TrigonalOps::Pointer TrigonalOps::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::string TrigonalOps::getNameOfClass() const
{
  return std::string("TrigonalOps");
}

// -----------------------------------------------------------------------------
std::string TrigonalOps::ClassName()
{
  return std::string("TrigonalOps");
}

// -----------------------------------------------------------------------------
TrigonalOps::Pointer TrigonalOps::New()
{
  Pointer sharedPtr(new(TrigonalOps));
  return sharedPtr;
}
