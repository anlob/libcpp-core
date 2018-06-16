/** \file IOManip.cpp
 * IO manipulator stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see LICENSES at top level directory
 * \see http://www.gnu.org/licenses
 */
#include "IOManip.h"

using namespace std;


static cstrtype cs_ty = { '\"', '\\', nullptr, 0 };
template<> const cstrtype *const cstrtype::dflt = &cs_ty;

static wcstrtype wcs_ty = { L'\"', L'\\', nullptr, 0 };
template<> const wcstrtype *const wcstrtype::dflt = &wcs_ty;
