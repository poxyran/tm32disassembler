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

// op type aliases

enum OPPROP {
    BINARY_UNGUARDED_SHORT,
    UNARY_PARAM7_UNGUARDED_SHORT,
    BINARY_UNGUARDED_PARAM7_RESULTLESS_SHORT,
    UNARY_SHORT,
    BINARY_SHORT,
    UNARY_PARAM7_SHORT,
    BINARY_PARAM7_RESULTLESS_SHORT,
    BINARY_UNGUARDED,
    BINARY_RESULTLESS,
    UNARY_PARAM7_UNGUARDED,
    UNARY,
    BINARY_PARAM7_RESULTLESS,
    BINARY,
    UNARY_PARAM7,
    UNARY_PARAM7_RESULTLESS,    // not described. e.g. allocd, dcb, dinvalid, prefd
    ZEROARY,
    ZEROARY_RESULTLESS,         // not described. e.g. iclr (clear instruction cache) is (zeroary, resultless)
    UNARY_RESULTLESS,
    ZEROARY_PARAM32_UNGUARDED,
    ZEROARY_PARAM32_RESULTLESS,
    NOPROP
};

enum PARAMSIGN {
    UNSIGNED,
    SIGNED,
    NA
};



struct OPERATION {
    int32_t opcode;
    uint8_t *opname;
    enum OPPROP property;
    enum PARAMSIGN sign;
    uint8_t paramfactor;
};

const static struct OPERATION oplist[] = {
  { 0, "igtri", UNARY_PARAM7_SHORT, SIGNED, 1 },
  { 1,"igeqi", UNARY_PARAM7_SHORT, SIGNED, 1 },
  { 2,"ilesi", UNARY_PARAM7_SHORT, SIGNED, 1 },
  { 3,"ineqi", UNARY_PARAM7, SIGNED, 1 },
  { 4,"ieqli", UNARY_PARAM7_SHORT, SIGNED, 1 },
  { 5,"iaddi", UNARY_PARAM7_SHORT, UNSIGNED, 1 },
  { 6,"ild16d",UNARY_PARAM7_SHORT, SIGNED, 2 },
  { 7,"ld32d", UNARY_PARAM7_SHORT, SIGNED, 4 },
  { 8,"uld8d", UNARY_PARAM7_SHORT, SIGNED, 1 },
  { 9,"lsri",  UNARY_PARAM7_SHORT, UNSIGNED, 1 },
  { 10,"asri", UNARY_PARAM7_SHORT, UNSIGNED, 1 },
  { 11,"asli", UNARY_PARAM7_SHORT, UNSIGNED, 1 },
  { 12,"iadd", BINARY_SHORT, UNSIGNED, 1 },
  { 13,"isub", BINARY_SHORT, NA, 0 },
  { 14,"igeq", BINARY_SHORT, NA, 0 },
  { 15,"igtr", BINARY_SHORT, NA, 0 },
  { 16,"bitand", BINARY_SHORT, NA, 0 },
  { 17,"bitor",BINARY_SHORT, NA, 0 },
  { 18,"asr",  BINARY_SHORT, NA, 0 },
  { 19,"asl",  BINARY_SHORT, NA, 0 },
  { 20,"ifloat", UNARY_SHORT, NA, 0 },
  { 21,"ifixrz", UNARY_SHORT, NA, 0 },
  { 22,"fadd", BINARY_SHORT, NA, 0 },
  { 23,"imin", BINARY_SHORT, NA, 0 },
  { 24,"imax", BINARY_SHORT, NA, 0 },
  { 25,"iavgonep", BINARY_SHORT, NA, 0 },
  { 26,"ume8uu", BINARY_SHORT, NA, 0 },
  { 27,"imul", BINARY_SHORT, NA, 0 },
  { 28,"fmul", BINARY_SHORT, NA, 0 },
  { 29,"h_st8d", BINARY_PARAM7_RESULTLESS_SHORT, SIGNED, 1 },   // unguarded in 26-bit format, guarded in 34-bit
  { 30,"h_st16d", BINARY_PARAM7_RESULTLESS_SHORT, SIGNED, 2 },  //    ditto for many ops
  { 31,"h_st32d", BINARY_PARAM7_RESULTLESS_SHORT, SIGNED, 4 },
  { 32,"isubi", UNARY_PARAM7, UNSIGNED, 1 },
  { 33,"ugtr", BINARY, NA, 0 },
  { 34,"ugtri", UNARY_PARAM7, UNSIGNED, 1 },
  { 35,"ugeq", BINARY, NA, 0 },
  { 36,"ugeqi", UNARY_PARAM7, UNSIGNED, 1 },
  { 37,"ieql", BINARY, NA, 0 },
  { 38,"ueqli", UNARY_PARAM7, UNSIGNED, 1 },
  { 39,"ineq" , BINARY, NA, 0 },
  { 40,"uneqi", UNARY_PARAM7, UNSIGNED, 1 },
  { 41,"ulesi", UNARY_PARAM7, SIGNED, 1 },
  { 42,"ileqi", UNARY_PARAM7, SIGNED, 1 },
  { 43,"uleqi", UNARY_PARAM7, UNSIGNED, 1 },
  { 44,"h_iabs", BINARY, NA, 0 },
  { 45,"carry", BINARY, NA, 0 },
  { 46,"izero", BINARY, NA, 0 },
  { 47,"inonzero", BINARY, NA, 0 },
  { 48,"bitxor", BINARY, NA, 0 },
  { 49,"bitandinv", BINARY, NA, 0 },
  { 50,"bitinv", UNARY, NA, 0 },
  { 51,"sex16", UNARY, NA, 0 },
  { 52,"packbytes", BINARY , NA, 0 },
  { 53,"pack16lsb", BINARY, NA, 0 },
  { 54,"pack16msb", BINARY, NA, 0 },
  { 55,"ubytesel", BINARY, UNSIGNED, 0},
  { 56,"ibytesel", BINARY, UNSIGNED, 0},
  { 57,"mergelsb", BINARY, NA, 0 },
  { 58,"mergemsb", BINARY, NA, 0 },
  { 64,"ume8ii", BINARY, NA, 0 },
  { 65,"h_dspiabs", BINARY, NA, 0 },
  { 66,"dspiadd", BINARY, NA, 0 },
  { 67,"dspuadd", BINARY, NA, 0 },
  { 68,"dspisub", BINARY, NA, 0 },
  { 69,"dspusub", BINARY, NA, 0 },
  { 70,"dspidualadd", BINARY, NA, 0 },
  { 71,"dspidualsub", BINARY, NA, 0 },
  { 72,"h_dspidualabs", BINARY, NA, 0 },
  { 73,"quadavg", BINARY, NA, 0 },
  { 74,"iclipi", BINARY, NA, 0 },
  { 75,"uclipi", BINARY, NA, 0 },
  { 76,"uclipu", BINARY, NA, 0 },
  { 77,"iflip", BINARY, NA, 0 },
  { 78,"dspuquadaddui", BINARY, NA, 0 },
  { 80,"quadumin", BINARY, NA, 0 },
  { 81,"quadumax", BINARY, NA, 0 },
  { 82,"dualiclipi", BINARY, NA, 0 },
  { 83,"dualuclipi", BINARY, NA, 0 },
  { 89,"quadumulmsb", BINARY, NA, 0 },
  { 90,"ufir8uu", BINARY, NA, 0 },
  { 91,"ifir8ui", BINARY, NA, 0 },
  { 92,"ifir8ii", BINARY, NA, 0 },
  { 93,"ifir16", BINARY, NA, 0 },
  { 94,"ufir16", BINARY, NA, 0 },
  { 95,"dspidualmul", BINARY, NA, 0 },
  { 96,"lsr", BINARY, NA, 0 },
  { 97,"rol", BINARY, NA, 0 },
  { 98,"roli", UNARY_PARAM7, UNSIGNED, 1 },
  { 99,"funshift1", BINARY, NA, 0 },
  { 100,"funshift2", BINARY, NA, 0 },
  { 101,"funshift3", BINARY, NA, 0 },
  { 102,"dualasr", BINARY, NA, 0 },
  { 103,"mergedual16lsb", BINARY, NA, 0 },
  { 108,"fdiv", BINARY, NA, 0 },
  { 109,"fdivflags", BINARY, NA, 0 },                       // errata: Appendix A Instruction Set TM3260 Rev 1.02 12 July 2004??
  { 110,"fsqrt", UNARY, NA, 0 },
  { 111,"fsqrtflags", UNARY, NA, 0 },
  { 112,"faddflags", BINARY, NA, 0 },
  { 113,"fsub", BINARY, NA, 0 },
  { 114,"fsubflags", BINARY, NA, 0 },
  { 115,"fabsval", UNARY, NA, 0 },
  { 116,"fabsvalflags", UNARY, NA, 0 },
  { 117,"ifloatrz", UNARY, NA, 0 },
  { 118,"ifloatrzflags", UNARY, NA, 0 },
  { 119,"ufloatrz", UNARY, NA, 0 },
  { 120,"ufloatrzflags", UNARY, NA, 0 },
  { 121,"ifixieee", UNARY, NA, 0 },
  { 122,"ifixieeeflags", UNARY, NA, 0 },
  { 123,"ufixieee", UNARY, NA, 0 },
  { 124,"ufixieeeflags", UNARY, NA, 0 },
  { 125,"ufixrz", UNARY, NA, 0 },
  { 126,"ufixrzflags", UNARY, NA, 0 },
  { 127,"ufloat", UNARY, NA, 0 },
  { 128,"ufloatflags", UNARY, NA, 0 },
  { 129,"ifixrzflags", UNARY, NA, 0 },
  { 130,"ifloatflags", UNARY, NA, 0 },
  { 138,"umul", BINARY, NA, 0 },
  { 139,"imulm", BINARY, NA, 0 },
  { 140,"umulm", BINARY, NA, 0 },
  { 141,"dspimul", BINARY, NA, 0 },
  { 142,"dspumul", BINARY, NA, 0 },
  { 143,"fmulflags", BINARY, NA, 0 },
  { 144,"fgtr", BINARY, NA, 0 },
  { 145,"fgtrflags", BINARY, NA, 0 },
  { 146,"fgeq", BINARY, NA, 0 },
  { 147,"fgeqflags", BINARY, NA, 0 },
  { 148,"feql", BINARY, NA, 0 },
  { 149,"feqlflags", BINARY, NA, 0 },
  { 150,"fneq", BINARY, NA, 0 },
  { 151,"fneqflags", BINARY, NA, 0 },
  { 152,"fsign", UNARY, NA, 0 },
  { 153,"fsignflags", UNARY, NA, 0 },
  { 154,"cycles", ZEROARY, NA, 0 },
  { 155,"hicycles", ZEROARY, NA, 0 },
  { 156,"readdpc", ZEROARY, NA, 0 },
  { 157,"readspc", ZEROARY, NA, 0 },
  { 158,"readpcsw", ZEROARY, NA, 0 },
  { 159,"writespc", UNARY_RESULTLESS, NA, 0 },
  { 160,"writedpc", UNARY_RESULTLESS, NA, 0 },
  { 161,"writepcsw", BINARY_RESULTLESS, NA, 0 },            // errata: writepcsw is binary (not unary): App. A TM3260 Ins Set Rev. 1.02
  { 162,"curcycles", ZEROARY, NA, 0 },
  { 176,"jmpt", BINARY_RESULTLESS, NA, 0 },
  { 177,"ijmpt", BINARY_RESULTLESS, NA, 0 },
  { 178,"jmpi", ZEROARY_PARAM32_RESULTLESS, UNSIGNED, 1 },
  { 179,"ijmpi", ZEROARY_PARAM32_RESULTLESS, UNSIGNED, 1 },
  { 180,"jmpf", BINARY, NA, 0 },
  { 181,"ijmpf", BINARY_RESULTLESS, NA, 0 },
  { 184,"iclr", ZEROARY_RESULTLESS, NA, 0 },
  { 191,"uimm", ZEROARY_PARAM32_UNGUARDED, UNSIGNED, 1 },   // errata: iimm (pseudo-op) shares opcode App. A TM3260 Ins Set Rev. 1.02
//  { 191,"iimm", ZEROARY_PARAM32_UNGUARDED, SIGNED, 1 },   // errata: uimm shares opcode App. A TM3260 Ins Set Rev. 1.02
  { 192,"ild8d", UNARY_PARAM7, SIGNED, 1 },
  { 193,"ild8r", BINARY, NA, 0 },
  { 194,"uld8r", BINARY, NA, 0 },
  { 195,"ild16r", BINARY, NA, 0 },
  { 196,"ild16x", BINARY, NA, 0 },
  { 197,"uld16d", UNARY_PARAM7, SIGNED, 2 },
  { 198,"uld16r", BINARY, NA, 0 },
  { 199,"uld16x", BINARY, NA, 0 },
  { 200,"ld32r", BINARY, NA, 0 },
  { 201,"ld32x", BINARY, NA, 0 },
  { 202,"rdtag", UNARY_PARAM7, SIGNED, 4 },
  { 203,"rdstatus", UNARY_PARAM7, UNSIGNED, 4 },
  { 205,"dcb", UNARY_PARAM7_RESULTLESS, SIGNED, 4 },
  { 206,"dinvalid", UNARY_PARAM7_RESULTLESS, SIGNED, 4 },
  { 209,"prefd", UNARY_PARAM7_RESULTLESS, SIGNED, 4 },
  { 210,"prefr", BINARY, NA, 0 },
  { 211,"pref16x", BINARY, NA, 0 },
  { 212,"pref32x", BINARY, NA, 0 },
  { 213,"allocd", UNARY_PARAM7_RESULTLESS, SIGNED, 4 },
  { 214,"allocr", BINARY_RESULTLESS, NA, 0 },
  { 215,"allocx", BINARY_RESULTLESS, NA, 0 },
  { 233,"swapbytes", UNARY, NA, 0 },
  { 234,"dspuquadabssubi", BINARY, NA, 0 },
  { 235,"quadsub", BINARY, NA, 0 },
  { 236,"quadadd", BINARY, NA, 0 },
  { 237,"mergeodd", BINARY, NA, 0 },
  { 238,"dualimulm", BINARY, NA, 0 },
  { 239,"dualasl", BINARY, NA, 0 },
  { 240,"dspuquadsub", BINARY, NA, 0 },
  { 241,"dspuquadadd", BINARY, NA, 0 },
  { 242,"dspiquadsub", BINARY, NA, 0 },
  { 243,"dspiquadadd", BINARY, NA, 0 },
  { 244,"addsub", BINARY, NA, 0 },
  { 245,"quaduleq", BINARY, NA, 0 },
  { 246,"quaduclipi", BINARY, NA, 0 },
  { 247,"quadiclipi", BINARY, NA, 0 },
  { 248,"quaduminbyte", UNARY, NA, 0 },
  { 249,"quadumaxbyte", UNARY, NA, 0 },
  { 250,"dspuquadabssub", BINARY, NA, 0 },
  { 251,"bilinear2", BINARY},
  { 252,"bilinear1", BINARY, NA, 0 },
  { 253,"quadavg0", BINARY, NA, 0 },
  { 254,"uclip8iasr8add", BINARY, NA, 0 },
  { 255,"nop", ZEROARY_RESULTLESS, NA, 0 },
  {  -1, "", NOPROP, NA, 0 },
};

// pseudo-ops which alias for an op with same opcode
//
// 11,"lsli"
// 12,"ident"
// 13,"ineg"
// 144,"fles"
// 145,"flesflags"
// 146,"fleq"
// 147,"fleqflags"
// 14,"ileq"
// 15,"iles"
// 192,"ild8"
// 197,"uld16"
// 19,"lsl"
// 209,"pref"
// 213,"alloc"
// 29,"st8"
// 29,"st8d"
// 30,"st16"
// 30,"st16d"
// 31,"st32"
// 31,"st32d"
// 33,"borrow"
// 33,"ules"
// 35,"uleq"
// 37,"ueql"
// 39,"uneq"
// 44,"iabs"
// 53,"zex16"
// 55,"zex8"
// 56,"sex8"
// 65,"dspiabs"
// 6,"ild16"
// 72,"dspidualabs"
// 76,"umin"
// 7,"ld32"
// 8,"uld8"

