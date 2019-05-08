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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>
#include "tm32dis.h"
#include "tm32disinstrs.h"

static struct option longopts[] = {
    {"memimg",  no_argument, 0, 'm'},
    {"help",    no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"debug",   no_argument, 0, 'd'},
    {"count",   required_argument, 0, 'c'},
    {"adjust",  required_argument, 0, 'a'},
    {"input",   required_argument, 0, 'i'},
    {"skip",    required_argument, 0, 's'},
    {"format",  required_argument, 0, 'f'},
    {0, 0, 0, 0}
};

static char version_msg[] =    
    "tm32dis - a disassembler for the TriMedia TM3260 five issue-slot VLIW core\n" \
    "Version " TM32DISVERSION  " copyright (c) 2011  asbokid <ballymunboy@gmail.com>\n\n" \
    "This program comes with ABSOLUTELY NO WARRANTY; This is free software,\n" \
    "and you are welcome to redistribute it under certain conditions:\n" \
    "GNU GPL v3 License: http://www.gnu.org/licenses/gpl.html\n";

static char usage_msg[] = 
    "USAGE:\n" \
    " -h, --help             Displays this text\n" \
    " -v, --version          Version informaton\n" \
    " -f, --format <n>       Output format style <n>\n" \
    " -d, --debug            Debug output\n" \
    " -c, --count <n>        Disassemble <n> bytes\n" \
    " -a, --adjust <offset>  Adjust offset\n" \
    " -s, --skip <n>         Skip <n> bytes\n" \
    " -i, --input <filename> TM3260 object filename\n" \
    " -m, --memimg           Memory image (bootloader)\n\n" \
    "Example:  tm32dis -s 913 -c 64 -a 0x40000000 -m -i 2701_bootrom.bin\n\n";


// main()
//
int main(int argc, char **argv) {
    FILE *fin = NULL;
    extern FILE *debugout;
    uint8_t *inputfilename = NULL;
    void *objbuf = NULL, *objbigendbuf = NULL;
    uint64_t skipcount = 0, dismcount = 0, filelength = 0, offset = 0;
    uint8_t *instrptr;
    uint32_t memoryimage = FALSE, debug = FALSE, outputformat = 0;

    while (TRUE) {
        int32_t optidx = 0;
        int8_t c = getopt_long(argc,argv,"mhvda:f:i:c:s:", longopts, &optidx);
        if (c == -1)
            break;

        switch (c) {
            case 'm': memoryimage = TRUE;
                      break;
            case 'h': fprintf(stdout, "%s\n%s", version_msg, usage_msg);
                      return 0;
            case 'v': fprintf(stdout, "%s", version_msg);
                      return 0;
                      break;
            case 'd': debug = TRUE;
                      break;
            case 'f': outputformat = strtol(optarg, NULL, 0);
                      break;
            case 's': skipcount = strtol(optarg, NULL, 0);
                      break;
            case 'a': offset = strtol(optarg, NULL, 0);
                      break;
            case 'c': dismcount = strtol(optarg, NULL, 0);
                      break;
            case 'i': inputfilename = (char *) malloc(strlen(optarg)+1);
                      strcpy(inputfilename, optarg);
                      break;
            default : 
            case '?': fprintf(stdout, "%s", version_msg);
                      fprintf(stderr, "%s", usage_msg);
                      goto badexit;
        }
    }
  
    (debugout = (debug == TRUE) ? stdout : fopen("/dev/null","w"));
    fprintf(debugout, "Debug Enabled\n"); 

    if(!inputfilename) {
        fprintf(stderr, "%s\n%s", version_msg,usage_msg);
        goto badexit;
    }
    if(!(fin=fopen(inputfilename, "rb"))) {
        fprintf(stderr, "Could not open tm32 object file '%s'\n", inputfilename);
        goto badexit;
    }

    fseek(fin, 0L, SEEK_END);                               // find object file length
    filelength=ftell(fin);
    fseek(fin, 0L, SEEK_SET);
    if(!(objbuf=(uint8_t *) malloc(filelength * sizeof(uint8_t)))) {
        fprintf(stderr, "Could not malloc %" PRIx64 " bytes working space.\n", filelength);
        goto badexit;
    }

    if(fread(objbuf, 1, filelength, fin) != filelength) {
        fprintf(stderr, "Could not read from tm32 object file '%s'\n", inputfilename);
        goto badexit;
    }

    fprintf(stdout, "Read in %" PRId64 " (0x%" PRIx64 ") bytes from file '%s'\n", filelength, filelength, inputfilename);

    if(skipcount>filelength || dismcount>filelength-skipcount) {
        fprintf(stderr, "Count parameter too large for file length\n");
        goto badexit;
    }
                                                            // the count of bytes to disassemble
    (dismcount = (dismcount == 0) ? filelength-skipcount : dismcount);
  
    if(skipcount)
        fprintf(stdout, "Skipping %" PRId64 " (0x%" PRIx64 ") bytes\n", skipcount, skipcount);
    if(offset)
        fprintf(stdout, "Using 0x%" PRIx64 " adjustment offset\n", offset);
    fprintf(stdout, "Disassembling %" PRId64 " (0x%" PRIx64 ") bytes\n", dismcount, dismcount);

    instrptr = (uint8_t *) objbuf;                          // the pointer to our instruction stream buffer
    instrptr += skipcount;

    if(memoryimage) {
        fprintf(stdout, "Transposing memory image from bit-striped to sequential bytes\n");
        if(!(objbigendbuf=(uint8_t *) calloc(((dismcount / 32) + 1) * 32 * sizeof(uint8_t), 1))) {
            fprintf(stderr, "Could not malloc %" PRId64 " bytes working space in big-endian buffer\n", dismcount);
            goto badexit;
        }                                                   // transform bits into sequential byte order
        extractmemimginstructions(instrptr, objbigendbuf, ((dismcount / 32) + 1) * 32);
        instrptr = objbigendbuf;
    }

    tmdisassemble(outputformat, instrptr, dismcount, offset);
    return 0;

badexit:
    if(fin)
        fclose(fin);
    if(objbuf)
        free(objbuf);
    if(objbigendbuf)
        free(objbigendbuf);
    if(debugout)
        fclose(debugout);
    return -1;
}

