// Intel8086 header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef Intel8086_H
#define Intel8086_H

//
// Intel8086 GPR registers
//

enum
{
    REG_Intel8086_AX,
    REG_Intel8086_BX,
    REG_Intel8086_CX,
    REG_Intel8086_DX,
    REG_Intel8086_AH,
    REG_Intel8086_AL,
    REG_Intel8086_BL,
    REG_Intel8086_BH,
    REG_Intel8086_CH,
    REG_Intel8086_CL,
    REG_Intel8086_DH,
    REG_Intel8086_DL,
    REG_Intel8086_DI,
    REG_Intel8086_SI,
    REG_Intel8086_BP,
    REG_Intel8086_SP,
};

//
//  Intel8086 relocation types
//

enum
{
    R_Intel8086_NONE = 0,
};

#endif
