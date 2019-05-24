#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
void main()
{
    float w1   = 00000.000;
    float w2   = 1.0;
    float w3   = 007.00;
    float w4   = 000130000.0;
    float w5   = 123456789.0000;
    double w6  = 1234567890123456.0;
    double w7  = 12345678901234567.0;
    double w8  = 123456789012345678.0;
    double w9  = 12345678901234567893.0;
    double w10 = 1234567890123456789012345.0;

    float e1 = 0e0;
    float e2 = 1e0;
    float e3 = 0e14;
    float e4 = 1e15;
    float e5 = 1e16;
    float e6 = 0e-14;
    float e7 = 1e-15;
    float e8 = 1e-16;
    double e9 = 1e100;
    double e10 = 1e+308;
    double e11 = 1e-307; // Was 1e-323, but that's flushed to zero sometimes. 1e-308 can be flushed to 0.
    double e12 = 1e+309;
    double e13 = 1e-324;
    double e24 = 1e+999;
    double e25 = 1e-999;

    double f1 = 0.5;
    double f2 = 0.125;
    double f31 = 0.1;
    double f32 = 0.2;
    double f33 = 0.3;
    double f34 = 0.4;
    double f35 = 0.5;
    double f36 = 0.6;
    double f37 = 0.7;
    double f38 = 0.8;
    double f39 = 0.9;
    double f4 = 0.33333333333333333333333333333333333333333333333333333333333333333333333333333;
    double f51 = 0.000000000000000000000000000000000000783475;
    double f52 = 0.0000000000000000000000000000000000007834750;
    double f53 = .00000000000000000000000000000000000078347500;
    double f54 = 0.000000000000000000000000000000000000783475000000;
    double f61 = 4.;
    double f62 = 40.;
    double f63 = 0.;
    double f64 = 04.;
    double f65 = .0;
    double f66 = .004;
    double f67 = .400;
    double f68 = .04000;

    double c1  = .081e-2;
    double c2  = .073e2;
    double c3  = 34.5e-1;
    double c4  = 35.7e-4;
    double c5  = 43.9e1;
    double c6  = 52.2e4;
    double c7  = 000610000e2;
    double c8  = 000610000e-6;
    double c9  = 000001234567890123450000.0;
    double c10 = 000999999999999999999000.0;
    double c11 = 0001230000.0045600000;
    double c12 = 0001230000.00405600000e-3;
    double c13 = 0001230000.004500600000e-4;
    double c14 = 00010230000.0045600000e-5;
    double c15 = 000120030000.0045600000e4;
    double c16 = 0001230000.0045600000e5;
    double c17 = 0001230000.0045600000e6;
    double c18 = 0001230000.00456007e6;

    double b11 = 72057594037927928.0;
    double b12 = 72057594037927936.0;
    double b13 = 72057594037927932.0;
    double b14 = 7205759403792793199999e-5;
    double b15 = 7205759403792793200001e-5;
    double b21 = 9223372036854774784.0;
    double b22 = 9223372036854775808.0;
    double b23 = 9223372036854775296.0;
    double b24 = 922337203685477529599999e-5;
    double b25 = 922337203685477529600001e-5;
    double b31 = 10141204801825834086073718800384.0;
    double b32 = 10141204801825835211973625643008.0;
    double b33 = 10141204801825834649023672221696.0;
    double b34 = 1014120480182583464902367222169599999e-5;
    double b35 = 1014120480182583464902367222169600001e-5;
    double b41 = 5708990770823838890407843763683279797179383808.0;
    double b42 = 5708990770823839524233143877797980545530986496.0;
    double b43 = 5708990770823839207320493820740630171355185152.0;
    double b44 = 5708990770823839207320493820740630171355185151999e-3;
    double b45 = 5708990770823839207320493820740630171355185152001e-3;

    float pi1 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679f;
    float pi2 = 3.14159265358979f;
    float pi3 = 3.141592653589793f;

    double dpi1 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679;
    double dpi2 = 3.14159265358979;
    double dpi3 = 3.141592653589793;

    float dfpi1 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679f;
    float dfpi2 = 3.14159265358979f;
    float dfpi3 = 3.141592653589793f;

    double lfpi1 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679LF;
    double lfpi2 = 3.14159265358979Lf;
    double lfpi3 = 3.141592653589793lF;

    double hfpi1 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679hF;
    double hfpi2 = 3.14159265358979hF;
    double hfpi3 = 3.141592653589793hf;
}
