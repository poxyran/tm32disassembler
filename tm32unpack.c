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


// unpackoperation() starts by unpacking an operation from an instruction into a byte array.
// The 24-bit part to the operation and the one or two byte extension are re-arranged into Little-Endian order. 
// The two extra opcode bits that are packed into the format field byte of the instruction are spliced into their
// correct places (at bits 25 and 24) in the uncompressed 42-bit operation.
//
// The splicing in of those two extra bits means that six bytes (rather than five bytes) are needed to store the
// uncompressed (42-bit) operation.
// 
// In practise, an eight byte array is used. This allows a uint64_t ptr to be used which simplifies bitwise operations
// 
uint64_t unpackoperation(uint8_t *instruction, uint16_t formatbits, uint32_t slotnumber) {
    uint8_t operation[8], ophexstring[30], opcodebits2524str[30], opsize = 0, opcodebits2524 = 0x00;
    uint16_t currentformatfield = formatbits;
    uint16_t opinsoffset = 0, opextoffset = 0;
    uint32_t *temp32ptr;
    uint64_t *temp64ptr, opint64;
    extern FILE *debugout;

    opsize = operationsize(currentformatfield, slotnumber);
    opinsoffset = operationoffset(currentformatfield,slotnumber);
    opextoffset = extensionoffset(currentformatfield,slotnumber);

    memset(operation,0x00,8);       // start afresh by clearing all bits in operation

    switch(opsize) {                // copy the operation bytes in the array
        case  0 :   return 0;       // we gotta NOP so return straight away
        case 40 :   operation[3]=*(2+instruction+(opextoffset/8)+1);
        case 32 :   operation[4]=*(2+instruction+(opextoffset/8));
        case 24 :   operation[5]=*(2+instruction+(opinsoffset/8)+2);
                    operation[6]=*(2+instruction+(opinsoffset/8)+1);
                    operation[7]=*(2+instruction+(opinsoffset/8));
                    break;
        default :   fprintf(stderr, "opsize encoding error = %d (should be 0,26,34 or 42 bits)\n", opsize + 2);
                    return -1;
    }
    
    fprintf(debugout, "\n-----------------------\n");
    fprintf(debugout, "Operation in slot #%d is %d bits long; 24-bits in bytes %d-%d ", 
                slotnumber, opsize + 2, (16+opinsoffset)/8,(16+opinsoffset+16)/8);

    if((opsize-24) /8 > 0)
        fprintf(debugout, "with %d byte extension at offset %d\n", (opsize-24)/8, (16+opextoffset)/8);
    else
        fprintf(debugout, "\n");

    switch(getrealopindex(formatbits, slotnumber)) {
        case 0 :    opcodebits2524 = (instruction[1] >> 6) & 0x03;  // in the 1st group of operations
                    break;
        case 1 :    opcodebits2524 = (instruction[1] >> 4) & 0x03;  // in the 1st group of operations
                    break;
        case 2 :    opcodebits2524 = (instruction[1] >> 2) & 0x03;  // in the 1st group of operations
                    break;
        case 3 :    opcodebits2524 = (instruction[11] >> 6) & 0x03; // in the 2nd group of operations
                    break;
        case 4 :    opcodebits2524 = (instruction[11] >> 4) & 0x03; // in the 2nd group of operations
                    break;
        default:    fprintf(stderr, "Decoding error in unpackoperation()\n");
                    return -1;
    }

    fprintf(debugout,"\n");
    if(getrealopindex(formatbits, slotnumber) < 3 )     // the operation is in the first group
        fprintf(debugout, "format byte[%d]    = 0x%02x & 0xfc = 0x%02x\n", 1 , instruction[1],  instruction[1]  & 0xfc);
    else                                                // operation must be in the second group
        fprintf(debugout, "format byte[%d]   = 0x%02x & 0xf0 = 0x%02x\n", 11, instruction[11], instruction[11] & 0xf0);

    opcodebits2524tostring(opcodebits2524, opcodebits2524str);

//  fprintf(debugout, "opcode bits [25-24] in little endian bit order: %s\n", opcodebits2524str);

    sprintf(ophexstring, "%02x %02x %02x %02x %02x %02x %02x %02x", operation[0],operation[1], operation[2],operation[3],
                                                                    operation[4], operation[5],operation[6], operation[7]);
    fprintf(debugout, "Op[63:0]          = %s\n", ophexstring);

//  now we need to left shift, then a logical OR with opcode bits 25 and 24 to obtain the full opcode for our slot.

    sprintf(ophexstring, "%01x %02x %02x %02x %02x %02x", operation[2],operation[3],operation[4], operation[5],
                                                              operation[6], operation[7]);
    fprintf(debugout, "Op[41:0]          = %s\n", ophexstring);

// swap the bytes before left shifting two bits to make room for the two extra opcode bits [25-24] from the format field

    temp32ptr = (uint32_t *) (operation + 1);
    *temp32ptr = bswap_32(bswap_32(*temp32ptr) << 2);

    sprintf(ophexstring, "%01x %02x %02x %02x %02x %02x", 
                operation[2],operation[3],operation[4], operation[5],operation[6], operation[7]);

    fprintf(debugout, "Op[41:24] << 2    = %s\n", ophexstring);

// now splice opcode bits 25 and 24 into our byte array using a logical OR

    operation[4] |= opcodebits2524;

    sprintf(ophexstring, "%01x %02x %02x %02x %02x %02x", 
                operation[2],operation[3],operation[4], operation[5],operation[6], operation[7]);

    fprintf(debugout, "Op |= (%d<<24)     = %s\n", opcodebits2524, ophexstring);

    fprintf(debugout, "Op[41:0]          = %s\n", ophexstring);

    temp64ptr = (uint64_t *) operation;
    opint64 = (uint64_t) bswap_64(*temp64ptr);

    fprintf(debugout, "(uint64_t)op>>24  = %" PRIx64 "\n", (uint64_t)(bswap_64(opint64) >> 16));

    return opint64;
}


