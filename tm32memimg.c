// An open source disassembler for the Trimedia TM3260, a five issue-slot VLIW processor core.
//
// More information in US Patents #5,787,302, #5,826,054, #5,852,741, #5,878,267 and #6,704,859
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
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "tm32dis.h"
#include "tm32disinstrs.h"


// for memory images, before disassembly, we need to transpose the instruction stream bits
// from the 32 byte "bit-striped" blocks into standard bit and byte sequential order
int extractmemimginstructions(uint8_t *objbuf, uint8_t *objbigendbuf, uint32_t dismcount) {

    uint8_t *inptr, *outptr, tempc;
    uint32_t i, j;
    for(i=0; i < dismcount; i++)
        for(j=0;j<8;j++) {
            inptr  = objbuf + i;
            outptr = objbigendbuf + (i/32)*32 + (j*sizeof(uint32_t)) + ((i - ((i/32)*32))/8);
            tempc = *inptr >> j & 1;
            *outptr |= (tempc << (i % 8));
        }

    for(i=0; i< dismcount; i+= 16) {
        fprintf(debugout, "%04x: ", i);
        for(j=0;j<16;j++)
            fprintf(debugout, "%02x ", objbigendbuf[i+j]);
        fprintf(debugout, "\n");
    }
    fprintf(debugout, "\n");
}

// reverse bitcount bits starting at bitoffset in byte array ptr.
// uses another Bit Twiddling Hack from Sean Eron Anderson's site.
// (commented out as, so far, not needed)
void reversebits(uint8_t *ptr, uint16_t bitoffset, uint16_t bitcount) {
/*
    uint16_t u16;
    uint16_t s = sizeof(u16) * 8; // bit size; must be power of 2 
    uint16_t mask = ~0;
    
    memcpy(&u16, ptr + (bitoffset / 8), 2 * sizeof(uint8_t));
    u16 = bswap_16(u16);

    fprintf(stdout, "reversebits() : ptr to %04x, first bit = %d, count of %d bits\n", u16, bitoffset % 8, bitcount);
    fprintf(stdout, "reversebits() : ourbits = %0x\n", u16 >> (16 - (bitoffset % 8 + bitcount)) & (1<<bitcount)-1);
    u16 >>=(16 - (bitoffset % 8 + bitcount)) & (1<<bitcount)-1;

    while ((s >>= 1) > 0)  {
        mask ^= (mask << s);
        u16 = ((u16 >> s) & mask) | ((u16 << s) & ~mask);
    }
    fprintf(stdout, "reversebits() : result>>%d = %0x\n", 16 - (bitoffset % 8 + bitcount), u16 >> 16 - (bitoffset % 8 + bitcount));
*/
}

// (big-endian) reorder of the operation opcodes, operands, predicates and parameters in the byte array *instruction.
// (commented out, as not yet needed)
void insbitreorder(uint8_t *instruction, uint16_t formatbits) {
/*
    int i;

    for(i=0;i<5;i++) {
        if (operationsize(formatbits,i) > 0) {
            fprintf(stdout, "insoffset[%d] at offset +%d bytes",  i, 2 + (operationoffset(formatbits,i) / 8));
                if(extensionoffset(formatbits,i) > 0)
                    fprintf(stdout, ", %d byte ext at offset +%d bytes\n", 
                        (operationsize(formatbits, i)-24)/8, 2 + (extensionoffset(formatbits,i) / 8));
                else
                    fprintf(stdout, ", no ext.\n");
        }
        switch(operationsize(formatbits, i)) {
            case 0 :    return;
            case 40:    reversebits(instruction, extensionoffset(formatbits,i)+8, 8);       // ext bits [24..31]
            case 32:    reversebits(instruction, extensionoffset(formatbits,i), 8);         // ext bits [32..39]
            case 24:    reversebits(instruction, operationoffset(formatbits,i), 7);         // ins bits [0..6]
                        reversebits(instruction, operationoffset(formatbits,i)+7, 7);       // ins bits [7..13]
                        reversebits(instruction, operationoffset(formatbits,i)+14, 7);      // ins bits [14..20]
                        reversebits(instruction, operationoffset(formatbits,i)+21, 3);      // ins bits [21..23]
            default:    break;
        }
    }
*/
}

// reordermemimgbits() iterates through the instructions in a byte array for a count of
// bytecount, and re-writes the array with the opcodes, operands, parameters of the operations
// in big-endian bit order
// (commented out as not currently needed)
void reordermemimgbits(uint8_t *objbuf, uint64_t bytecount) {
/*
    uint16_t currentformatfield;
    uint16_t nextformatfield = bswap_16(BRTARGETFORMATBYTES);       // start with an uncompressed branch target 
                                                                    // instruction  (format bytes == 0xaa 0x02)
    uint16_t inslength;                                             // an uncompressed TM32 instruction is 224 bits long:
                                                                    //  16 + (3* 24) + 8 + (2* 24) + (5* 16);   
    uint64_t opint64 = 0;
    uint32_t offset = 0;
    uint8_t *instrptr = objbuf;
    uint8_t opsize = 0, currentinstruction[30];
    uint32_t i, insnum = 0;

    currentformatfield = nextformatfield;                           // set the first format field to an instruction of 5 x 42-bits
                                                                    // this is a branch target and has an uncompressed instruction

// -------------- main loop

    while( (void *)instrptr < (void *)(objbuf + bytecount)) {   // main loop - iterate through instructions. // n.b. incorporate count..

        inslength = instructionlength(currentformatfield);
        fprintf(stdout, "ins (%d bytes) before = ", inslength/8);
        for(i=0; i < inslength / 8; i++)
            fprintf(stdout, "%02x ", instrptr[i]);
        fprintf(stdout,"\n");
        memcpy(currentinstruction, instrptr, inslength / 8);
        insbitreorder(currentinstruction, currentformatfield);
        fprintf(stdout, "ins (%d bytes) after  = ", inslength/8);
        for(i=0; i < inslength / 8; i++)
            fprintf(stdout, "%02x ", instrptr[i]);
        fprintf(stdout,"\n\n");
        memcpy(&nextformatfield, currentinstruction, 2);            // format field for the next instruction

        instrptr+=inslength/8;
        currentformatfield = nextformatfield;
    }
*/
}   


