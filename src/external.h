#include "types.h"
#include "z80.h"

byte external_IN(reg_pair port);
void external_OUT(reg_pair port, byte val);