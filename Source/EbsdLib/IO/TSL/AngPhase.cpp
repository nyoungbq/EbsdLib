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

#include "AngPhase.h"

#include <cstring>

#include "EbsdLib/Core/EbsdLibConstants.h"
#include "EbsdLib/IO/TSL/AngConstants.h"
#include "EbsdLib/Utilities/EbsdStringUtils.hpp"

HKLFamily::HKLFamily() = default;
HKLFamily::~HKLFamily() = default;

// -----------------------------------------------------------------------------
void HKLFamily::printSelf(std::stringstream& stream) const
{
  stream << EbsdLib::Ang::HKLFamilies;
  stream << " " << h << " " << k << " " << l << " " << diffractionIntensity << " " << (int)(s1) << " " << (int)(s2) << "\n";
}

// -----------------------------------------------------------------------------
void HKLFamily::copyToStruct(HKLFamily_t* ptr)
{
  ptr->h = h;
  ptr->k = k;
  ptr->l = l;
  ptr->s1 = s1;
  ptr->diffractionIntensity = diffractionIntensity;
  ptr->s2 = s2;
}

// -----------------------------------------------------------------------------
void HKLFamily::copyFromStruct(HKLFamily_t* ptr)
{
  h = ptr->h;
  k = ptr->k;
  l = ptr->l;
  s1 = ptr->s1;
  diffractionIntensity = ptr->diffractionIntensity;
  s2 = ptr->s2;
}

// -----------------------------------------------------------------------------
HKLFamily::Pointer HKLFamily::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::string HKLFamily::getNameOfClass() const
{
  return std::string("HKLFamily");
}

// -----------------------------------------------------------------------------
std::string HKLFamily::ClassName()
{
  return std::string("HKLFamily");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AngPhase::AngPhase()
: m_PhaseIndex(-1)
, m_Symmetry(0)
, m_NumberFamilies(0)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AngPhase::~AngPhase() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::parseMaterialName(std::vector<std::string>& tokens)
{
  m_MaterialName.clear();
  tokens.erase(tokens.begin());
  for(const auto& token : tokens)
  {
    m_MaterialName += token + " ";
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::parseFormula(std::vector<std::string>& tokens)
{
  m_Formula.clear();
  tokens.erase(tokens.begin());
  for(const auto& token : tokens)
  {
    m_Formula += token + " ";
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// void AngPhase::parseInfo(std::vector<std::string>& tokens)
//{
//  m_Info.clear();
//  for(const auto& token : tokens)
//  {
//    m_Info += token + " ";
//  }
//}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// void AngPhase::parseSymmetry(char* value, size_t start, size_t length)
//{
//  if (value[start] == ':')
//  {
//    ++start;
//  } // move past the ":" character
//  ByteArrayType data(&(value[start]), strlen(value) - start);
//  bool ok = false;
//  m_Symmetry = data.toUInt(&ok, 10);
//}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::parseLatticeConstants(std::vector<std::string>& tokens)
{
  m_LatticeConstants.clear();

  m_LatticeConstants.push_back(std::stof(tokens[1])); // A
  m_LatticeConstants.push_back(std::stof(tokens[2])); // B
  m_LatticeConstants.push_back(std::stof(tokens[3])); // C
  m_LatticeConstants.push_back(std::stof(tokens[4])); // Alpha
  m_LatticeConstants.push_back(std::stof(tokens[5])); // Beta
  m_LatticeConstants.push_back(std::stof(tokens[6])); // Gamma
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
// void AngPhase::parseNumberFamilies(char* value, size_t start, size_t length)
//{
//  if (value[start] == ':')
//  {
//    ++start;
//  } // move past the ":" character
//  ByteArrayType data(&(value[start]), strlen(value) - start);
//  bool ok = false;
//  m_NumberFamilies = data);
//}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::parseHKLFamilies(std::vector<std::string>& tokens)
{
  HKLFamily::Pointer family = HKLFamily::New();
  try
  {
    family->h = std::stoi(tokens[1]);
    family->k = std::stoi(tokens[2]);
    family->l = std::stoi(tokens[3]);
    family->s1 = std::stoi(tokens[4]);
    family->diffractionIntensity = std::stof(tokens[5]);
    if(tokens.size() > 6)
    {
      family->s2 = tokens[6].at(0);
    }
    if(family->s1 > 1)
    {
      family->s1 = 1;
    }
    if(family->s2 > 1)
    {
      family->s2 = 1;
    }
  } catch(std::invalid_argument& e)
  {
    std::cout << e.what() << std::endl;

  } catch(std::out_of_range& e)
  {
    std::cout << e.what() << std::endl;
  }

  m_HKLFamilies.push_back(family);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::parseCategories(std::vector<std::string>& tokens)
{
  m_Categories.clear();
  try
  {
    if(tokens[0].size() != EbsdLib::Ang::Categories.size())
    {
      tokens[0] = EbsdStringUtils::replace(tokens[0], EbsdLib::Ang::Categories, "");
      m_Categories.push_back(std::stoi(tokens.at(0)));
    }
    for(size_t i = 1; i < tokens.size(); ++i)
    {
      m_Categories.push_back(std::stoi(tokens.at(i)));
    }
  } catch(std::invalid_argument& e)
  {
    std::cout << e.what() << std::endl;

  } catch(std::out_of_range& e)
  {
    std::cout << e.what() << std::endl;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::printSelf(std::stringstream& stream)
{
  stream << EbsdLib::Ang::Phase << ": " << m_PhaseIndex << std::string("\n");
  stream << EbsdLib::Ang::MaterialName << ": " << m_MaterialName << std::string("\n");
  stream << EbsdLib::Ang::Formula << ": " << m_Formula << std::string("\n");
  // stream << EbsdLib::Ang::Info << ": " << m_Info << std::string("\n");
  stream << EbsdLib::Ang::Symmetry << ": " << m_Symmetry << std::string("\n");

  stream << EbsdLib::Ang::LatticeConstants;

  for(const auto& latticeConstant : m_LatticeConstants)
  {
    stream << " " << latticeConstant;
  }
  stream << std::string("\n");

  stream << EbsdLib::Ang::NumberFamilies << ": " << m_NumberFamilies << std::string("\n");

  for(const auto& family : m_HKLFamilies)
  {
    family->printSelf(stream);
  }

  stream << EbsdLib::Ang::Categories;
  for(const auto& category : m_Categories)
  {
    stream << " " << category;
  }
  stream << std::string("\n");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
unsigned int AngPhase::determineLaueGroup()
{
  uint32_t symmetry = getSymmetry();
  unsigned int crystal_structure = EbsdLib::CrystalStructure::UnknownCrystalStructure;

  switch(symmetry)
  {
  case EbsdLib::Ang::PhaseSymmetry::Cubic:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_O:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_Td:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_Oh:
    crystal_structure = EbsdLib::CrystalStructure::Cubic_High;
    break;
  case EbsdLib::Ang::PhaseSymmetry::Tetrahedral:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_T:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_Th:
    crystal_structure = EbsdLib::CrystalStructure::Cubic_Low;
    break;
  case EbsdLib::Ang::PhaseSymmetry::DiTetragonal:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D4:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C4v:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D2d:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D4h:
    crystal_structure = EbsdLib::CrystalStructure::Tetragonal_High;
    break;
  case EbsdLib::Ang::PhaseSymmetry::Tetragonal:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C4:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_S4:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C4h:
    crystal_structure = EbsdLib::CrystalStructure::Tetragonal_Low;
    break;
  case EbsdLib::Ang::PhaseSymmetry::Orthorhombic:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D2:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C2v:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D2h:
    crystal_structure = EbsdLib::CrystalStructure::OrthoRhombic;
    break;
  case EbsdLib::Ang::PhaseSymmetry::Monoclinic_c:
  case EbsdLib::Ang::PhaseSymmetry::Monoclinic_b:
  case EbsdLib::Ang::PhaseSymmetry::Monoclinic_a:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C2_c:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C1h_c:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C2h_c:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C2_b:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C1h_b:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C2h_b:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C2_a:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C1h_a:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C2h_a:
    crystal_structure = EbsdLib::CrystalStructure::Monoclinic;
    break;
  case EbsdLib::Ang::PhaseSymmetry::Triclinic:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C1:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_S2:
    crystal_structure = EbsdLib::CrystalStructure::Triclinic;
    break;
  case EbsdLib::Ang::PhaseSymmetry::DiHexagonal:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D6:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C6v:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D3h:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D6h:
    crystal_structure = EbsdLib::CrystalStructure::Hexagonal_High;
    break;
  case EbsdLib::Ang::PhaseSymmetry::Hexagonal:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C6:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C3h:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C6h:
    crystal_structure = EbsdLib::CrystalStructure::Hexagonal_Low;
    break;
  case EbsdLib::Ang::PhaseSymmetry::DiTrigonal:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D3:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C3v:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_D3d:
    crystal_structure = EbsdLib::CrystalStructure::Trigonal_High;
    break;
  case EbsdLib::Ang::PhaseSymmetry::Trigonal:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_C3:
  case EbsdLib::Ang::PhaseSymmetry::k_Sym_S6:
    crystal_structure = EbsdLib::CrystalStructure::Trigonal_Low;
    break;

  default:
    crystal_structure = EbsdLib::CrystalStructure::UnknownCrystalStructure;
  }
  return crystal_structure;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::setLatticeConstantA(float a)
{
  m_LatticeConstants[0] = a;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::setLatticeConstantB(float a)
{
  m_LatticeConstants[1] = a;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::setLatticeConstantC(float a)
{
  m_LatticeConstants[2] = a;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::setLatticeConstantAlpha(float a)
{
  m_LatticeConstants[3] = a;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::setLatticeConstantBeta(float a)
{
  m_LatticeConstants[4] = a;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AngPhase::setLatticeConstantGamma(float a)
{
  m_LatticeConstants[5] = a;
}

// -----------------------------------------------------------------------------
AngPhase::Pointer AngPhase::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
void AngPhase::setMaterialName(const std::string& value)
{
  m_MaterialName = value;
}

// -----------------------------------------------------------------------------
std::string AngPhase::getMaterialName() const
{
  return m_MaterialName;
}

// -----------------------------------------------------------------------------
void AngPhase::setFormula(const std::string& value)
{
  m_Formula = value;
}

// -----------------------------------------------------------------------------
std::string AngPhase::getFormula() const
{
  return m_Formula;
}

//// -----------------------------------------------------------------------------
// void AngPhase::setInfo(const std::string& value)
//{
//  m_Info = value;
//}

//// -----------------------------------------------------------------------------
// std::string AngPhase::getInfo() const
//{
//  return m_Info;
//}

// -----------------------------------------------------------------------------
std::string AngPhase::getNameOfClass() const
{
  return std::string("AngPhase");
}

// -----------------------------------------------------------------------------
std::string AngPhase::ClassName()
{
  return std::string("AngPhase");
}
