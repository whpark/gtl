import os

codes = [
        ('latin1',              range(0x0080,  0x0100)),
        ('latin_extended',      range(0x0100,  0x0250)),
        #('latin_extended_A',    range(0x0100,  0x0180)),
        #('latin_extended_B',    range(0x0180,  0x0250)),
        ('other1',              range(0x0250,  0xA000)),
        ('other2',              range(0xA000,  0xE000)),
        ('other3',              range(0xE000,  0x10000)),
        ('other4',              range(0x10000, 0x12400)),
        ('other5',              range(0x12400, 0x1E000)),
        ('other6',              range(0x1E000, 0x10FFFF)),
        ('', range(0,0))
       ]

def WriteCode (codes, fh, fcpp, bLower:bool) :
    prev = 0;
    sig = 'UL' if bLower else 'LU';
    for (mark, r) in codes :
        if (len(mark) == 0) :
            break;
        type = 'char16_t' if (max(r) < 0x10000) else 'char32_t';
        fcpp.write('//{:-<77}\n'.format('-'));
        fcpp.write('//{}\n'.format(mark));
        fcpp.write(f'\tGTL_DATA std::map<{type}, {type}> const map{sig}_{mark}_g = {{ \n');
        begin = 0xffffffff;
        end = 0;
        for i in r :
            c = chr(i);
            l = c.lower() if bLower else c.upper();
            if (c != l) :

                if (i-prev > 10) :
                    fcpp.write(f'\n//-----------{i-prev:>5}\n');
                prev = i;

                begin = min(begin, i)
                end = max(end, i)
                if (len(l) == 1) :
                    il = ord(l);
                    fcpp.write(f'\t\t{{ 0x{i:04x}, 0x{il:04x} }}, // {c}, {l} : {il-i}\n')
                else :
                    fcpp.write(f'//\t\t{{ 0x{i:04x}, u\'{l}\' }}, // {c}, {l}\n')

        fcpp.write('\t};\n\n');
        fh.write(f'\tconstexpr static inline std::pair<{type}, {type}> const range{sig}_{mark}_g {{ 0x{begin:04x}, 0x{end+1:04x} }};\n');

#=====================================================================================
# header / cpp

path_h = 'latin_charset.h';
fh = open(path_h, mode = 'w', encoding='utf-8-sig');

fh.write('#pragma once\n'
        '//=========\n'
        '//Automatically Generated File.\n'
        '//\n'
        '//        PWH.\n'
        '//\n'
        '//=========\n'
        '\n'
        '\n'
        '#include "gtl/config_gtl.h"\n'
        '#include "gtl/_lib_gtl.h"\n'
        '\n'
        '#include <cstdint>\n'
        '#include <map>\n'
        '\n'
        '\n'
        'namespace gtl::charset {\n'
        '\n\n'
        );

path_cpp = 'latin_charset.cpp';
fcpp = open(path_cpp, mode = 'w', encoding='utf-8-sig');

fcpp.write('#include "pch.h"\n'
        '\n'
        '//=========\n'
        '//Automatically Generated File.\n'
        '//\n'
        '//        PWH.\n'
        '//\n'
        '//=========\n'
        '\n'
        '\n'
        '#include "gtl/string/latin_charset.h"\n'
        '\n'
        'namespace gtl::charset {\n'
        '#pragma pack(push, 8)\n'
        '\n'
        '\n'
        '\n'
        );


#-------------------------------------------------------------------------------------

for (mark, r) in codes :
    if (len(mark) == 0) :
        break;
    type = 'char16_t' if (max(r) < 0x10000) else 'char32_t';
    fh.write(f'\tGTL_DATA extern std::map<{type}, {type}> const mapUL_{mark}_g;\n');
    fh.write(f'\tGTL_DATA extern std::map<{type}, {type}> const mapLU_{mark}_g;\n');
fh.write('\n')



WriteCode(codes, fh, fcpp, True);
WriteCode(codes, fh, fcpp, False);


fcpp.write('\n\n}; // namespace gtl::charset\n\n')
fcpp.close();

fh.write('\n\n} // namespace gtl::charset\n\n')
fh.flush();
