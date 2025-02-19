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

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "CtfConstants.h"
#include "CtfHeaderEntry.h"
#include "CtfPhase.h"
#include "DataParser.hpp"
#include "EbsdLib/Core/EbsdLibConstants.h"
#include "EbsdLib/Core/EbsdSetGetMacros.h"
#include "EbsdLib/EbsdLib.h"
#include "EbsdLib/IO/EbsdReader.h"

#define CTF_READER_PTR_PROP(name, var, type)                                                                                                                                                           \
  type* get##name##Pointer()                                                                                                                                                                           \
  {                                                                                                                                                                                                    \
    return static_cast<type*>(getPointerByName(#var));                                                                                                                                                 \
  }

/**
 * @class CtfReader CtfReader.h EbsdLib/IO/HKL/CtfReader.h
 * @brief This class is a self contained HKL .ctf file reader and will read a
 * single .ctf file and store all the data in column centric pointers.
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Aug 1, 2011
 * @version 1.0
 */
class EbsdLib_EXPORT CtfReader : public EbsdReader
{
public:
  CtfReader();
  ~CtfReader() override;

  /**
   * @brief Returns the name of the class for CtfReader
   */
  std::string getNameOfClass() const;
  /**
   * @brief Returns the name of the class for CtfReader
   */
  static std::string ClassName();
  using CtfIntHeaderType = CtfHeaderEntry<int, Int32HeaderParser>;
  using CtfFloatHeaderType = CtfHeaderEntry<float, FloatHeaderParser>;
  EBSDHEADER_INSTANCE_PROPERTY(CtfStringHeaderEntry, std::string, Channel, EbsdLib::Ctf::ChannelTextFile)
  EBSDHEADER_INSTANCE_PROPERTY(CtfStringHeaderEntry, std::string, Prj, EbsdLib::Ctf::Prj)
  EBSDHEADER_INSTANCE_PROPERTY(CtfStringHeaderEntry, std::string, Author, EbsdLib::Ctf::Author)
  EBSDHEADER_INSTANCE_PROPERTY(CtfStringHeaderEntry, std::string, JobMode, EbsdLib::Ctf::JobMode)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, XCells, EbsdLib::Ctf::XCells)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, YCells, EbsdLib::Ctf::YCells)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, ZCells, EbsdLib::Ctf::ZCells)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, XStep, EbsdLib::Ctf::XStep)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, YStep, EbsdLib::Ctf::YStep)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, ZStep, EbsdLib::Ctf::ZStep)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, AcqE1, EbsdLib::Ctf::AcqE1)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, AcqE2, EbsdLib::Ctf::AcqE2)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, AcqE3, EbsdLib::Ctf::AcqE3)
  EBSDHEADER_INSTANCE_PROPERTY(CtfStringHeaderEntry, std::string, Euler, EbsdLib::Ctf::Euler)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, Mag, EbsdLib::Ctf::Mag)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, Coverage, EbsdLib::Ctf::Coverage)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, Device, EbsdLib::Ctf::Device)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, KV, EbsdLib::Ctf::KV)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, TiltAngle, EbsdLib::Ctf::TiltAngle)
  EBSDHEADER_INSTANCE_PROPERTY(CtfFloatHeaderType, float, TiltAxis, EbsdLib::Ctf::TiltAxis)
  EBSDHEADER_INSTANCE_PROPERTY(CtfIntHeaderType, int, NumPhases, EbsdLib::Ctf::NumPhases)
  EBSD_INSTANCE_PROPERTY(std::vector<CtfPhase::Pointer>, PhaseVector)

  CTF_READER_PTR_PROP(Phase, Phase, int)
  CTF_READER_PTR_PROP(X, X, float)
  CTF_READER_PTR_PROP(Y, Y, float)
  CTF_READER_PTR_PROP(Z, Z, float)
  CTF_READER_PTR_PROP(BandCount, Bands, int)
  CTF_READER_PTR_PROP(Error, Error, int)
  CTF_READER_PTR_PROP(Euler1, Euler1, float)
  CTF_READER_PTR_PROP(Euler2, Euler2, float)
  CTF_READER_PTR_PROP(Euler3, Euler3, float)
  CTF_READER_PTR_PROP(MeanAngularDeviation, MAD, float)
  CTF_READER_PTR_PROP(BandContrast, BC, int)
  CTF_READER_PTR_PROP(BandSlope, BS, int)

  /* These will be in a 3D ctf file */
  CTF_READER_PTR_PROP(GrainIndex, GrainIndex, int)
  CTF_READER_PTR_PROP(GrainRandomColourR, GrainRandomColourR, int)
  CTF_READER_PTR_PROP(GrainRandomColourG, GrainRandomColourG, int)
  CTF_READER_PTR_PROP(GrainRandomColourB, GrainRandomColourB, int)

  /**
   * @brief Returns the pointer to the data for a given feature
   * @param featureName The name of the feature to return the pointer to.
   */
  void* getPointerByName(const std::string& featureName) override;
  //  void setPointerByName(const std::string& name, void* p);

  /**
   * @brief Returns an enumeration value that depicts the numerical
   * primitive type that the data is stored as (Int, Float, etc).
   * @param featureName The name of the feature.
   */
  EbsdLib::NumericTypes::Type getPointerType(const std::string& featureName) override;
  int getTypeSize(const std::string& featureName);
  DataParser::Pointer getParser(const std::string& featureName, void* ptr, size_t size);

  std::vector<std::string> getColumnNames();

  /**
   * @brief Reads the complete HKL .ctf file.
   * @return 1 on success
   */
  int readFile() override;

  /**
   * @brief Reads ONLY the header portion of the HKL .ctf file
   * @return 1 on success
   */
  int readHeaderOnly() override;

  void readOnlySliceIndex(int slice);

  int getXDimension() override;
  void setXDimension(int xdim) override;
  int getYDimension() override;
  void setYDimension(int ydim) override;

  void printHeader(std::ostream& out);

  /**
   * @brief writeFile
   * @param filepath
   */
  int writeFile(const std::string& filepath);

private:
  int m_SingleSliceRead = -1;

  std::map<std::string, DataParser::Pointer> m_NamePointerMap;

  /**
   * @brief
   * @param reader
   * @param headerLines
   * @return
   */
  int getHeaderLines(std::ifstream& reader, std::vector<std::string>& headerLines);

  /**
   * Checks that the line is the header of the columns for the data.
   *
   * @param columns
   *            line values
   * @return <code>true</code> if the line is the columns header line,
   *         <code>false</code> otherwise
   */
  bool isDataHeaderLine(const std::vector<std::string>& columns) const;

  /**
   *
   */
  int parseHeaderLines(std::vector<std::string>& headerLines);

  /**
   * @brief
   * @param in The input file stream to read from
   */
  int readData(std::ifstream& in);

  /**
   * @brief Reads a line of Data from the ASCII based file
   * @param line The current line of data
   * @param row Current Row of Data
   * @param i The current index into a flat array
   * @param xCells Number of X Data Points
   * @param yCells Number of Y Data Points
   * @param col The current Column of Data
   */
  int parseDataLine(std::string& line, size_t row, size_t col, size_t i, size_t xCells, size_t yCells);

public:
  CtfReader(const CtfReader&) = delete;            // Copy Constructor Not Implemented
  CtfReader(CtfReader&&) = delete;                 // Move Constructor Not Implemented
  CtfReader& operator=(const CtfReader&) = delete; // Copy Assignment Not Implemented
  CtfReader& operator=(CtfReader&&) = delete;      // Move Assignment Not Implemented
};
