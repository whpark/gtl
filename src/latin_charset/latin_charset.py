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

def WriteCode_Module (codes, fmdl, bLower:bool) :
    prev = 0;
    sig = 'UL' if bLower else 'LU';
    for (mark, r) in codes :
        if (len(mark) == 0) :
            break;
        type = 'char16_t' if (max(r) < 0x10000) else 'char32_t';
        begin = 0xffffffff;
        end = 0;

        fmdl.write('//{:-<77}\n'.format('-'));
        fmdl.write('//{}\n'.format(mark));
        fmdl.write(f'\tstd::map<{type}, {type}> const map{sig}_{mark}_g = {{ \n');
        for i in r :
            c = chr(i);
            l = c.lower() if bLower else c.upper();
            if (c != l) :

                if (i-prev > 10) :
                    fmdl.write(f'\n//-----------{i-prev:>5}\n');
                prev = i;

                begin = min(begin, i)
                end = max(end, i)
                if (len(l) == 1) :
                    il = ord(l);
                    fmdl.write(f'\t\t{{ 0x{i:04x}, 0x{il:04x} }}, // {c}, {l} : {il-i}\n')
                else :
                    fmdl.write(f'//\t\t{{ 0x{i:04x}, u\'{l}\' }}, // {c}, {l}\n')

        fmdl.write('\t};\n\n');
        fmdl.write(f'\tconstexpr std::pair<{type}, {type}> const range{sig}_{mark}_g {{ 0x{begin:04x}, 0x{end+1:04x} }};\n');
#=====================================================================================
# header / cpp

path_mdl = 'string_latin_charset.ixx';
fmdl = open(path_mdl, mode = 'w', encoding='utf-8-sig');

fmdl.write(''
        '//=========\n'
        '//Automatically Generated File.\n'
        '//\n'
        '//        PWH.\n'
        '//\n'
        '//=========\n'
        '\n'
        'module;\n'
        '\n'
        '#include <cstdint>\n'
        '#include <map>\n'
        '\n'
        '#include "gtl/_config.h"\n'
        '#include "gtl/_macro.h"\n'
        '\n'
        'export module gtl:string_latin_charset;\n'
        '\n'
        'export namespace gtl::charset {\n'
        '\n'
        '\n'
        );

#-------------------------------------------------------------------------------------

WriteCode_Module(codes, fmdl, True);
WriteCode_Module(codes, fmdl, False);


fmdl.write('\n\n}; // export namespace gtl::charset\n\n')
fmdl.close();
