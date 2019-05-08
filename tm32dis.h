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

#define TM32DISVERSION "1.24"

#define TRUE 1
#define FALSE 0

#define MAXTM32INSLEN   224
#define MAXSLOT         5

#define BITMASK6_0      0x7f                            //   src1[6:0]   | param[13:7]
#define BITMASK13_7     0x7f << 7                       //   src2[6:0]   | param[6:0]   | dst[6:0]
#define BITMASK20_14    0x7f << 14                      //   dst[6:0]    | param[6:0]   | guard[6:0] | dst[6:0]
#define BITMASK23_21    7 << 21                         //   opcode[2:0] | param[16:14]
#define BITMASK25_24    3 << 24                         //   opcode[4:3] | param[18:17]
#define BITMASK28_26    7 << 26                         //   opcode[7:5]
#define BITMASK28_21    0xff << 21                      //   opcode[7:0]
#define BITMASK30_21    0x3ff << 21                     //   param[23:14]
#define BITMASK30_26    0x1f << 26                      //   param[23:19]
#define BITMASK32_26    0x7f << 26                      //   dst[6:0]    | param[6:0]
#define BITMASK33_26    0x1f << 26                      //   param[23:19]
#define BITMASK41_34    0xff << 34                      //   param[31:24]
#define BITMASK41_35    0x7f << 35                      //   param[6:0]  | dst[6:0]
#define BITMASK41_26    0xffff << 26

#define OPBITS6_0(x)    (uint32_t)(x & 0x7f)            //   src1[6:0]   | param[13:7]
#define OPBITS13_7(x)   (uint32_t)((x >> 7) & 0x7f)     //   src2[6:0]   | param[6:0]   | dst[6:0]
#define OPBITS20_14(x)  (uint32_t)((x >> 14) & 0x7f)    //   dst[6:0]    | param[6:0]   | guard[6:0] | dst[6:0]
#define OPBITS23_21(x)  (uint32_t)((x >> 21) & 7)       //   opcode[2:0] | param[16:14]
#define OPBITS25_24(x)  (uint32_t)((x >> 24) & 3)       //   opcode[4:3] | param[18:17]
#define OPBITS25_21(x)  (uint32_t)((x >> 21) & 0x1f)    //   opcode[4:0]
#define OPBITS28_26(x)  (uint32_t)((x >> 26) & 7)       //   opcode[7:5]
#define OPBITS28_21(x)  (uint32_t)((x >> 21) & 0xff)    //   opcode[7:0]
#define OPBITS30_21(x)  (uint32_t)((x >> 21) & 0x3ff)   //   param[23:14]
#define OPBITS30_26(x)  (uint32_t)((x >> 26) & 0x1f)    //   param[23:19]
#define OPBITS32_26(x)  (uint32_t)((x >> 26) & 0x7f)    //   dst[6:0]    | param[6:0]
#define OPBITS33_26(x)  (uint32_t)((x >> 26) & 0x1f)    //   param[23:19]
#define OPBITS41_34(x)  (uint32_t)((x >> 34) & 0xff)    //   param[31:24]
#define OPBITS41_35(x)  (uint32_t)((x >> 35) & 0x7f)    //   param[6:0]  | dst[6:0]
#define OPBITS41_26(x)  (uint32_t)((x >> 26) & 0xffff)  //   both extension bytes

#define OPBITS33(x)     (uint32_t)((x >> 33) & 1)       //   used to encode type for 42-bit operations
#define OPBITS32(x)     (uint32_t)((x >> 32) & 1)       //
#define OPBITS31(x)     (uint32_t)((x >> 31) & 1)       //
#define OPBITS32_31(x)  (uint32_t)((x >> 31) & 3)       //
#define OPBITS29(x)     (uint32_t)((x >> 29) & 1)       //   sign flag for 7-bit parameteric operations

#define PARAM32BITS(x)  (uint32_t)(((x>>7) & 0x7f) | ((x<<7) & 0x7f<<7) | ((x>>7) & 0x3ff<<14) | ((x>>10) & 0xff<<24))

/*
#define PARAM32BITS1(x) (uint32_t)(((x>>7) & 0x7f))     //   param32[13:7]
#define PARAM32BITS2(x) (uint32_t)(((x<<7) & 0x7f<<7))  //   param32[6:0]
#define PARAM32BITS3(x) (uint32_t)(((x>>7)& 0x3ff<<14)) //   param32[23:14]
#define PARAM32BITS4(x) (uint32_t)(((x>>10) & 0xff<<24))//   param32[31:24]
*/

#define BRTARGETFORMATBYTES 0xaa02                      //   these format bytes are always used to encode a
                                                        //   Branch Target Instruction -  they indicate the
                                                        //   beginning of a Decision Tree. The 'dtree' is
                                                        //   the scheduling unit for a Trimedia TM32 VLIW core.
FILE *debugout;

uint8_t operationsize(uint16_t formatbits, uint8_t slotnumber );
uint16_t instructionlength(uint16_t formatbits);
uint8_t *formatfieldstring(uint16_t formatbits);
uint8_t *opintstr(uint64_t opint64, uint8_t opsize);
uint8_t getrealopindex(uint16_t formatbits, uint8_t slotnumber);
int32_t opcodebits2524tostring(uint8_t opcodebits2524, uint8_t *str);
uint16_t operationoffset(uint16_t formatbits, uint8_t slotnumber);
uint16_t extensionoffset(uint16_t formatbits, uint8_t slotnumber);
const struct OPERATION *decodeop(uint32_t opcode);
int32_t signextend(uint8_t x);
uint64_t unpackoperation(uint8_t *instruction, uint16_t formatbits, uint32_t slotnumber);
uint64_t decodeoperation(uint32_t opsize, uint64_t opint64, uint8_t *opstring);
void insbitreorder(uint8_t *instruction, uint16_t formatbits);
void reversebits(uint8_t *ptr, uint16_t bitoffset, uint16_t bitcount);
int extractmemimginstructions(uint8_t *objbuf, uint8_t *objbigendbuf, uint32_t dismcount);
void reordermemimgbits(uint8_t *objbuf, uint64_t bytecount);
void tmdisassemble(uint32_t printoutformat, uint8_t *objbuf, uint64_t bytecount, uint64_t offset);
