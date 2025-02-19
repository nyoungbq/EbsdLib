/* ============================================================================
 * Copyright (c) 2017 BlueQuartz Software, LLC
 * All rights reserved.
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
 * Neither the names of any of the BlueQuartz Software contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#pragma once

#include <memory>

#include <string>

#include "EbsdLib/EbsdLib.h"

class EbsdLib_EXPORT LambertUtilities
{
public:
  using Self = LambertUtilities;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static Pointer New();

  /**
   * @brief Returns the name of the class for LambertUtilities
   */
  virtual std::string getNameOfClass() const;
  /**
   * @brief Returns the name of the class for LambertUtilities
   */
  static std::string ClassName();

  virtual ~LambertUtilities();

  using EnumType = uint32_t;

  enum class Hemisphere : EnumType
  {
    North = 0,
    South = 1
  };

  /**
   * @brief This function will convert a vertex that is assumed to be part of a
   * LambertSquare (a,b,0) to a Sphere with (x, y, z) cartesian coordiate. The
   * input vertex value should fall between -sqrt(PI/2) <= (x,y) <= sqrt(PI/2)
   * otherwise it is undefined behavior.
   * @param vert Input vertex which will be over written
   * @param hemi
   * @return
   */
  static int32_t LambertSquareVertToSphereVert(float* vert, Hemisphere hemi);

protected:
  LambertUtilities();

public:
  LambertUtilities(const LambertUtilities&) = delete;            // Copy Constructor Not Implemented
  LambertUtilities(LambertUtilities&&) = delete;                 // Move Constructor Not Implemented
  LambertUtilities& operator=(const LambertUtilities&) = delete; // Copy Assignment Not Implemented
  LambertUtilities& operator=(LambertUtilities&&) = delete;      // Move Assignment Not Implemented

private:
};
