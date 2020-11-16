import os

pathIn = 'HangeulCodeTable.txt'
pathOutH = 'HangeulCodeTable.h'
pathOutC = 'HangeulCodeTable.cpp'

class codepair:
    codeUni = ''
    codeKSSM = ''
    code5601 = ''
    code949 = ''
    index = ''
    def __init__(self, uni, kssm, code5601, code949, index):
        self.codeUni = uni
        self.codeKSSM = kssm
        self.code5601 = code5601
        self.code949 = code949
        self.index = index
    def KeyUnicode(self) -> int:
        return int(self.codeUni, 16)
    def KeyKSSM(self) -> int:
        return int(self.codeKSSM, 16)


# Read Hangeul Text File
#-----------------
fIn = open(pathIn, mode='r', encoding='utf8')
lines = fIn.readlines()

# read codes
codes = []
code5601_pre = '0'
nItems = 0
for line in list(lines):
    words = line.split('\t')
    if (len(words) <= 5) :
        continue

    codeUni = words[2].strip().lower()
    code5601 = words[3].strip().lower()
    code949 = words[4].strip().lower()
    codeKSSM = words[5].strip().lower()
    index = words[0].strip()
    codeChar = words[1].strip()
    codePrint = chr(int(codeUni, 16))

    if (len(code5601) <= 1) :
       code5601 = code5601_pre
    if (codePrint != codeChar) :
        raise Exception('codePrint({}) != codeChar({})'.format(codePrint, codeChar))

    codes.append(codepair(codeUni, codeKSSM, code5601, code949, index))
    nItems += 1;


#-----------------
strLineSeperator = '//------------------------------------------------------------------------------------------------\n'

#=================
# .h
fOutHeader = open(pathOutH, mode='w', encoding='utf-8-sig')
fOutHeader.write(
                '#pragma once\n'
                '//=========\n'
                '//Automatically Generated File.\n'
                '//\n'
                '//        PWH.\n'
                '//\n'
                '//=========\n'
                '\n'
                '\n'
                '#include "gtl/config_gtl.h"\n\n'
                '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM)\n\n'
                '#include <cstdint>\n'
                '#include <map>\n'
                '\n'
                '\n'
                '#include "gtl/_lib_gtl.h"\n\n'
                'namespace gtl {\n'
                '#pragma pack(push, 8)\n'
                '\n'
                '\n'
                '\tstruct S_HANGEUL_CODE {\n'
                '\t\twchar_t cUnicode;\n'
                '\t\tuint16_t cKSC;\n'
                '\t\tuint16_t c949;\n'
                '\t\tuint16_t cKSSM;\n'
                '\t};\n'
                '\t\n'
                '\tusing T_HANGEUL_TABLE = std::array<S_HANGEUL_CODE, ' + '{}'.format(nItems) + '>;\n\n'
                '\tGTL_DATA extern T_HANGEUL_TABLE const tblHangeulCode_g;\n'
                '\tGTL_DATA extern std::map<char16_t, uint16_t> const mapUTF16toKSSM_g;\n'
                '\tGTL_DATA extern std::map<uint16_t, char16_t> const mapKSSMtoUTF16_g;\n'
                '\n'
                '#pragma pack(pop)\n'
                '}\t// namespace gtl\n'
                '\n\n'
                '#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM\n'
                )

fOutHeader.flush()


#=================
# .cpp
fOutImpl = open(pathOutC, mode='w', encoding='utf-8-sig')
fOutImpl.write(#'#include "pch.h"\n\n' 
                '//=========\n'
                '//Automatically Generated File.\n'
                '//\n'
                '//        PWH.\n'
                '//\n'
                '//=========\n'
                '\n\n'
                '#include "gtl/config_gtl.h"\n'
                '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM)\n'
                '\n\n'
                '#include <cstdlib>\n'
                '#include <cstdint>\n'
                '#include <array>\n\n'
                '#include "gtl/string/' + pathOutH + '"\n\n\n'
                'namespace gtl {\n\n\n'
                )

fOutImpl.write('\t' + strLineSeperator)


#-----------------
# Whole Table
codes.sort(key=codepair.KeyUnicode)
nCount = 0
fOutImpl.write('\tGTL_DATA T_HANGEUL_TABLE const tblHangeulCode_g { {\n\n');
fOutImpl.write('\t\t//               unicode,  5601,    949,   kssm\n')
for code in codes :
    #out = '\t' + '/*' + code.index.format("{:5}") + '*/' + '{ 0x' + code.codeUni + '/*' + chr(int(code.codeUni, 16)) + '*/' + ', 0x' + code.code5601 + ', 0x' + code.code949 + ', 0x' + code.codeKSSM + ' },'
    #out = '\t /*{:5}*/ \{ 0x{}/*{}*/, 0x{}, 0x{}, 0x{} \}'.format(code.index, code.codeUni, chr(int(code.codeUni, 16)), code.code5601, code.code949, code.codeKSSM);
    if (nCount == 0) :
        fOutImpl.write('\t\t');

    out = '/*{v.index:>5}*/ {{ 0x{v.codeUni:<4}/*{codePrint}*/, 0x{v.code5601:<4}, 0x{v.code949:<4}, 0x{v.codeKSSM:<4} }}'.format(v = code, codePrint = chr(int(code.codeUni, 16)));
    fOutImpl.write(out)

    nCount += 1;
    if (nCount < 4) :
        fOutImpl.write(',\t');
    else :
        nCount = 0;
        fOutImpl.write(',\n');

fOutImpl.write('\n\t} };\n\n\n');
fOutImpl.write('\n\n');


#-----------------
# map W -> KSSM
codes.sort(key=codepair.KeyUnicode)
nCount = 0
fOutImpl.write('\t' + strLineSeperator)
fOutImpl.write('\tGTL_DATA std::map<char16_t, uint16_t> const mapUTF16toKSSM_g { {\n')
for code in codes :
    if (nCount == 0) :
        fOutImpl.write('\t\t');

    out = '{{ 0x{v.codeUni:<4}/*{codePrint}*/, 0x{v.codeKSSM:<4} }},'.format(v = code, codePrint = chr(int(code.codeUni, 16)));
    fOutImpl.write(out)

    nCount += 1;
    if (nCount < 8) :
        fOutImpl.write('\t');
    else :
        nCount = 0;
        fOutImpl.write('\n');

fOutImpl.write('\n\t} };\n')
fOutImpl.write('\n\n')

#-----------------
# map KSSM -> W
codes.sort(key=codepair.KeyKSSM)
nCount = 0
fOutImpl.write('\t' + strLineSeperator)
fOutImpl.write('\tGTL_DATA std::map<uint16_t, char16_t> const mapKSSMtoUTF16_g { {\n')
for code in codes :
    if (nCount == 0) :
        fOutImpl.write('\t\t');

    out = '{{ 0x{v.codeKSSM:<4}, 0x{v.codeUni:<4}/*{codePrint}*/ }},'.format(v = code, codePrint = chr(int(code.codeUni, 16)));
    fOutImpl.write(out)

    nCount += 1;
    if (nCount < 8) :
        fOutImpl.write('\t');
    else :
        nCount = 0;
        fOutImpl.write('\n');

fOutImpl.write('\n\t} };\n')
fOutImpl.write('\n\n')

fOutImpl.write('}\t// namespace gtl\n\n'
               '#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM\n'
               )

#fSource = open('./HangeulCodeTable.py', mode='r')
#sources = fSource.readlines()
#fOutImpl.write('//========================\n'
#           '/' '* python script :\n\n'
#           '----\n')
#for i in list(range(0, len(sources))) :
#    sources[i] = '\t' + sources[i];
#fOutImpl.writelines(sources)
#fOutImpl.write('----\n' '*' '/\n\n')

fOutImpl.flush()
