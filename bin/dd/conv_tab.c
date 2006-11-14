/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Keith Muller of the University of California, San Diego and Lance
 * Visser of Convex Computer Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)conv_tab.c	8.1 (Berkeley) 5/31/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

/*
 * There are currently six tables:
 *
 *	ebcdic		-> ascii	32V		conv=oldascii
 *	ascii		-> ebcdic	32V		conv=oldebcdic
 *	ascii		-> ibm ebcdic	32V		conv=oldibm
 *
 *	ebcdic		-> ascii	POSIX/S5	conv=ascii
 *	ascii		-> ebcdic	POSIX/S5	conv=ebcdic
 *	ascii		-> ibm ebcdic	POSIX/S5	conv=ibm
 *
 * Other tables are built from these if multiple conversions are being
 * done.
 *
 * Tables used for conversions to/from IBM and EBCDIC to support an extension
 * to POSIX P1003.2/D11. The tables referencing POSIX contain data extracted
 * from tables 4-3 and 4-4 in P1003.2/Draft 11.  The historic tables were
 * constructed by running against a file with all possible byte values.
 *
 * More information can be obtained in "Correspondences of 8-Bit and Hollerith
 * Codes for Computer Environments-A USASI Tutorial", Communications of the
 * ACM, Volume 11, Number 11, November 1968, pp. 783-789.
 */

u_char casetab[256];

/* EBCDIC to ASCII -- 32V compatible. */
const u_char e2a_32V[] = {
	0000, 0001, 0002, 0003, 0234, 0011, 0206, 0177,		/* 0000 */
	0227, 0215, 0216, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0235, 0205, 0010, 0207,		/* 0020 */
	0030, 0031, 0222, 0217, 0034, 0035, 0036, 0037,		/* 0030 */
	0200, 0201, 0202, 0203, 0204, 0012, 0027, 0033,		/* 0040 */
	0210, 0211, 0212, 0213, 0214, 0005, 0006, 0007,		/* 0050 */
	0220, 0221, 0026, 0223, 0224, 0225, 0226, 0004,		/* 0060 */
	0230, 0231, 0232, 0233, 0024, 0025, 0236, 0032,		/* 0070 */
	0040, 0240, 0241, 0242, 0243, 0244, 0245, 0246,		/* 0100 */
	0247, 0250, 0133, 0056, 0074, 0050, 0053, 0041,		/* 0110 */
	0046, 0251, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0120 */
	0260, 0261, 0135, 0044, 0052, 0051, 0073, 0136,		/* 0130 */
	0055, 0057, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0140 */
	0270, 0271, 0174, 0054, 0045, 0137, 0076, 0077,		/* 0150 */
	0272, 0273, 0274, 0275, 0276, 0277, 0300, 0301,		/* 0160 */
	0302, 0140, 0072, 0043, 0100, 0047, 0075, 0042,		/* 0170 */
	0303, 0141, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0200 */
	0150, 0151, 0304, 0305, 0306, 0307, 0310, 0311,		/* 0210 */
	0312, 0152, 0153, 0154, 0155, 0156, 0157, 0160,		/* 0220 */
	0161, 0162, 0313, 0314, 0315, 0316, 0317, 0320,		/* 0230 */
	0321, 0176, 0163, 0164, 0165, 0166, 0167, 0170,		/* 0240 */
	0171, 0172, 0322, 0323, 0324, 0325, 0326, 0327,		/* 0250 */
	0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,		/* 0260 */
	0340, 0341, 0342, 0343, 0344, 0345, 0346, 0347,		/* 0270 */
	0173, 0101, 0102, 0103, 0104, 0105, 0106, 0107,		/* 0300 */
	0110, 0111, 0350, 0351, 0352, 0353, 0354, 0355,		/* 0310 */
	0175, 0112, 0113, 0114, 0115, 0116, 0117, 0120,		/* 0320 */
	0121, 0122, 0356, 0357, 0360, 0361, 0362, 0363,		/* 0330 */
	0134, 0237, 0123, 0124, 0125, 0126, 0127, 0130,		/* 0340 */
	0131, 0132, 0364, 0365, 0366, 0367, 0370, 0371,		/* 0350 */
	0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,		/* 0360 */
	0070, 0071, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to EBCDIC -- 32V compatible. */
const u_char a2e_32V[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0117, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0112, 0340, 0132, 0137, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0152, 0320, 0241, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0232, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to IBM EBCDIC -- 32V compatible. */
const u_char a2ibm_32V[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0137, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0241, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0232, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* EBCDIC to ASCII -- POSIX and System V compatible. */
const u_char e2a_POSIX[] = {
	0000, 0001, 0002, 0003, 0234, 0011, 0206, 0177,		/* 0000 */
	0227, 0215, 0216, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0235, 0205, 0010, 0207,		/* 0020 */
	0030, 0031, 0222, 0217, 0034, 0035, 0036, 0037,		/* 0030 */
	0200, 0201, 0202, 0203, 0204, 0012, 0027, 0033,		/* 0040 */
	0210, 0211, 0212, 0213, 0214, 0005, 0006, 0007,		/* 0050 */
	0220, 0221, 0026, 0223, 0224, 0225, 0226, 0004,		/* 0060 */
	0230, 0231, 0232, 0233, 0024, 0025, 0236, 0032,		/* 0070 */
	0040, 0240, 0241, 0242, 0243, 0244, 0245, 0246,		/* 0100 */
	0247, 0250, 0325, 0056, 0074, 0050, 0053, 0174,		/* 0110 */
	0046, 0251, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0120 */
	0260, 0261, 0041, 0044, 0052, 0051, 0073, 0176,		/* 0130 */
	0055, 0057, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0140 */
	0270, 0271, 0313, 0054, 0045, 0137, 0076, 0077,		/* 0150 */
	0272, 0273, 0274, 0275, 0276, 0277, 0300, 0301,		/* 0160 */
	0302, 0140, 0072, 0043, 0100, 0047, 0075, 0042,		/* 0170 */
	0303, 0141, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0200 */
	0150, 0151, 0304, 0305, 0306, 0307, 0310, 0311,		/* 0210 */
	0312, 0152, 0153, 0154, 0155, 0156, 0157, 0160,		/* 0220 */
	0161, 0162, 0136, 0314, 0315, 0316, 0317, 0320,		/* 0230 */
	0321, 0345, 0163, 0164, 0165, 0166, 0167, 0170,		/* 0240 */
	0171, 0172, 0322, 0323, 0324, 0133, 0326, 0327,		/* 0250 */
	0330, 0331, 0332, 0333, 0334, 0335, 0336, 0337,		/* 0260 */
	0340, 0341, 0342, 0343, 0344, 0135, 0346, 0347,		/* 0270 */
	0173, 0101, 0102, 0103, 0104, 0105, 0106, 0107,		/* 0300 */
	0110, 0111, 0350, 0351, 0352, 0353, 0354, 0355,		/* 0310 */
	0175, 0112, 0113, 0114, 0115, 0116, 0117, 0120,		/* 0320 */
	0121, 0122, 0356, 0357, 0360, 0361, 0362, 0363,		/* 0330 */
	0134, 0237, 0123, 0124, 0125, 0126, 0127, 0130,		/* 0340 */
	0131, 0132, 0364, 0365, 0366, 0367, 0370, 0371,		/* 0350 */
	0060, 0061, 0062, 0063, 0064, 0065, 0066, 0067,		/* 0360 */
	0070, 0071, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to EBCDIC -- POSIX and System V compatible. */
const u_char a2e_POSIX[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0232, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0137, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0152, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0112, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0241, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};

/* ASCII to IBM EBCDIC -- POSIX and System V compatible. */
const u_char a2ibm_POSIX[] = {
	0000, 0001, 0002, 0003, 0067, 0055, 0056, 0057,		/* 0000 */
	0026, 0005, 0045, 0013, 0014, 0015, 0016, 0017,		/* 0010 */
	0020, 0021, 0022, 0023, 0074, 0075, 0062, 0046,		/* 0020 */
	0030, 0031, 0077, 0047, 0034, 0035, 0036, 0037,		/* 0030 */
	0100, 0132, 0177, 0173, 0133, 0154, 0120, 0175,		/* 0040 */
	0115, 0135, 0134, 0116, 0153, 0140, 0113, 0141,		/* 0050 */
	0360, 0361, 0362, 0363, 0364, 0365, 0366, 0367,		/* 0060 */
	0370, 0371, 0172, 0136, 0114, 0176, 0156, 0157,		/* 0070 */
	0174, 0301, 0302, 0303, 0304, 0305, 0306, 0307,		/* 0100 */
	0310, 0311, 0321, 0322, 0323, 0324, 0325, 0326,		/* 0110 */
	0327, 0330, 0331, 0342, 0343, 0344, 0345, 0346,		/* 0120 */
	0347, 0350, 0351, 0255, 0340, 0275, 0137, 0155,		/* 0130 */
	0171, 0201, 0202, 0203, 0204, 0205, 0206, 0207,		/* 0140 */
	0210, 0211, 0221, 0222, 0223, 0224, 0225, 0226,		/* 0150 */
	0227, 0230, 0231, 0242, 0243, 0244, 0245, 0246,		/* 0160 */
	0247, 0250, 0251, 0300, 0117, 0320, 0241, 0007,		/* 0170 */
	0040, 0041, 0042, 0043, 0044, 0025, 0006, 0027,		/* 0200 */
	0050, 0051, 0052, 0053, 0054, 0011, 0012, 0033,		/* 0210 */
	0060, 0061, 0032, 0063, 0064, 0065, 0066, 0010,		/* 0220 */
	0070, 0071, 0072, 0073, 0004, 0024, 0076, 0341,		/* 0230 */
	0101, 0102, 0103, 0104, 0105, 0106, 0107, 0110,		/* 0240 */
	0111, 0121, 0122, 0123, 0124, 0125, 0126, 0127,		/* 0250 */
	0130, 0131, 0142, 0143, 0144, 0145, 0146, 0147,		/* 0260 */
	0150, 0151, 0160, 0161, 0162, 0163, 0164, 0165,		/* 0270 */
	0166, 0167, 0170, 0200, 0212, 0213, 0214, 0215,		/* 0300 */
	0216, 0217, 0220, 0232, 0233, 0234, 0235, 0236,		/* 0310 */
	0237, 0240, 0252, 0253, 0254, 0255, 0256, 0257,		/* 0320 */
	0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,		/* 0330 */
	0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,		/* 0340 */
	0312, 0313, 0314, 0315, 0316, 0317, 0332, 0333,		/* 0350 */
	0334, 0335, 0336, 0337, 0352, 0353, 0354, 0355,		/* 0360 */
	0356, 0357, 0372, 0373, 0374, 0375, 0376, 0377,		/* 0370 */
};
