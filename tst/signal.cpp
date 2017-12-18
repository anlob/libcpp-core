#include "../src/Signal.h"
#include "signal.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_PROC == 1) || (_TEST_SIG_MASK == 1)
void test_sig_mask()
{
  SigMask mask(SigSetData().setall());
}
#endif
