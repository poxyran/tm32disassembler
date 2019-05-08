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


// operationsize() is passed the two header bytes from the previous TM32 instruction. These 
// two bytes encode the compressed size of the next TM32 instruction in the stream
//
// operationsize() returns the compressed size (in bits) of operation n that will be executed
// in issue-slot n of the TriMedia
//
// (uint16_t) formatbits is little-endian, so the (2-bit) issue slot encodings for each of the
// five slots, are to be found in the following bits
//
//                      1 1 1 1 1 1
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5  == bit
//----------------------------------
//  x x x x x x 5-5 4-4 3-3 2-2 1-1  == slot
//
// For example, the compression format for issue-slot 2 is found in bits 13 and 12
// a little-endian (x86) host will swap the two format bytes from their big-endian code storage.

uint8_t operationsize(uint16_t formatbits, uint8_t slotnumber ) {
    uint8_t retval;

    switch(formatbits >> (2 * slotnumber) & 3) { 
        case 0    : retval = 24;    // 26-bit operation (remember to add the two bits from
                    break;          //                  the two byte format field)
        case 1    : retval = 32;    // 34-bit operation
                    break;
        case 2    : retval = 40;    // 42-bit operation
                    break;
        case 3    : retval = 0;     // 0-bit operation (NOP)
                    break;
        default   : fprintf(stderr, "encoding error  (* -1 bits *)\n");
                    retval = 0xff;
        }
//      fprintf(stdout, "Format bits: %04x:  Slot %d has opsize of %d bits\n", formatbits, slotnumber, retval);
        return retval;
}

// instructionlength() is passed a 16 bit (two byte) formatfield in which the operation sizes 
// for each of the five issue-slots in the instruction is encoded.
//
// instructionlength() returns the total size of the instruction in bits.
// For the TM32, the length is calculated as 16 bits (for the formatfield) plus 24 bits for each
// operation, plus the one or two extension bytes for each operation,  and if there are more than
// 3 operations in an instruction, plus one further byte needed for the format field of the
// second group of instructions.

uint16_t instructionlength(uint16_t formatbits) {
    uint32_t i;
    uint16_t len = 0, instrcount = 0;

    len = 16;           // add the two bytes for the 1st format field
    for(i=0;i<5;i++)
    switch(formatbits >> (2 * i) & 3) { 
        case 0  :   len += 24;  // 26-bit operation
                    instrcount++;
                    break;
        case 1  :   len += 32;  // 34-bit
                    instrcount++;
                    break;
        case 2  :   len += 40;  // 42-bit operation
                    instrcount++;
                    break;
        case 3  :   break;      // 0-bit operation (NOP)
        default   : fprintf(stderr, "encoding error  (* -1 bits *)\n");
                    return -1;
        }
    if(instrcount >3)
        len += 8;       // add eight bits for the format field of the 2nd group 
    return len;
}

// formatfieldstring() is passed a two byte format field.
// It returns a string of binary digits that represent the bit-encoded lengths
// of each of the five operations for that instruction.

uint8_t *formatfieldstring(uint16_t formatbits) {
    uint32_t i;
    static uint8_t formatstr[30];
    uint8_t *str=formatstr;

    for(i=0;i<5;i++) {
        switch(formatbits >> (2 * i) & 3) { 
            case 0    : *str++ = '0';       // 26-bit
                        *str++ = '0';
                        break;
            case 1    : *str++ = '1';       // 34-bit
                        *str++ = '0';
                        break;
            case 2    : *str++ = '0';       // 42-bit
                        *str++ = '1';
                        break;
            case 3    : *str++ = '1';       // 0-bit operation (NOP)
                        *str++ = '1';
                        break;
            default   : return ("encoding error  (* -1 bits *)");
            }
        *str++ = ' ';
    }
    *str++ = '\0';
    return formatstr;
}


// opintstr() returns a hex string representing the opint64, formatted according to its bit length.
uint8_t *opintstr(uint64_t opint64, uint8_t opsize) {
    static uint8_t opint64str[24];

    switch (opsize) {
        case 0  : sprintf(opint64str, "\0");
                  break;
        case 24 : sprintf(opint64str, " %01x %02x %02x %02x", (uint8_t) (opint64>>24 & 0x03),
                    (uint8_t)(opint64>>16 & 0xff), (uint8_t)(opint64>>8 & 0xff), (uint8_t)(opint64 & 0xff));
                  break;
        case 32 : sprintf(opint64str, " %01x %02x %02x %02x %02x",
                    (uint8_t)(opint64>>32 & 0x03), (uint8_t)(opint64>>24 & 0xff), (uint8_t)(opint64>>16 & 0xff),
                    (uint8_t)(opint64>>8 & 0xff), (uint8_t)(opint64 & 0xff));
                  break;
        case 40 : sprintf(opint64str, " %01x %02x %02x %02x %02x %02x",
                    (uint8_t)(opint64>>40 & 0x03), (uint8_t)(opint64>>32 & 0xff), (uint8_t)(opint64>>24 & 0xff), 
                    (uint8_t)(opint64>>16 & 0xff), (uint8_t)(opint64>>8 & 0xff), (uint8_t)(opint64 & 0xff));
                  break;
        default : sprintf(opint64str, "opintstr() error");
    }
    return opint64str;
}

// getrealopindex() returns the operation index for the operation in slot n.
// For example, if an instruction has a NOP in slot #0, NOP in slot #1 but
// a Real Op in slot #3 and a Real op in slot #4, then...
// the index of the Op in slot #3 will 0 and the index of the Op in slot #4 will be 1.

uint8_t getrealopindex(uint16_t formatbits, uint8_t slotnumber) {
    uint8_t realopindex = 0;
    uint32_t i;
    
    for(i=0;i<slotnumber;i++)
        if(operationsize(formatbits, i) > 0)
            realopindex++;
    return realopindex;
}

// opcodebits2524tostring() converts bits 0 and 1 in the byte opcodebits2524 to an ASCII string
// of binary digits, and returns that string in str.

int32_t opcodebits2524tostring(uint8_t opcodebits2524, uint8_t *str) {
    *str = '\0';
    switch (opcodebits2524 & 0x03) {
        case 0 : strcpy(str, "00");
                 break;
        case 1 : strcpy(str, "10");
                 break;
        case 2 : strcpy(str, "01");
                 break;
        case 3 : strcpy(str, "11");
                 break;
        default: sprintf(str, "error opcodebits2524 : %02x\n", opcodebits2524 & 0x03);
                 return -1;
    }
    return 0;
}

// operationoffset() calculates the bit offset from the start of the instruction
// for the 24-bit part of slot operation n 

uint16_t operationoffset(uint16_t formatbits, uint8_t slotnumber) {
    uint16_t i, offset = 0, oplen = 0, opcount = 0;
    
    if (operationsize(formatbits, slotnumber) == 0) // a NOP in slotnumber, so operation size for that is zero
        return 0;

    for(i=0;i<slotnumber;i++) {
        oplen = operationsize(formatbits, i);
        if(oplen>0) {
            opcount++;
            offset+=24;
        }
    }
    if(opcount > 2)
        offset+=8;          // add the formatbits byte for the 2nd operations group, if used

    return offset;
}

// extensionoffset() calculates the bit offset from the start of the instruction for
// the 1 or 2 byte extension to slot operation n 
// returns zero if the operation uses no extension bytes

uint16_t extensionoffset(uint16_t formatbits, uint8_t slotnumber) {
    uint32_t i;
    uint16_t extbegin = 0, extoffset = 0,oplen = 0, opcount = 0;

    if(operationsize(formatbits,slotnumber) <= 24)      // operation has no extension
        return 0;

    for(i=0;i<MAXSLOT;i++) {                    // count the number of non-NOP operations in the instruction.
        oplen = operationsize(formatbits, i);   // (count * 24) + format bytes is a pointer to the extension bytes
        if(oplen>0)
            opcount++;
    }
    extbegin+=(opcount*24);

    if(opcount >3)                              // 
        extbegin+=8;                            // add format byte to ext. offset for 2nd operations group, if used

    extoffset = extbegin;

    for(i=0;i<slotnumber;i++) {
        oplen = operationsize(formatbits, i);
        if(oplen>0) 
            extoffset+= oplen - 24;
    }
    return extoffset;
}

// decodeop() iterates the operations list for an opcode and returns the corresponding operation structure
const struct OPERATION *decodeop(uint32_t opcode) {

    uint32_t i=0;
    int32_t code=0;

    while((code = oplist[i].opcode) >= 0) {
        if(code == opcode)
            return(&oplist[i]);
        i++;
    }
    return &oplist[i];      // opcode not found, so return a null operation struct
}

// sign extending from a constant 7-bit width
// taken from Sean Eron Anderson's Bit Twiddling Hacks.
int32_t signextend(uint8_t x) {
struct {int32_t x:7;} s;
    return (s.x = x);
}

