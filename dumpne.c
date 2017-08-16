#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>

#include "semblance.h"
#include "ne.h"

struct header_ne {
    word  ne_magic;             /* 00 NE signature 'NE' */
    byte  ne_ver;               /* 02 Linker version number */
    byte  ne_rev;               /* 03 Linker revision number */
    word  ne_enttab;            /* 04 Offset to entry table relative to NE */
    word  ne_cbenttab;          /* 06 Length of entry table in bytes */
    dword ne_crc;               /* 08 Checksum */
    word  ne_flags;             /* 0c Flags about segments in this file */
    byte  ne_autodata;          /* 0e Automatic data segment number */
    byte  ne_unused;            /* 0f */
    word  ne_heap;              /* 10 Initial size of local heap */
    word  ne_stack;             /* 12 Initial size of stack */
    word  ne_ip;                /* 14 Initial IP */
    word  ne_cs;                /* 16 Initial CS */
    word  ne_sp;                /* 18 Initial SP */
    word  ne_ss;                /* 1a Initial SS */
    word  ne_cseg;              /* 1c # of entries in segment table */
    word  ne_cmod;              /* 1e # of entries in module reference tab. */
    word  ne_cbnrestab;         /* 20 Length of nonresident-name table */
    word  ne_segtab;            /* 22 Offset to segment table */
    word  ne_rsrctab;           /* 24 Offset to resource table */
    word  ne_restab;            /* 26 Offset to resident-name table */
    word  ne_modtab;            /* 28 Offset to module reference table */
    word  ne_imptab;            /* 2a Offset to imported name table */
    dword ne_nrestab;           /* 2c Offset to nonresident-name table */
    word  ne_cmovent;           /* 30 # of movable entry points */
    word  ne_align;             /* 32 Logical sector alignment shift count */
    word  ne_cres;              /* 34 # of resource segments */
    byte  ne_exetyp;            /* 36 Flags indicating target OS */
    byte  ne_flagsothers;       /* 37 Additional information flags */
    word  ne_pretthunks;        /* 38 Offset to return thunks */
    word  ne_psegrefbytes;      /* 3a Offset to segment ref. bytes */
    word  ne_swaparea;          /* 3c Reserved by Microsoft */
    byte  ne_expver_min;        /* 3e Expected Windows version number (minor) */
    byte  ne_expver_maj;        /* 3f Expected Windows version number (major) */
};

static void print_flags(word flags){
    char buffer[1024];
    
    if ((flags & 0x0003) == 0)
        strcpy(buffer, "no DGROUP");
    else if ((flags & 0x0003) == 1)
        strcpy(buffer, "single DGROUP");
    else if ((flags & 0x0003) == 2)
        strcpy(buffer, "multiple DGROUPs");
    else if ((flags & 0x0003) == 3)
        strcpy(buffer, "(unknown DGROUP type 3)");
    if (flags & 0x0004)
        strcat(buffer, ", global initialization");
    if (flags & 0x0008)
        strcat(buffer, ", protected mode only");
    if (flags & 0x0010)
        strcat(buffer, ", 8086");
    if (flags & 0x0020)
        strcat(buffer, ", 80286");
    if (flags & 0x0040)
        strcat(buffer, ", 80386");
    if (flags & 0x0080)
        strcat(buffer, ", 80x87");
    if ((flags & 0x0700) == 0x0100)
        strcat(buffer, ", fullscreen"); /* FRAMEBUF */
    else if ((flags & 0x0700) == 0x0200)
        strcat(buffer, ", console"); /* API compatible */
    else if ((flags & 0x0700) == 0x0300)
        strcat(buffer, ", GUI"); /* uses API */
    else if ((flags & 0x0700) == 0)
        ; /* none? */
    else
        sprintf(buffer+strlen(buffer), ", (unknown application type %d)",
                (flags & 0x0700) >> 8);
    if (flags & 0x0800)
        strcat(buffer, ", self-loading"); /* OS/2 family */
    if (flags & 0x1000)
        strcat(buffer, ", (unknown flag 0x1000)");
    if (flags & 0x2000)
        strcat(buffer, ", contains linker errors");
    if (flags & 0x4000)
        strcat(buffer, ", non-conforming program");
    if (flags & 0x8000)
        strcat(buffer, ", library");
    
    printf("Flags: 0x%04x (%s)\n", flags, buffer);
}

static void print_os2flags(word flags){
    char buffer[1024];

    buffer[0] = 0;
    if (flags & 0x0001)
        strcat(buffer, ", long filename support");
    if (flags & 0x0002)
        strcat(buffer, ", 2.x protected mode");
    if (flags & 0x0004)
        strcat(buffer, ", 2.x proportional fonts");
    if (flags & 0x0008)
        strcat(buffer, ", fast-load area"); /* gangload */
    if (flags & 0xfff0)
        sprintf(buffer+strlen(buffer), ", (unknown flags 0x%04x)", flags & 0xfff0);

    if(buffer[0])
        printf("OS/2 flags: 0x%04x (%s)\n", flags, buffer+2);
    else
        printf("OS/2 flags: 0x0000\n");
}

static const char *const exetypes[] = {
    "unknown",                  /* 0 */
    "OS/2",                     /* 1 */
    "Windows (16-bit)",         /* 2 */
    "European Dos 4.x",         /* 3 */
    "Windows 386 (32-bit)",     /* 4 */
    "BOSS",                     /* 5 */
    0
};

void print_header(struct header_ne *header){
    printf("Linker version: %d.%d\n", header->ne_ver, header->ne_rev); /* 02 */
    printf("Checksum: %08x\n", header->ne_crc); /* 08 */
    print_flags(header->ne_flags); /* 0c */
    printf("Heap size: %d bytes\n", header->ne_heap); /* 10 */
    printf("Stack size: %d bytes\n", header->ne_stack); /* 12 */
    if (header->ne_exetyp <= 5) /* 36 */
        printf("Target OS: %s\n", exetypes[header->ne_exetyp]);
    else
        printf("Target OS: (unknown value %d)\n", header->ne_exetyp);
    print_os2flags(header->ne_flagsothers); /* 37 */
    printf("Swap area: %d\n", header->ne_swaparea); /* 3c */
    printf("Expected Windows version: %d.%d\n", /* 3e */
           header->ne_expver_maj, header->ne_expver_min);
    printf("\n");
}

void print_export(entry *entry_table, int count) {
    int i;

    for (i = 0; i < count; i++)
        if (entry_table[i].segment == 0xfe)
            /* absolute value */
            printf("\t%5d\t   %04x\t%s\n", i+1, entry_table[i].offset, entry_table[i].name ? entry_table[i].name : "<no name>");
        else if (entry_table[i].segment)
            printf("\t%5d\t%2d:%04x\t%s\n", i+1, entry_table[i].segment,
                entry_table[i].offset, entry_table[i].name ? entry_table[i].name : "<no name>");
}

void print_specfile(char *module_name, entry *entry_table, int count) {
    int i;
    FILE *specfile;
    char spec_name[13];
    sprintf(spec_name, "%.8s.ORD", module_name);
    specfile = fopen(spec_name, "w");
    if (!specfile) {
        fprintf(stderr, "Couldn't open %s: %s\n", spec_name, strerror(errno));
    }

    fprintf(specfile, "# Generated by dumpne -o\n");
    for (i = 0; i < count; i++) {
        if (entry_table[i].name)
            fprintf(specfile, "%d\t%s\n", i+1, entry_table[i].name);
        else if (entry_table[i].segment)
            fprintf(specfile, "%d\n", i+1);
    }

    fclose(specfile);
}

/* return the first entry (module name/desc) */
static char *read_res_name_table(long start, entry *entry_table){
    /* reads (non)resident names into our entry table */
    byte length;
    char *first;
    char *name;
    word ordinal;

    fseek(f, start, SEEK_SET);

    length = read_byte();
    first = malloc((length+1)*sizeof(char));
    fread(first, sizeof(char), length, f);
    first[length] = 0;
    fseek(f, sizeof(word), SEEK_CUR);

    while ((length = read_byte())){
        name = malloc((length+1)*sizeof(char));
        fread(name, sizeof(char), length, f);
        name[length] = 0;   /* null term */
        ordinal = read_word();
        entry_table[ordinal-1].name = name;
    }

    return first;
}

static unsigned get_entry_table(long start, entry **table) {
    byte length, index;
    int count = 0;
    entry *ret = NULL;
    unsigned i;

    /* get a count */
    fseek(f, start, SEEK_SET);
    while ((length = read_byte())) {
        index = read_byte();
        count += length;
        if (index != 0)
            fseek(f, ((index == 0xff) ? 6 : 3) * length, SEEK_CUR);
    }
    ret = calloc(sizeof(entry), count);

    fseek(f, start, SEEK_SET);
    count = 0;
    while ((length = read_byte())) {
        index = read_byte();
        for (i = 0; i < length; i++) {
            if (index == 0xff) {
                ret[count].flags = read_byte();
                fseek(f, 2, SEEK_CUR); /* CD 3F */
                ret[count].segment = read_byte();
                ret[count].offset = read_word();
            } else if (index == 0x00) {
                /* no entries, just here to skip ordinals */
            } else {
                ret[count].flags = read_byte();
                ret[count].segment = index;
                ret[count].offset = read_word();
            }
            count++;
        }
    }

    *table = ret;
    return count;
}

static void load_exports(import_module *module) {
    FILE *specfile;
    char spec_name[18];
    char line[300], *p;
    int count;
    word ordinal;

    sprintf(spec_name, "%.8s.ORD", module->name);
    specfile = fopen(spec_name, "r");
    if (!specfile) {
        sprintf(spec_name, "spec/%.8s.ORD", module->name);
        specfile = fopen(spec_name, "r");
        if (!specfile) {
            fprintf(stderr, "Note: couldn't find specfile for module %s; exported names won't be given.\n", module->name);
            fprintf(stderr, "      To create a specfile, run `dumpne -o <module.dll>'.\n");
            module->exports = NULL;
            module->export_count = 0;
            return;
        }
    }

    /* first grab a count */
    count = 0;
    while (fgets(line, sizeof(line), specfile)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        count++;
    }

    module->exports = malloc(count * sizeof(export));

    fseek(specfile, 0, SEEK_SET);
    count = 0;
    while (fgets(line, sizeof(line), specfile)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if ((p = strchr(line, '\n'))) *p = 0;   /* kill final newline */
        if (sscanf(line, "%hu", &ordinal) != 1) {
            fprintf(stderr, "Error reading specfile near line: `%s'\n", line);
            continue;
        }
        module->exports[count].ordinal = ordinal;

        p = strchr(line, '\t');
        if (!p) {
            module->exports[count].name = NULL;
            continue;
        }
        p++;
        module->exports[count].name = strdup(p);
        count++;
    }

    module->export_count = count;

    fclose(specfile);
}

static void get_import_module_table(long start, import_module **table, word count) {
    import_module *ret = NULL;
    word offset;
    word length;
    unsigned i;

    fseek(f, start, SEEK_SET);
    ret = malloc(count*sizeof(import_module));
    for (i = 0; i < count; i++) {
        offset = read_word();
        length  = import_name_table[offset];
        ret[i].name = malloc((length+1)*sizeof(char));
        memcpy(ret[i].name, &import_name_table[offset+1], length);
        ret[i].name[length] = 0;
        load_exports(&ret[i]);
    }

    *table = ret;
}

static void free_entry_table(entry *table, int count) {
    int i;
    if (table) {
        for (i = 0; i < count; i++)
            free(table[i].name);
        free(table);
    }
}

static void free_import_module_table(import_module *table, word count) {
    unsigned i, j;
    if (table) {
        for (i=0;i<count;i++) {
            free(table[i].name);
            for (j = 0; j < table[i].export_count; j++)
                free(table[i].exports[j].name);
            free(table[i].exports);
        }
        free(table);
    }
}

void dump_file(char *file){
    word magic;
    int mz = 0; /* found an MZ header? */
    long offset_ne = 0;
    struct header_ne header;
    char *module_name = NULL;
    char *module_desc = NULL;
    int i;

    /* clear our globals */
    f = NULL;
    entry_table = NULL;
    entry_count = 0;
    import_name_table = NULL;
    import_module_table = NULL;

    fprintf(stderr, "%s\n", file);

    f = fopen(file, "r");
    if (!f) {
        fprintf(stderr, "Cannot open %s: %s\n", file, strerror(errno));
        return;
    }

    /* print header info */
    if (fread(&magic, 2, 1, f) != 1) {
        fprintf(stderr, "Cannot read %s: %s\n", file, strerror(errno));
        goto done;
    }

    if (magic == 0x5a4d){ /* MZ */
        fseek(f, 0x3c, SEEK_SET);
        offset_ne = read_dword();
        fseek(f, offset_ne, SEEK_SET);
        mz = 1;
    }

    fread(&header, sizeof(header), 1, f);

    if (header.ne_magic == 0x4550){
        fprintf(stderr, "PE header found\n");
        goto done;
    } else if (header.ne_magic != 0x454e){
        if (mz)
            fprintf(stderr, "MZ header found but no NE header\n");
        else
            fprintf(stderr, "no NE header found\n");
        goto done;
    }

    /* read our various tables */
    entry_count = get_entry_table(offset_ne + header.ne_enttab, &entry_table);
    module_name = read_res_name_table(offset_ne + header.ne_restab, entry_table);
    module_desc = read_res_name_table(header.ne_nrestab, entry_table);
    fseek(f, offset_ne + header.ne_imptab, SEEK_SET);
    import_name_table = malloc(header.ne_enttab - header.ne_imptab);
    fread(import_name_table, sizeof(byte), header.ne_enttab - header.ne_imptab, f);
    get_import_module_table(offset_ne + header.ne_modtab, &import_module_table, header.ne_cmod);

    if (mode == SPECFILE) {
        print_specfile(module_name, entry_table, entry_count);
        goto done;
    }

    printf("Module name: %s\n", module_name);
    printf("Module description: %s\n\n", module_desc);

    if (mode & DUMPHEADER)
        print_header(&header);

    if (mode & DUMPEXPORT) {
        printf("Exports:\n");
        print_export(entry_table, entry_count);
    }

    if (mode & DUMPIMPORTMOD) {
        printf("Imported modules:\n");
        for (i = 0; i < header.ne_cmod; i++)
            printf("\t%s\n", import_module_table[i].name);
    }

    if (mode & DISASSEMBLE){
        fseek(f, offset_ne + header.ne_segtab, SEEK_SET);
        print_segments(header.ne_cseg, header.ne_align, header.ne_cs, header.ne_ip);
    }

    if (mode & DUMPRSRC){
        fseek(f, offset_ne + header.ne_rsrctab, SEEK_SET);
        if (header.ne_rsrctab != header.ne_restab)
            print_rsrc(offset_ne + header.ne_rsrctab);
        else
            printf("no resource table\n");
    }

//    printf("%s:\t%04x %04x %04x %04x %04x %04x %04x\n", file, header.ne_segtab, header.ne_rsrctab, header.ne_restab, header.ne_modtab, header.ne_imptab, header.ne_enttab, header.ne_nrestab);

done:
    free_entry_table(entry_table, entry_count);
    free(import_name_table);
    free_import_module_table(import_module_table, header.ne_cmod);
    free(module_name);
    free(module_desc);
    fclose(f);
    fflush(stdout);
    return;
}

word disassemble_segment[MAXARGS] = {0};
word disassemble_count = 0;

word resource_type[MAXARGS] = {0};
word resource_id[MAXARGS] = {0};
word resource_count = 0;

static const struct option long_options[] = {
    {"disassemble",   optional_argument, NULL, 'd'},
    {"dump-header",   no_argument,       NULL, 'h'},
    {"specfile",      no_argument,       NULL, 'o'},
    {"dump-resource", optional_argument, NULL, 'r'},
    {"full-contents", no_argument,       NULL, 's'},
    {0,0,0,0}
};

int main(int argc, char *argv[]){
    int opt;

    mode = 0;
    
    while ((opt = getopt_long(argc, argv, "d::hor::s", long_options, NULL)) >= 0){
        switch (opt) {
        case 'd': /* disassemble only */
            mode |= DISASSEMBLE;
            if (optarg){
                if (disassemble_count == MAXARGS){
                    fprintf(stderr, "Too many segments specified\n");
                    return 1;
                }
                if (!sscanf(optarg, "%hu", &disassemble_segment[disassemble_count++])){
                    fprintf(stderr, "Not a segment number: '%s'\n", optarg);
                    return 1;
                }
            }
            break;
        case 'h': /* dump header only */
            mode |= DUMPHEADER;
            break;
        case 'o': /* make a specfile */
            mode = SPECFILE;
            break;
        case 'r': /* dump resources only */
        {
            int ret;
            char type[256];
            unsigned i;
            mode |= DUMPRSRC;
            if (optarg){
                if (resource_count == MAXARGS){
                    fprintf(stderr, "Too many resources specified\n");
                    return 1;
                }
                if (0 >= (ret = sscanf(optarg, "%s %hu", type, &resource_id[resource_count])))
                    /* empty argument, so do nothing */
                    break;
                if (ret == 1)
                    resource_id[resource_count] = 0;

                /* todo(?): let the user specify string [exe-defined] types, and also
                 * string id names */
                if (!sscanf(type, "%hu", &resource_type[resource_count])){
                    resource_type[resource_count] = 0;
                    for (i=1;i<rsrc_types_count;i++){
                        if(rsrc_types[i] && !strcasecmp(rsrc_types[i], type)){
                            resource_type[resource_count] = 0x8000|i;
                            break;
                        }
                    }
                    if(!resource_type[resource_count]){
                        fprintf(stderr, "Unrecognized resource type '%s'\n", type);
                        return 1;
                    }
                }
                resource_count++;
            }
            break;
        }
        case 's': /* dump everything */
            mode |= DUMPHEADER | DUMPRSRC | DISASSEMBLE;
            break;
        default: /* '?' */
            /* todo: probably just print the whole help message here */
            fprintf(stderr, "Usage: dumpne (-d|-h|-r|-s) <file>\n");
            return 1;
        }
    }

    if (mode == 0)
        mode = ~0;

    /* todo */
    asm_syntax = MASM;

    if (optind == argc)
        printf("No input given\n");

    while (optind < argc){
        dump_file(argv[optind++]);
    }
}