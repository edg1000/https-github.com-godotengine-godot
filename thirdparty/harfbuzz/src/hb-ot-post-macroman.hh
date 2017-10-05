/*
 * Copyright © 2017  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_OT_POST_MACROMAN_HH
#if 0 /* Make checks happy. */
#define HB_OT_POST_MACROMAN_HH
#include "hb-private.hh"
#endif


_S(".notdef")
_S(".null")
_S("nonmarkingreturn")
_S("space")
_S("exclam")
_S("quotedbl")
_S("numbersign")
_S("dollar")
_S("percent")
_S("ampersand")
_S("quotesingle")
_S("parenleft")
_S("parenright")
_S("asterisk")
_S("plus")
_S("comma")
_S("hyphen")
_S("period")
_S("slash")
_S("zero")
_S("one")
_S("two")
_S("three")
_S("four")
_S("five")
_S("six")
_S("seven")
_S("eight")
_S("nine")
_S("colon")
_S("semicolon")
_S("less")
_S("equal")
_S("greater")
_S("question")
_S("at")
_S("A")
_S("B")
_S("C")
_S("D")
_S("E")
_S("F")
_S("G")
_S("H")
_S("I")
_S("J")
_S("K")
_S("L")
_S("M")
_S("N")
_S("O")
_S("P")
_S("Q")
_S("R")
_S("S")
_S("T")
_S("U")
_S("V")
_S("W")
_S("X")
_S("Y")
_S("Z")
_S("bracketleft")
_S("backslash")
_S("bracketright")
_S("asciicircum")
_S("underscore")
_S("grave")
_S("a")
_S("b")
_S("c")
_S("d")
_S("e")
_S("f")
_S("g")
_S("h")
_S("i")
_S("j")
_S("k")
_S("l")
_S("m")
_S("n")
_S("o")
_S("p")
_S("q")
_S("r")
_S("s")
_S("t")
_S("u")
_S("v")
_S("w")
_S("x")
_S("y")
_S("z")
_S("braceleft")
_S("bar")
_S("braceright")
_S("asciitilde")
_S("Adieresis")
_S("Aring")
_S("Ccedilla")
_S("Eacute")
_S("Ntilde")
_S("Odieresis")
_S("Udieresis")
_S("aacute")
_S("agrave")
_S("acircumflex")
_S("adieresis")
_S("atilde")
_S("aring")
_S("ccedilla")
_S("eacute")
_S("egrave")
_S("ecircumflex")
_S("edieresis")
_S("iacute")
_S("igrave")
_S("icircumflex")
_S("idieresis")
_S("ntilde")
_S("oacute")
_S("ograve")
_S("ocircumflex")
_S("odieresis")
_S("otilde")
_S("uacute")
_S("ugrave")
_S("ucircumflex")
_S("udieresis")
_S("dagger")
_S("degree")
_S("cent")
_S("sterling")
_S("section")
_S("bullet")
_S("paragraph")
_S("germandbls")
_S("registered")
_S("copyright")
_S("trademark")
_S("acute")
_S("dieresis")
_S("notequal")
_S("AE")
_S("Oslash")
_S("infinity")
_S("plusminus")
_S("lessequal")
_S("greaterequal")
_S("yen")
_S("mu")
_S("partialdiff")
_S("summation")
_S("product")
_S("pi")
_S("integral")
_S("ordfeminine")
_S("ordmasculine")
_S("Omega")
_S("ae")
_S("oslash")
_S("questiondown")
_S("exclamdown")
_S("logicalnot")
_S("radical")
_S("florin")
_S("approxequal")
_S("Delta")
_S("guillemotleft")
_S("guillemotright")
_S("ellipsis")
_S("nonbreakingspace")
_S("Agrave")
_S("Atilde")
_S("Otilde")
_S("OE")
_S("oe")
_S("endash")
_S("emdash")
_S("quotedblleft")
_S("quotedblright")
_S("quoteleft")
_S("quoteright")
_S("divide")
_S("lozenge")
_S("ydieresis")
_S("Ydieresis")
_S("fraction")
_S("currency")
_S("guilsinglleft")
_S("guilsinglright")
_S("fi")
_S("fl")
_S("daggerdbl")
_S("periodcentered")
_S("quotesinglbase")
_S("quotedblbase")
_S("perthousand")
_S("Acircumflex")
_S("Ecircumflex")
_S("Aacute")
_S("Edieresis")
_S("Egrave")
_S("Iacute")
_S("Icircumflex")
_S("Idieresis")
_S("Igrave")
_S("Oacute")
_S("Ocircumflex")
_S("apple")
_S("Ograve")
_S("Uacute")
_S("Ucircumflex")
_S("Ugrave")
_S("dotlessi")
_S("circumflex")
_S("tilde")
_S("macron")
_S("breve")
_S("dotaccent")
_S("ring")
_S("cedilla")
_S("hungarumlaut")
_S("ogonek")
_S("caron")
_S("Lslash")
_S("lslash")
_S("Scaron")
_S("scaron")
_S("Zcaron")
_S("zcaron")
_S("brokenbar")
_S("Eth")
_S("eth")
_S("Yacute")
_S("yacute")
_S("Thorn")
_S("thorn")
_S("minus")
_S("multiply")
_S("onesuperior")
_S("twosuperior")
_S("threesuperior")
_S("onehalf")
_S("onequarter")
_S("threequarters")
_S("franc")
_S("Gbreve")
_S("gbreve")
_S("Idotaccent")
_S("Scedilla")
_S("scedilla")
_S("Cacute")
_S("cacute")
_S("Ccaron")
_S("ccaron")
_S("dcroat")


#endif /* HB_OT_POST_MACROMAN_HH */
