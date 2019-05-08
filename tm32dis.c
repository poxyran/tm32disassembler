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


// tmdisassemble() iterates through a byte array for a count of bytecount,
// and disassembles the TM32 instruction stream, using an arbitrary offset
void tmdisassemble(uint32_t printoutformat, uint8_t *objbuf, uint64_t bytecount, uint64_t offset) {

    uint16_t currentformatfield;
    uint16_t nextformatfield = bswap_16(BRTARGETFORMATBYTES);       // start with an uncompressed branch target 
                                                                    // instruction  (format bytes == 0xaa 0x02)
    uint16_t inslength;                                             // an uncompressed TM32 instruction is 224 bits long:
                                                                    //  16 + (3* 24) + 8 + (2* 24) + (5* 16);   
    uint64_t opint64 = 0;                                           // a 64 bit integer to hold the current operation (or "syllable")
    uint8_t *instrptr = objbuf;
    static uint8_t formatbitstr[20], operationstring[50], currentinstruction[30], opsize = 0;
    uint32_t i, insnum = 0;

    currentformatfield = nextformatfield;                           // set the first format field to an instruction of 5 x 42-bits
                                                                    // this is a branch target and has an uncompressed instruction
    fprintf(stdout, "\ndisassembly\n");

// -------------- main loop - iterate through instructions

    while( (void *) instrptr < (void *) objbuf + bytecount) {

        inslength = instructionlength(currentformatfield);
        memcpy(currentinstruction, instrptr, inslength / 8);
    
        if(inslength == MAXTM32INSLEN) {                            // start of a new decision tree ... 
            fprintf(stdout, "\n");                                  // it would be better here to check whether all the 
            insnum = 0;                                             // operations are uncompressed - implying branch-in point
        }
        else
            insnum++;

        memcpy(&nextformatfield, currentinstruction, 2);            // format field for the next instruction

        switch(printoutformat) {
            case 1:
                fprintf(stdout, "(* 0x%08" PRIx64 " *) ", offset);   
                instrptr += inslength/8;
                for(i=0;i<5;i++) {                                      
                    opint64 = (uint64_t) unpackoperation(currentinstruction, currentformatfield, i);
                    opsize = operationsize(currentformatfield, i);
                    decodeoperation(opsize, opint64, operationstring);
                    strcat(operationstring, (i == 4) ? ";" : ",");
                    fprintf(stdout, "   %-36s", operationstring);
                }
                fprintf(stdout, "\n");
                break; 
            case 0:
            default:
                fprintf(stdout, "(* instruction %-3d : %d bits (%d bytes) long *)\n", insnum, inslength, inslength / 8);
                fprintf(stdout, "(* offset          : 0x%08" PRIx64 " *)\n", offset);
                fprintf(stdout, "(* bytes           : ");
                for(i=0;i<(inslength/8);i++) 
                    fprintf(stdout,"%02x ", (uint8_t) *instrptr++);
                fprintf(stdout, "*)\n");
    
                fprintf(stdout, "(* format bytes    : 0x%02x%02x & 0xff03 = ",  (uint8_t)(bswap_16(nextformatfield) >> 8) & 0xff,
                                                                                (uint8_t)bswap_16(nextformatfield) & 0xff);
                fprintf(stdout, "0x%04x, ", bswap_16(nextformatfield) & 0xff03);
                fprintf(stdout, "format in little endian bit order: %s *)\n", formatfieldstring(nextformatfield));

                                                                    // print each of the five ops in an instruction to stdout
                for(i=0;i<5;i++) {                                      
                    opint64 = (uint64_t) unpackoperation(currentinstruction, currentformatfield, i);
                    opsize = operationsize(currentformatfield, i);
                    decodeoperation(opsize, opint64, operationstring);
                    strcat(operationstring, (i == 4) ? ";" : ",");
                    fprintf(stdout, "   %-33s", operationstring);
                    fprintf(stdout, "           (* %2d bits:%s *)\n", (opsize == 0 ? 0 : opsize+2), opintstr(opint64,opsize));
                }
                fprintf(stdout, "\n");
            }

        offset += inslength / 8 ;
        currentformatfield = nextformatfield;
    }
    fprintf(stdout,"\nend disassembly\n");
}   
