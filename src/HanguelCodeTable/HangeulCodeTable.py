﻿import os

pathIn = 'HangeulCodeTable.txt'
pathOutH = 'HangeulCodeMap.h'
pathOutC_UTF16_KSSM = 'HangeulCodeMapUTF16_KSSM.cpp'
pathOutC_KSSM_UTF16 = 'HangeulCodeMapKSSM_UTF16.cpp'
pathOutModule_UTF16_KSSM = 'string_convert_codepage_utf16_to_kssm.ixx'
pathOutModule_KSSM_UTF16 = 'string_convert_codepage_kssm_to_utf16.ixx'

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

##=================
## .h
#fOutHeader = open(pathOutH, mode='w', encoding='utf-8-sig')
#fOutHeader.write(
#                '#pragma once\n'
#                '//=========\n'
#                '//Automatically Generated File.\n'
#                '//\n'
#                '//        PWH.\n'
#                '//\n'
#                '//=========\n'
#                '\n'
#                '\n'
#                '#include "gtl/config_gtl.h"\n\n'
#                '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n\n'
#                '#include <cstdint>\n'
#                '#include <map>\n'
#                '\n'
#                '\n'
#                '#include "gtl/_lib_gtl.h"\n\n'
#                'namespace gtl::charset::KSSM {\n'
#                '#pragma pack(push, 8)\n'
#                '\n'
#                '\n'
#                '\tGTL_DATA extern std::map<char16_t, uint16_t> const mapUTF16toKSSM_g;\n'
#                '\tGTL_DATA extern std::map<uint16_t, char16_t> const mapKSSMtoUTF16_g;\n'
#                '\n'
#                '#pragma pack(pop)\n'
#                '}\t// namespace gtl::charset::KSSM\n'
#                '\n\n'
#                '#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
#                )
#fOutHeader.flush()

##=================
## .cpp U16->KSSM
#fOutImpl = open(pathOutC_UTF16_KSSM, mode='w', encoding='utf-8-sig')
#fOutImpl.write( '#include "pch.h"\n\n'
#                '//=========\n'
#                '//Automatically Generated File.\n'
#                '//\n'
#                '//        PWH.\n'
#                '//\n'
#                '//=========\n'
#                '\n\n'
#                '#include "gtl/config_gtl.h"\n'
#                '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
#                '\n\n'
#                '#include <cstdlib>\n'
#                '#include <cstdint>\n'
#                '#include <array>\n\n'
#                '#include "gtl/string/' + pathOutH + '"\n\n\n'
#                'namespace gtl::charset::KSSM {\n\n\n'
#                '\t' + strLineSeperator
#                )
#
##-----------------
## map UTF16 -> KSSM
#codes.sort(key=codepair.KeyUnicode)
#nCount = 0
#fOutImpl.write('\t' + strLineSeperator)
#fOutImpl.write('\tGTL_DATA std::map<char16_t, uint16_t> const mapUTF16toKSSM_g { {\n')
#for code in codes :
#    if (nCount == 0) :
#        fOutImpl.write('\t\t');
#
#    out = '{{ 0x{v.codeUni:<4}/*{codePrint}*/, 0x{v.codeKSSM:<4} }},'.format(v = code, codePrint = chr(int(code.codeUni, 16)));
#    fOutImpl.write(out)
#
#    nCount += 1;
#    if (nCount < 8) :
#        fOutImpl.write('\t');
#    else :
#        nCount = 0;
#        fOutImpl.write('\n');
#
#fOutImpl.write('\n\t} };\n')
#fOutImpl.write('\n\n')
#
#fOutImpl.write('}\t// namespace gtl::charset::KSSM\n\n'
#               '#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
#               )
#fOutImpl.flush()
#fOutImpl.close()

#=================
# .ixx UTF16->KSSM
fOutModuleUTF16_KSSM = open(pathOutModule_UTF16_KSSM, mode='w', encoding='utf-8-sig')
fOutModuleUTF16_KSSM.write(
        'module ;\n'
        '\n'
        '#include "gtl/_config.h"\n'
        '#include "gtl/_macro.h"\n'
        '\n'
        '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
        '\t#include <map>\n'
        '#endif\n'
        '\n'
        'export module gtl:string_convert_codepage_utf16_to_kssm;\n'
        '\n'
        '//=========\n'
        '//Automatically Generated File.\n'
        '//\n'
        '//        PWH.\n'
        '//\n'
        '//=========\n'
        '\n'
        '\n'
        '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
        '\n'
        '\n'
        'namespace gtl::charset::KSSM {\n'
        '\n'
        '\n'
        '\n'
        )
codes.sort(key=codepair.KeyUnicode)
nCount = 0
fOutModuleUTF16_KSSM.write('\t' + strLineSeperator)
fOutModuleUTF16_KSSM.write('\tstd::map<char16_t, uint16_t> const mapUTF16toKSSM_g { {\n')
for code in codes :
    if (nCount == 0) :
        fOutModuleUTF16_KSSM.write('\t\t');

    out = '{{ 0x{v.codeUni:<4}/*{codePrint}*/, 0x{v.codeKSSM:<4} }},'.format(v = code, codePrint = chr(int(code.codeUni, 16)));
    fOutModuleUTF16_KSSM.write(out)

    nCount += 1;
    if (nCount < 8) :
        fOutModuleUTF16_KSSM.write('\t');
    else :
        nCount = 0;
        fOutModuleUTF16_KSSM.write('\n');

fOutModuleUTF16_KSSM.write('\n\t} };\n')
fOutModuleUTF16_KSSM.write('\n\n')

fOutModuleUTF16_KSSM.write('}\t// namespace gtl::charset::KSSM\n\n'
               '#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n')
fOutModuleUTF16_KSSM.flush()
fOutModuleUTF16_KSSM.close()


##=================
## .cpp KSSM->U16
#fOutImpl = open(pathOutC_KSSM_UTF16, mode='w', encoding='utf-8-sig')
#fOutImpl.write( '#include "pch.h"\n\n'
#                '//=========\n'
#                '//Automatically Generated File.\n'
#                '//\n'
#                '//        PWH.\n'
#                '//\n'
#                '//=========\n'
#                '\n\n'
#                '#include "gtl/config_gtl.h"\n'
#                '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
#                '\n\n'
#                '#include <cstdlib>\n'
#                '#include <cstdint>\n'
#                '#include <array>\n\n'
#                '#include "gtl/string/' + pathOutH + '"\n\n\n'
#                'namespace gtl::charset::KSSM {\n\n\n'
#                '\t' + strLineSeperator
#                )
#
##-----------------
## map KSSM -> UTF16
#codes.sort(key=codepair.KeyKSSM)
#nCount = 0
#fOutImpl.write('\t' + strLineSeperator)
#fOutImpl.write('\tGTL_DATA std::map<uint16_t, char16_t> const mapKSSMtoUTF16_g { {\n')
#for code in codes :
#    if (nCount == 0) :
#        fOutImpl.write('\t\t');
#
#    out = '{{ 0x{v.codeKSSM:<4}, 0x{v.codeUni:<4}/*{codePrint}*/ }},'.format(v = code, codePrint = chr(int(code.codeUni, 16)));
#    fOutImpl.write(out)
#
#    nCount += 1;
#    if (nCount < 8) :
#        fOutImpl.write('\t');
#    else :
#        nCount = 0;
#        fOutImpl.write('\n');
#
#fOutImpl.write('\n\t} };\n')
#fOutImpl.write('\n\n')
#
#fOutImpl.write('}\t// namespace gtl::charset::KSSM\n\n'
#               '#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
#               )
#fOutImpl.flush()
#fOutImpl.close()

#----------------
# .ixx KSSM -> UTF16
fOutModuleKSSM_UTF16 = open(pathOutModule_KSSM_UTF16, mode='w', encoding='utf-8-sig')
fOutModuleKSSM_UTF16.write(
        'module ;\n'
        '\n'
        '#include "gtl/_config.h"\n'
        '#include "gtl/_macro.h"\n'
        '\n'
        '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
        '\t#include <map>\n'
        '#endif\n'
        '\n'
        'export module gtl:string_convert_codepage_kssm_to_utf16;\n'
        '\n'
        '//=========\n'
        '//Automatically Generated File.\n'
        '//\n'
        '//        PWH.\n'
        '//\n'
        '//=========\n'
        '\n'
        '\n'
        '#if (GTL_STRING_SUPPORT_CODEPAGE_KSSM) && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n'
        '\n'
        '\n'
        'namespace gtl::charset::KSSM {\n'
        '\n'
        '\n'
        '\n'
        )

codes.sort(key=codepair.KeyKSSM)
nCount = 0
fOutModuleKSSM_UTF16.write('\t' + strLineSeperator)
fOutModuleKSSM_UTF16.write('\tstd::map<uint16_t, char16_t> const mapKSSMtoUTF16_g { {\n')
for code in codes :
    if (nCount == 0) :
        fOutModuleKSSM_UTF16.write('\t\t');

    out = '{{ 0x{v.codeKSSM:<4}, 0x{v.codeUni:<4}/*{codePrint}*/ }},'.format(v = code, codePrint = chr(int(code.codeUni, 16)));
    fOutModuleKSSM_UTF16.write(out)

    nCount += 1;
    if (nCount < 8) :
        fOutModuleKSSM_UTF16.write('\t');
    else :
        nCount = 0;
        fOutModuleKSSM_UTF16.write('\n');

fOutModuleKSSM_UTF16.write('\n\t} };\n')
fOutModuleKSSM_UTF16.write('\n\n')

fOutModuleKSSM_UTF16.write('}\t// namespace gtl::charset::KSSM\n\n'
               '#endif // GTL_STRING_SUPPORT_CODEPAGE_KSSM && !(GTL_STRING_SUPPORT_CODEPAGE_KSSM_LIBICONV)\n')
fOutModuleKSSM_UTF16.flush()
fOutModuleKSSM_UTF16.close()

