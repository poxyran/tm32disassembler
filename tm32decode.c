// An open source disassembler for the Trimedia TM3260, a five issue-slot VLIW processor core.
//
// Derived from documentation in US Patents #5,787,302, #5,826,054, #5,852,741, #5,878,267 and #6,704,859
//
// (c) 2011 asbokid <ballymunboy@gmail.com> 
//     
//   This program is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.

#if defined(__MINGW32__)
#include "windows/byteswap.h"
#else
#include <byteswap.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "tm32dis.h"
#include "tm32disinstrs.h"


// decodeoperation() takes the 64-bit unsigned integer opint64 and parses out the bit fields
// which hold the operation code, operands, parameters, predicates, etc..
uint64_t decodeoperation(uint32_t opsize, uint64_t opint64, uint8_t *opstring) {
    const struct OPERATION *op;

    if(opint64 == 0)
        sprintf(opstring, "IF r1   nop");
    else {
      switch (opsize) {
        case 24 :
            op = decodeop(OPBITS25_21(opint64));
            fprintf(debugout, "26:opcode[4:0]    = %d = %s\n", OPBITS25_21(opint64), op->opname);
            switch(op->property) {
                case BINARY_UNGUARDED_SHORT:
                case BINARY_SHORT:
                    sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", 
                        1, op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS20_14(opint64));
                    break;
                case UNARY_PARAM7_UNGUARDED_SHORT:
                case UNARY_PARAM7_SHORT:
                    sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d", 
                        1, op->opname, op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)), 
                        OPBITS6_0(opint64), OPBITS20_14(opint64));
                    break;
                case BINARY_UNGUARDED_PARAM7_RESULTLESS_SHORT:
                case BINARY_PARAM7_RESULTLESS_SHORT:
                    sprintf(opstring,"IF r%-3d %s(%d) r%d r%d", 
                        1, op->opname, op->paramfactor * signextend(OPBITS20_14(opint64)), OPBITS6_0(opint64), OPBITS13_7(opint64));
                    break;
                case UNARY_SHORT:
                    sprintf(opstring,"IF r%-3d %s r%d -> r%d", 
                        OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64));
                    break;
                default:
                    sprintf(opstring,"26: ILLEGAL OP! = %s", op->opname);
            }
            break;
        case 32 :
            switch(OPBITS33(opint64)) {     // bit 33 identies short or long opcode
                case 0: // short opcode
                    op = decodeop(OPBITS25_21(opint64));
                    fprintf(debugout, "34-0:opcode[4:0]  = %d = %s\n", OPBITS25_21(opint64), op->opname);
                    switch(op->property) {
                        case BINARY_UNGUARDED_SHORT:
                        case BINARY_SHORT:
                                sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", OPBITS20_14(opint64), op->opname,
                                 OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS32_26(opint64));
                            break;
                        case UNARY_SHORT:
                                sprintf(opstring,"IF r%-3d %s r%d -> r%d",  OPBITS20_14(opint64), op->opname, 
                                OPBITS6_0(opint64), OPBITS32_26(opint64));
                            break;
                        case UNARY_PARAM7_UNGUARDED_SHORT:
                        case UNARY_PARAM7_SHORT:
                            sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d",  OPBITS20_14(opint64), op->opname, 
                                op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                OPBITS6_0(opint64), OPBITS32_26(opint64));
                            break;
                        case BINARY_UNGUARDED_PARAM7_RESULTLESS_SHORT:
                        case BINARY_PARAM7_RESULTLESS_SHORT:
                            sprintf(opstring,"IF r%-3d %s(%d) r%d r%d", OPBITS20_14(opint64), op->opname, 
                                op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS32_26(opint64)) : OPBITS32_26(opint64)),
                                OPBITS6_0(opint64), OPBITS13_7(opint64));
                            break;
                        default:
                            sprintf(opstring,"34-0: ILLEGAL OP! = %s", op->opname);
                            break;
                    }
                    break;
                case 1:     // OPTBITS33 == 1 == long opcode in 34-bits
                    op = decodeop(OPBITS28_21(opint64));
                    fprintf(debugout, "34-1:opcode[7:0]  = %d = %s\n", OPBITS28_21(opint64), op->opname);

                        switch(op->property) {
                            case BINARY_UNGUARDED:
                                sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", 
                                    1, op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS20_14(opint64));
                                break;
                            case BINARY:
                                sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", 
                                    1, op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS20_14(opint64));
                                break;
                            case BINARY_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s r%d r%d", 
                                    OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64));
                                break;
                            case UNARY_PARAM7:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d", 
                                    OPBITS20_14(opint64), op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64), OPBITS20_14(opint64));
                                break;                          
                            case UNARY_PARAM7_UNGUARDED:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d", 
                                    1, op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64), OPBITS20_14(opint64));
                                break;                          
                            case UNARY: // case UNARY_SHORT:
                                sprintf(opstring,"IF r%-3d %s r%d -> r%d", 
                                    OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64));             
                                break;
                            case UNARY_PARAM7_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d", 
                                    OPBITS20_14(opint64), op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64));
                                break;
                            case ZEROARY_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s", 
                                    OPBITS20_14(opint64), op->opname);
                                break;
                            default:
                                sprintf(opstring,"34-1: ILLEGAL OP! = %s", op->opname);
                                break;
                        }
                    break;
                }
            break;
        case 40 : 
            switch(OPBITS33(opint64)) 
                case 1:{        // when set, bit 33 identifies <zeroary_param32> e.g. iimm/uimm
                    op = decodeop(191);
                    sprintf(opstring,"IF r%-3d %s(0x%x) -> r%d", 1, op->opname, 
                        PARAM32BITS(opint64), OPBITS20_14(opint64));
                    break;
                case 0:         // when not set, bit 33 identifies <zeroary_param32_resultless> e.g. jmpi/ijmpi
                                
                    if (!(OPBITS33(opint64)) && !(OPBITS32(opint64))) { 
                        op = (OPBITS31(opint64)) ? decodeop(179) : decodeop(178); 
                                // if bit 31 (signed flag) is set
                                // zeroary_param32_resultless (signed) == jmpi
                                // else .._param32_resultless(unsigned)== ijmpi
                        sprintf(opstring,"IF r%-3d %s(0x%x)", 
                            OPBITS20_14(opint64), op->opname, PARAM32BITS(opint64));
                    }                                                               
                    else {                  // a long opcode operation taking 42-bits
                        op = decodeop(OPBITS28_21(opint64));
                        fprintf(debugout, "42:opcode[7:0]    = %d = %s\n", OPBITS28_21(opint64), op->opname);
                        switch(op->property) {
                            case BINARY_UNGUARDED_SHORT:
                                sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", 
                                    1, op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS41_35(opint64));
                                break;
                            case UNARY_PARAM7_UNGUARDED_SHORT:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d", 
                                    1, op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64), OPBITS41_35(opint64));
                                break;
                            case BINARY_UNGUARDED_PARAM7_RESULTLESS_SHORT:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d r%d",
                                    1, op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS41_35(opint64)) : OPBITS41_35(opint64)), 
                                    OPBITS6_0(opint64), OPBITS13_7(opint64));
                                break;
                            case UNARY_SHORT:
                                sprintf(opstring,"IF r%-3d %s r%d -> r%d", 
                                    OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS41_35(opint64));
                                break;
                            case BINARY_SHORT:
                                sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", 
                                    OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS41_35(opint64));
                                break;
                            case UNARY_PARAM7_SHORT:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d", OPBITS20_14(opint64), op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64), OPBITS41_35(opint64));
                                break;
                            case BINARY_PARAM7_RESULTLESS_SHORT:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d r%d", OPBITS20_14(opint64), op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS41_35(opint64)) : OPBITS41_35(opint64)),
                                    OPBITS6_0(opint64), OPBITS13_7(opint64));
                                break;
                            case BINARY_UNGUARDED:
                                sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", 
                                    1, op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS41_35(opint64));
                                break;
                            case BINARY_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s r%d r%d", 
                                    OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64));
                                break;
                            case UNARY_PARAM7_UNGUARDED:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d", 
                                    1, op->opname, op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64), OPBITS41_35(opint64));
                                break;
                            case UNARY:
                                sprintf(opstring,"IF r%-3d %s r%d -> r%d", 
                                    OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS41_35(opint64));
                                break;
                            case BINARY_PARAM7_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d r%d", OPBITS20_14(opint64), op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS41_35(opint64)) : OPBITS41_35(opint64)), 
                                    OPBITS6_0(opint64), OPBITS13_7(opint64));
                                break;
                            case BINARY:
                                sprintf(opstring,"IF r%-3d %s r%d r%d -> r%d", 
                                    OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64), OPBITS13_7(opint64), OPBITS41_35(opint64));
                                break;
                            case UNARY_PARAM7:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d -> r%d", OPBITS20_14(opint64), op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64), OPBITS41_35(opint64));
                                break;
                            case UNARY_PARAM7_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s(%d) r%d", OPBITS20_14(opint64), op->opname, 
                                    op->paramfactor * (op->sign==SIGNED ? signextend(OPBITS13_7(opint64)) : OPBITS13_7(opint64)),
                                    OPBITS6_0(opint64));
                                break;
                            case ZEROARY:
                                sprintf(opstring,"IF r%-3d %s -> %d", OPBITS20_14(opint64), op->opname, OPBITS41_35(opint64));
                                break;
                            case ZEROARY_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s", OPBITS20_14(opint64), op->opname);
                                break;
                            case UNARY_RESULTLESS:
                                sprintf(opstring,"IF r%-3d %s %d", OPBITS20_14(opint64), op->opname, OPBITS6_0(opint64));
                                break;
                            default:
                                sprintf(opstring,"42: ILLEGAL OP! = %s", op->opname);
                        } // end of switch(op->property) {
                    } // else
                break;
            }   // case 40
            break;
        default :
            sprintf(opstring,"Unknown operation size: %d", opsize);
       }
    }

    fprintf(debugout, "OPBITS[6:0]       = %d \n", OPBITS6_0(opint64));
    fprintf(debugout, "OPBITS[13:7]      = %d \n", OPBITS13_7(opint64));
    fprintf(debugout, "OPBITS[20:14]     = %d \n", OPBITS20_14(opint64));
    fprintf(debugout, "OPBITS[28:21]     = %d \n", OPBITS28_21(opint64));
    fprintf(debugout, "OPBITS[29]        = %d \n", OPBITS29(opint64));
    fprintf(debugout, "OPBITS[32:26]     = %d\n", OPBITS32_26(opint64));
    fprintf(debugout, "OPBITS[33:32:31]  = %x:%x:%x\n", OPBITS33(opint64), OPBITS32(opint64), OPBITS31(opint64));
    fprintf(debugout, "OPBITS[41:35]     = %d\n", OPBITS41_35(opint64));
    fprintf(debugout, "opstring          = %s\n\n", opstring);
    
    return opint64;
}


