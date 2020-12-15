/** 
 *  \file
 *  Tools
 * ----------------------------------------------------------------------------
 *  \date       2020
 *  \author     Simon Rohou
 *  \copyright  Copyright 2020 Simon Rohou
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#ifndef __TUBEX_TOOLS_H__
#define __TUBEX_TOOLS_H__

#include <string>
#include "ibex_Interval.h"

namespace tubex
{
  /**
   * \class Tools
   * \brief Basic features provided here in order to avoid overkill dependencies
   */
  class Tools
  {
    public:

      /**
       * \brief Replaces all occurrences of the search string in the input with the format string. The input sequence is modified in-place.
       *
       * \note Same signature as boost::algorithm::replace_all
       *
       * \param input an input string
       * \param search a substring to be searched for 
       * \param format a substitute string
       * \return void
       */
      static void replace_all(std::string& input, const std::string& search, const std::string& format);
      
      /**
       * \brief Returns a random number inside an interval
       *
       * \param intv the bounds
       * \return a random double
       */
      static double rand_in_bounds(const ibex::Interval& intv);
  };
}

#endif