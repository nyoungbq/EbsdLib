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

#include "MonoclinicOps.h"

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

namespace Monoclinic
{

static const std::array<size_t, 3> OdfNumBins = {72, 36, 72}; // Represents a 5Deg bin

static const std::array<double, 3> OdfDimInitValue = {std::pow((0.7f * ((EbsdLib::Constants::k_PiD)-std::sin((EbsdLib::Constants::k_PiD)))), (1.0 / 3.0)),
                                                      std::pow((0.75 * ((EbsdLib::Constants::k_PiOver2D)-std::sin((EbsdLib::Constants::k_PiOver2D)))), (1.0 / 3.0)),
                                                      std::pow((0.75 * ((EbsdLib::Constants::k_PiD)-std::sin((EbsdLib::Constants::k_PiD)))), (1.0 / 3.0))};
static const std::array<double, 3> OdfDimStepValue = {OdfDimInitValue[0] / static_cast<double>(OdfNumBins[0] / 2), OdfDimInitValue[1] / static_cast<double>(OdfNumBins[1] / 2),
                                                      OdfDimInitValue[2] / static_cast<double>(OdfNumBins[2] / 2)};

static const int symSize0 = 2;
static const int symSize1 = 2;
static const int symSize2 = 2;

static const int k_OdfSize = 186624;
static const int k_MdfSize = 186624;
static const int k_SymOpsCount = 2;
static const int k_NumMdfBins = 36;

static const std::vector<QuatD> QuatSym = {QuatD(0.000000000, 0.000000000, 0.000000000, 1.000000000), QuatD(0.000000000, 1.000000000, 0.000000000, 0.000000000)};

static const std::vector<OrientationD> RodSym = {{0.0, 0.0, 0.0}, {0.0, 10000000000.0, 0.0}};

static const double MatSym[k_SymOpsCount][3][3] = {{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}},

                                                   {{-1.0, 0.0, 0.0}, {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}}};

} // namespace Monoclinic

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MonoclinicOps::MonoclinicOps() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MonoclinicOps::~MonoclinicOps() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool MonoclinicOps::getHasInversion() const
{
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int MonoclinicOps::getODFSize() const
{
  return Monoclinic::k_OdfSize;
}

// -----------------------------------------------------------------------------
std::array<int32_t, 3> MonoclinicOps::getNumSymmetry() const
{
  return {Monoclinic::symSize0, Monoclinic::symSize1, Monoclinic::symSize2};
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int MonoclinicOps::getMDFSize() const
{
  return Monoclinic::k_MdfSize;
}

// -----------------------------------------------------------------------------
int MonoclinicOps::getMdfPlotBins() const
{
  return Monoclinic::k_NumMdfBins;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int MonoclinicOps::getNumSymOps() const
{
  return Monoclinic::k_SymOpsCount;
}

// -----------------------------------------------------------------------------
std::array<size_t, 3> MonoclinicOps::getOdfNumBins() const
{
  return Monoclinic::OdfNumBins;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::string MonoclinicOps::getSymmetryName() const
{
  return "Monoclinic 2/m";
  ;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationD MonoclinicOps::calculateMisorientation(const QuatD& q1, const QuatD& q2) const
{
  return calculateMisorientationInternal(Monoclinic::QuatSym, q1, q2);
}

// -----------------------------------------------------------------------------
OrientationF MonoclinicOps::calculateMisorientation(const QuatF& q1f, const QuatF& q2f) const

{
  QuatD q1 = q1f.to<double>();
  QuatD q2 = q2f.to<double>();
  OrientationD axisAngle = calculateMisorientationInternal(Monoclinic::QuatSym, q1, q2);
  return axisAngle;
}

QuatD MonoclinicOps::getQuatSymOp(int32_t i) const
{
  return Monoclinic::QuatSym[i];
}

void MonoclinicOps::getRodSymOp(int i, double* r) const
{
  r[0] = Monoclinic::RodSym[i][0];
  r[1] = Monoclinic::RodSym[i][1];
  r[2] = Monoclinic::RodSym[i][2];
}

EbsdLib::Matrix3X3D MonoclinicOps::getMatSymOpD(int i) const
{
  return {Monoclinic::MatSym[i][0][0], Monoclinic::MatSym[i][0][1], Monoclinic::MatSym[i][0][2], Monoclinic::MatSym[i][1][0], Monoclinic::MatSym[i][1][1],
          Monoclinic::MatSym[i][1][2], Monoclinic::MatSym[i][2][0], Monoclinic::MatSym[i][2][1], Monoclinic::MatSym[i][2][2]};
}

EbsdLib::Matrix3X3F MonoclinicOps::getMatSymOpF(int i) const
{
  return {static_cast<float>(Monoclinic::MatSym[i][0][0]), static_cast<float>(Monoclinic::MatSym[i][0][1]), static_cast<float>(Monoclinic::MatSym[i][0][2]),
          static_cast<float>(Monoclinic::MatSym[i][1][0]), static_cast<float>(Monoclinic::MatSym[i][1][1]), static_cast<float>(Monoclinic::MatSym[i][1][2]),
          static_cast<float>(Monoclinic::MatSym[i][2][0]), static_cast<float>(Monoclinic::MatSym[i][2][1]), static_cast<float>(Monoclinic::MatSym[i][2][2])};
}

void MonoclinicOps::getMatSymOp(int i, double g[3][3]) const
{
  g[0][0] = Monoclinic::MatSym[i][0][0];
  g[0][1] = Monoclinic::MatSym[i][0][1];
  g[0][2] = Monoclinic::MatSym[i][0][2];
  g[1][0] = Monoclinic::MatSym[i][1][0];
  g[1][1] = Monoclinic::MatSym[i][1][1];
  g[1][2] = Monoclinic::MatSym[i][1][2];
  g[2][0] = Monoclinic::MatSym[i][2][0];
  g[2][1] = Monoclinic::MatSym[i][2][1];
  g[2][2] = Monoclinic::MatSym[i][2][2];
}

void MonoclinicOps::getMatSymOp(int i, float g[3][3]) const
{
  g[0][0] = static_cast<float>(Monoclinic::MatSym[i][0][0]);
  g[0][1] = static_cast<float>(Monoclinic::MatSym[i][0][1]);
  g[0][2] = static_cast<float>(Monoclinic::MatSym[i][0][2]);
  g[1][0] = static_cast<float>(Monoclinic::MatSym[i][1][0]);
  g[1][1] = static_cast<float>(Monoclinic::MatSym[i][1][1]);
  g[1][2] = static_cast<float>(Monoclinic::MatSym[i][1][2]);
  g[2][0] = static_cast<float>(Monoclinic::MatSym[i][2][0]);
  g[2][1] = static_cast<float>(Monoclinic::MatSym[i][2][1]);
  g[2][2] = static_cast<float>(Monoclinic::MatSym[i][2][2]);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType MonoclinicOps::getODFFZRod(const OrientationType& rod) const
{
  return _calcRodNearestOrigin(Monoclinic::RodSym, rod);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType MonoclinicOps::getMDFFZRod(const OrientationType& inRod) const
{
  throw EbsdLib::method_not_implemented("MonoclinicOps::getMDFFZRod not implemented");

  double w = 0.0, n1 = 0.0, n2 = 0.0, n3 = 0.0;
  double FZw = 0.0, FZn1 = 0.0, FZn2 = 0.0, FZn3 = 0.0;

  OrientationType rod = LaueOps::_calcRodNearestOrigin(Monoclinic::RodSym, inRod);
  OrientationType ax = OrientationTransformation::ro2ax<OrientationType, OrientationType>(rod);
  n1 = ax[0];
  n2 = ax[1], n3 = ax[2], w = ax[3];

  /// FIXME: Are we missing code for MonoclinicOps MDF FZ Rodrigues calculation?

  return OrientationTransformation::ax2ro<OrientationType, OrientationType>(OrientationType(FZn1, FZn2, FZn3, FZw));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QuatD MonoclinicOps::getNearestQuat(const QuatD& q1, const QuatD& q2) const
{
  return _calcNearestQuat(Monoclinic::QuatSym, q1, q2);
}

QuatF MonoclinicOps::getNearestQuat(const QuatF& q1f, const QuatF& q2f) const
{
  return _calcNearestQuat(Monoclinic::QuatSym, q1f.to<double>(), q2f.to<double>()).to<float>();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int MonoclinicOps::getMisoBin(const OrientationType& rod) const
{
  double dim[3];
  double bins[3];
  double step[3];

  OrientationType ho = OrientationTransformation::ro2ho<OrientationType, OrientationType>(rod);

  dim[0] = Monoclinic::OdfDimInitValue[0];
  dim[1] = Monoclinic::OdfDimInitValue[1];
  dim[2] = Monoclinic::OdfDimInitValue[2];
  step[0] = Monoclinic::OdfDimStepValue[0];
  step[1] = Monoclinic::OdfDimStepValue[1];
  step[2] = Monoclinic::OdfDimStepValue[2];
  bins[0] = static_cast<double>(Monoclinic::OdfNumBins[0]);
  bins[1] = static_cast<double>(Monoclinic::OdfNumBins[1]);
  bins[2] = static_cast<double>(Monoclinic::OdfNumBins[2]);

  return _calcMisoBin(dim, bins, step, ho);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType MonoclinicOps::determineEulerAngles(double random[3], int choose) const
{
  double init[3];
  double step[3];
  int32_t phi[3];
  double h1, h2, h3;

  init[0] = Monoclinic::OdfDimInitValue[0];
  init[1] = Monoclinic::OdfDimInitValue[1];
  init[2] = Monoclinic::OdfDimInitValue[2];
  step[0] = Monoclinic::OdfDimStepValue[0];
  step[1] = Monoclinic::OdfDimStepValue[1];
  step[2] = Monoclinic::OdfDimStepValue[2];

  phi[0] = static_cast<int32_t>(choose % Monoclinic::OdfNumBins[0]);
  phi[1] = static_cast<int32_t>((choose / Monoclinic::OdfNumBins[0]) % Monoclinic::OdfNumBins[1]);
  phi[2] = static_cast<int32_t>(choose / (Monoclinic::OdfNumBins[0] * Monoclinic::OdfNumBins[1]));

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
OrientationType MonoclinicOps::randomizeEulerAngles(const OrientationType& synea) const
{
  size_t symOp = getRandomSymmetryOperatorIndex(Monoclinic::k_SymOpsCount);
  QuatD quat = OrientationTransformation::eu2qu<OrientationType, QuatD>(synea);
  QuatD qc = Monoclinic::QuatSym[symOp] * quat;
  return OrientationTransformation::qu2eu<QuatD, OrientationType>(qc);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
OrientationType MonoclinicOps::determineRodriguesVector(double random[3], int choose) const
{
  double init[3];
  double step[3];
  int32_t phi[3];
  double h1, h2, h3;

  init[0] = Monoclinic::OdfDimInitValue[0];
  init[1] = Monoclinic::OdfDimInitValue[1];
  init[2] = Monoclinic::OdfDimInitValue[2];
  step[0] = Monoclinic::OdfDimStepValue[0];
  step[1] = Monoclinic::OdfDimStepValue[1];
  step[2] = Monoclinic::OdfDimStepValue[2];
  phi[0] = static_cast<int32_t>(choose % Monoclinic::OdfNumBins[0]);
  phi[1] = static_cast<int32_t>((choose / Monoclinic::OdfNumBins[0]) % Monoclinic::OdfNumBins[1]);
  phi[2] = static_cast<int32_t>(choose / (Monoclinic::OdfNumBins[0] * Monoclinic::OdfNumBins[1]));

  _calcDetermineHomochoricValues(random, init, step, phi, h1, h2, h3);
  OrientationType ho(h1, h2, h3);
  OrientationType ro = OrientationTransformation::ho2ro<OrientationType, OrientationType>(ho);
  ro = getMDFFZRod(ro);
  return ro;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int MonoclinicOps::getOdfBin(const OrientationType& rod) const
{
  double dim[3];
  double bins[3];
  double step[3];

  OrientationType ho = OrientationTransformation::ro2ho<OrientationType, OrientationType>(rod);

  dim[0] = Monoclinic::OdfDimInitValue[0];
  dim[1] = Monoclinic::OdfDimInitValue[1];
  dim[2] = Monoclinic::OdfDimInitValue[2];
  step[0] = Monoclinic::OdfDimStepValue[0];
  step[1] = Monoclinic::OdfDimStepValue[1];
  step[2] = Monoclinic::OdfDimStepValue[2];
  bins[0] = static_cast<double>(Monoclinic::OdfNumBins[0]);
  bins[1] = static_cast<double>(Monoclinic::OdfNumBins[1]);
  bins[2] = static_cast<double>(Monoclinic::OdfNumBins[2]);

  return _calcODFBin(dim, bins, step, ho);
}

void MonoclinicOps::getSchmidFactorAndSS(double load[3], double& schmidfactor, double angleComps[2], int& slipsys) const
{
  schmidfactor = 0;
  slipsys = 0;
}

void MonoclinicOps::getSchmidFactorAndSS(double load[3], double plane[3], double direction[3], double& schmidfactor, double angleComps[2], int& slipsys) const
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
  for(int i = 0; i < Monoclinic::k_SymOpsCount; i++)
  {
    // compute slip system
    double slipPlane[3] = {0};
    slipPlane[2] = Monoclinic::MatSym[i][2][0] * plane[0] + Monoclinic::MatSym[i][2][1] * plane[1] + Monoclinic::MatSym[i][2][2] * plane[2];

    // dont consider negative z planes (to avoid duplicates)
    if(slipPlane[2] >= 0)
    {
      slipPlane[0] = Monoclinic::MatSym[i][0][0] * plane[0] + Monoclinic::MatSym[i][0][1] * plane[1] + Monoclinic::MatSym[i][0][2] * plane[2];
      slipPlane[1] = Monoclinic::MatSym[i][1][0] * plane[0] + Monoclinic::MatSym[i][1][1] * plane[1] + Monoclinic::MatSym[i][1][2] * plane[2];

      double slipDirection[3] = {0};
      slipDirection[0] = Monoclinic::MatSym[i][0][0] * direction[0] + Monoclinic::MatSym[i][0][1] * direction[1] + Monoclinic::MatSym[i][0][2] * direction[2];
      slipDirection[1] = Monoclinic::MatSym[i][1][0] * direction[0] + Monoclinic::MatSym[i][1][1] * direction[1] + Monoclinic::MatSym[i][1][2] * direction[2];
      slipDirection[2] = Monoclinic::MatSym[i][2][0] * direction[0] + Monoclinic::MatSym[i][2][1] * direction[1] + Monoclinic::MatSym[i][2][2] * direction[2];

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

double MonoclinicOps::getmPrime(const QuatD& q1, const QuatD& q2, double LD[3]) const
{
  return 0.0;
}

double MonoclinicOps::getF1(const QuatD& q1, const QuatD& q2, double LD[3], bool maxS) const
{
  return 0.0;
}

double MonoclinicOps::getF1spt(const QuatD& q1, const QuatD& q2, double LD[3], bool maxS) const
{
  return 0.0;
}

double MonoclinicOps::getF7(const QuatD& q1, const QuatD& q2, double LD[3], bool maxS) const
{
  return 0.0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

namespace Monoclinic
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
      direction[0] = 1.0;
      direction[1] = 0.0;
      direction[2] = 0.0;
      (gTranspose * direction).copyInto<float>(m_xyz011->getPointer(i * 6));
      std::transform(m_xyz011->getPointer(i * 6), m_xyz011->getPointer(i * 6 + 3),
                     m_xyz011->getPointer(i * 6 + 3),             // write to the next triplet in memory
                     [](float value) { return value *= -1.0F; }); // Multiply each value by -1.0

      // -----------------------------------------------------------------------------
      // 111 Family
      direction[0] = 0.0;
      direction[1] = 1.0;
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
} // namespace Monoclinic

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MonoclinicOps::generateSphereCoordsFromEulers(EbsdLib::FloatArrayType* eulers, EbsdLib::FloatArrayType* xyz001, EbsdLib::FloatArrayType* xyz011, EbsdLib::FloatArrayType* xyz111) const
{
  size_t nOrientations = eulers->getNumberOfTuples();

  // Sanity Check the size of the arrays
  if(xyz001->getNumberOfTuples() < nOrientations * Monoclinic::symSize0)
  {
    xyz001->resizeTuples(nOrientations * Monoclinic::symSize0 * 3);
  }
  if(xyz011->getNumberOfTuples() < nOrientations * Monoclinic::symSize1)
  {
    xyz011->resizeTuples(nOrientations * Monoclinic::symSize1 * 3);
  }
  if(xyz111->getNumberOfTuples() < nOrientations * Monoclinic::symSize2)
  {
    xyz111->resizeTuples(nOrientations * Monoclinic::symSize2 * 3);
  }

#ifdef EbsdLib_USE_PARALLEL_ALGORITHMS
  tbb::parallel_for(tbb::blocked_range<size_t>(0, nOrientations), Monoclinic::GenerateSphereCoordsImpl(eulers, xyz001, xyz011, xyz111), tbb::auto_partitioner());
#else
  Monoclinic::GenerateSphereCoordsImpl serial(eulers, xyz001, xyz011, xyz111);
  serial.generate(0, nOrientations);
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool MonoclinicOps::inUnitTriangle(double eta, double chi) const
{
  return !(eta < 0 || eta > (180.0 * EbsdLib::Constants::k_PiOver180D) || chi < 0 || chi > (90.0 * EbsdLib::Constants::k_PiOver180D));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EbsdLib::Rgb MonoclinicOps::generateIPFColor(double* eulers, double* refDir, bool convertDegrees) const
{
  return generateIPFColor(eulers[0], eulers[1], eulers[2], refDir[0], refDir[1], refDir[2], convertDegrees);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EbsdLib::Rgb MonoclinicOps::generateIPFColor(double phi1, double phi, double phi2, double refDir0, double refDir1, double refDir2, bool degToRad) const
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

  for(int j = 0; j < Monoclinic::k_SymOpsCount; j++)
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

  double etaMin = 0.0;
  double etaMax = 180.0;
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
EbsdLib::Rgb MonoclinicOps::generateRodriguesColor(double r1, double r2, double r3) const
{
  double range1 = 2.0f * Monoclinic::OdfDimInitValue[0];
  double range2 = 2.0f * Monoclinic::OdfDimInitValue[1];
  double range3 = 2.0f * Monoclinic::OdfDimInitValue[2];
  double max1 = range1 / 2.0f;
  double max2 = range2 / 2.0f;
  double max3 = range3 / 2.0f;
  double red = (r1 + max1) / range1;
  double green = (r2 + max2) / range2;
  double blue = (r3 + max3) / range3;

  // Scale values from 0 to 1.0
  red = red / max1;
  green = green / max1;
  blue = blue / max2;

  return EbsdLib::RgbColor::dRgb(static_cast<int32_t>(red * 255), static_cast<int32_t>(green * 255), static_cast<int32_t>(blue * 255), 255);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::array<std::string, 3> MonoclinicOps::getDefaultPoleFigureNames() const
{
return {"<001>", "<100>", "<010>"};
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<EbsdLib::UInt8ArrayType::Pointer> MonoclinicOps::generatePoleFigure(PoleFigureConfiguration_t& config) const
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
  EbsdLib::FloatArrayType::Pointer xyz001 = EbsdLib::FloatArrayType::CreateArray(numOrientations * Monoclinic::symSize0, dims, label0 + std::string("xyzCoords"), true);
  // this is size for CUBIC ONLY, <011> Family
  EbsdLib::FloatArrayType::Pointer xyz011 = EbsdLib::FloatArrayType::CreateArray(numOrientations * Monoclinic::symSize1, dims, label1 + std::string("xyzCoords"), true);
  // this is size for CUBIC ONLY, <111> Family
  EbsdLib::FloatArrayType::Pointer xyz111 = EbsdLib::FloatArrayType::CreateArray(numOrientations * Monoclinic::symSize2, dims, label2 + std::string("xyzCoords"), true);

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
EbsdLib::UInt8ArrayType::Pointer MonoclinicOps::generateIPFTriangleLegend(int imageDim) const
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

      x = -1.0f + 2.0f * xIndex * xInc;
      y = 2.0f * yIndex * yInc;

      double sumSquares = (x * x) + (y * y);
      if(sumSquares > 1.0) // Outside unit circle
      {
        color = 0xFFFFFFFF;
      }
      else if(sumSquares > (rad - 2 * xInc) && sumSquares < (rad + 2 * xInc)) // Black Border line
      {
        color = 0xFF000000;
      }

      else if(xIndex == 0) // Black Border line
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
MonoclinicOps::Pointer MonoclinicOps::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::string MonoclinicOps::getNameOfClass() const
{
  return std::string("MonoclinicOps");
}

// -----------------------------------------------------------------------------
std::string MonoclinicOps::ClassName()
{
  return std::string("MonoclinicOps");
}

// -----------------------------------------------------------------------------
MonoclinicOps::Pointer MonoclinicOps::New()
{
  Pointer sharedPtr(new(MonoclinicOps));
  return sharedPtr;
}
