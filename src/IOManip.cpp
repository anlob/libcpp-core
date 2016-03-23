#include "IOManip.h"

using namespace std;


static cstrtype cs_ty = { '\"', '\\', nullptr, 0 };
template<> const cstrtype *const cstrtype::dflt = &cs_ty;

static wcstrtype wcs_ty = { L'\"', L'\\', nullptr, 0 };
template<> const wcstrtype *const wcstrtype::dflt = &wcs_ty;
