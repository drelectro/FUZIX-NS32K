/*
 *  Dhrystone Benchmark 2.2
 *
 *  Original: Reinhold P. Weicker, CACM Vol 27 No 10, 10/84 pg 1013
 *            Version 2.2: anti-optimisation additions, no register
 *
 *  Adapted for FUZIX on CPC-8 (Z80 @ 2.4576 MHz):
 *    - int is 16-bit: long used for iteration count and timing
 *    - Timing via times() syscall; kernel tick = 10/sec (deciseconds)
 *    - Single file (fcc has no cross-file optimisation to defeat)
 *    - Integer-only result calculations (no floating point)
 *    - Record union flattened (only variant 1 is used by benchmark)
 *    - Static record allocation (no malloc)
 *
 *  Build:  See build_dhrystone.sh
 *  Run:    dhrystone [iterations]     (default: 1000)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>

/* ------------------------------------------------------------------ */
/* Dhrystone type definitions                                         */
/* ------------------------------------------------------------------ */

#define Ident_1    0
#define Ident_2    1
#define Ident_3    2
#define Ident_4    3
#define Ident_5    4

typedef int         Enumeration;
typedef int         One_Thirty;
typedef int         One_Fifty;
typedef char        Capital_Letter;
typedef int         Boolean;
typedef char        Str_30[31];
typedef int         Arr_1_Dim[50];
typedef int         Arr_2_Dim[50][50];

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*
 * Record type -- union flattened, only variant 1 used by benchmark.
 * Size is identical to the original union version (39 bytes on Z80).
 */
typedef struct record {
    struct record  *Ptr_Comp;
    Enumeration     Discr;
    Enumeration     Enum_Comp;
    int             Int_Comp;
    Str_30          Str_Comp;
} Rec_Type, *Rec_Pointer;

/* ------------------------------------------------------------------ */
/* Global variables                                                   */
/* ------------------------------------------------------------------ */

Rec_Pointer     Ptr_Glob;
Rec_Pointer     Next_Ptr_Glob;
int             Int_Glob;
Boolean         Bool_Glob;
char            Ch_1_Glob;
char            Ch_2_Glob;
Arr_1_Dim       Arr_1_Glob;
Arr_2_Dim       Arr_2_Glob;

/* Static record storage */
Rec_Type        Glob_Record;
Rec_Type        Next_Glob_Record;

/* ------------------------------------------------------------------ */
/* Forward declarations                                               */
/* ------------------------------------------------------------------ */

void Proc_1(Rec_Pointer Ptr_Val_Par);
void Proc_2(One_Fifty *Int_Par_Ref);
void Proc_3(Rec_Pointer *Ptr_Ref_Par);
void Proc_4(void);
void Proc_5(void);
void Proc_6(Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par);
void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val,
            One_Fifty *Int_Par_Ref);
void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref,
            int Int_1_Par_Val, int Int_2_Par_Val);
Enumeration Func_1(Capital_Letter Ch_1_Par_Val,
                    Capital_Letter Ch_2_Par_Val);
Boolean Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref);
Boolean Func_3(Enumeration Enum_Par_Val);

/* ================================================================== */
/* Main program                                                       */
/* ================================================================== */

int main(int argc, char *argv[])
{
    One_Fifty       Int_1_Loc;
    One_Fifty       Int_2_Loc;
    One_Fifty       Int_3_Loc;
    char            Ch_Index;
    Enumeration     Enum_Loc;
    Str_30          Str_1_Loc;
    Str_30          Str_2_Loc;
    long            Number_Of_Runs;
    long            Run_Index;
    struct tms      tms_start;
    struct tms      tms_end;
    long            elapsed;
    long            dps_x100;
    long            dmips_x1000;

    /* Parse iteration count */
    if (argc > 1)
        Number_Of_Runs = (long)atoi(argv[1]);
    else
        Number_Of_Runs = 1000L;

    if (Number_Of_Runs < 1) {
        printf("Usage: dhrystone [n]\n");
        return 1;
    }

    /* Set up global record pointers */
    Ptr_Glob = &Glob_Record;
    Next_Ptr_Glob = &Next_Glob_Record;

    Ptr_Glob->Ptr_Comp  = Next_Ptr_Glob;
    Ptr_Glob->Discr     = Ident_1;
    Ptr_Glob->Enum_Comp = Ident_3;
    Ptr_Glob->Int_Comp  = 40;
    strcpy(Ptr_Glob->Str_Comp,
        "DHRYSTONE PROGRAM, SOME STRING");

    strcpy(Str_1_Loc,
        "DHRYSTONE PROGRAM, 1'ST STRING");

    Arr_2_Glob[8][7] = 10;

    printf("Dhrystone 2.2 (Rarr Special)\n");
    printf("Runs: %ld\n", Number_Of_Runs);
    printf("Running...\n");
    fflush(stdout);

    /* ---- START TIMING (CTC-backed) ---- */
    times(&tms_start);

    for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index) {

        Proc_5();
        Proc_4();

        Int_1_Loc = 2;
        Int_2_Loc = 3;
        strcpy(Str_2_Loc,
            "DHRYSTONE PROGRAM, 2'ND STRING");

        Enum_Loc = Ident_2;
        Bool_Glob = !Func_2(Str_1_Loc, Str_2_Loc);

        while (Int_1_Loc < Int_2_Loc) {
            Int_3_Loc = 5 * Int_1_Loc - Int_2_Loc;
            Proc_7(Int_1_Loc, Int_2_Loc, &Int_3_Loc);
            Int_1_Loc += 1;
        }

        Proc_8(Arr_1_Glob, Arr_2_Glob,
               Int_1_Loc, Int_3_Loc);

        Proc_1(Ptr_Glob);

        for (Ch_Index = 'A'; Ch_Index <= Ch_2_Glob; ++Ch_Index) {
            if (Enum_Loc == Func_1(Ch_Index, 'C'))
                Proc_6(Ident_1, &Enum_Loc);
        }

        /* Version 2.2 additions (defeat dead-code elimination) */
        Int_3_Loc = Int_2_Loc * Int_1_Loc;
        Int_2_Loc = Int_3_Loc / Int_1_Loc;
        Int_2_Loc = 7 * (Int_3_Loc - Int_2_Loc) - Int_1_Loc;

        Proc_2(&Int_1_Loc);
    }

    /* ---- END TIMING ---- */
    times(&tms_end);

    printf("\nDone.\n\n");

    /* Elapsed ticks -- use tms_etime (ticks.full = deciseconds, 10/sec).
     * Note: tms_utime/tms_stime are raw CTC ticks (100/sec) — different
     * units from tms_etime.  The fallback would need /100 not /10. */
    elapsed = (long)(tms_end.tms_etime - tms_start.tms_etime);

    printf("E start: %ld End: %ld\n", tms_start.tms_etime, tms_end.tms_etime);
    printf("U start: %ld End: %ld\n", tms_start.tms_utime, tms_end.tms_utime);

    if (elapsed <= 0) {
        /* Fallback: user + system CPU ticks (100/sec), scale to 10/sec */
        elapsed = (long)((tms_end.tms_utime + tms_end.tms_stime)
                       - (tms_start.tms_utime + tms_start.tms_stime));
        elapsed = (elapsed + 5L) / 10L;
    }

    if (elapsed <= 0) {
        printf("Elapsed = 0, check your timer or try more iterations\n");
        //return 1;
        elapsed = 1; /* Avoid divide-by-zero for very short runs */
    }

    /* Print timing -- kernel tick = 10/sec (deciseconds) */
    printf("Time: %ld.%ld s  (%ld ticks)\n",
        elapsed / 10L, elapsed % 10L, elapsed);

    /* Dhrystones/sec with 2 decimal places */
    dps_x100 = Number_Of_Runs * 1000L / elapsed;
    printf("Dhrystones/s: %ld.%02ld\n",
        dps_x100 / 100L, dps_x100 % 100L);

    /* DMIPS = Dhrystones/s / 1757, displayed with 3 decimal places */
    dmips_x1000 = dps_x100 * 10L / 1757L;
    printf("DMIPS: %ld.%03ld\n",
        dmips_x1000 / 1000L, dmips_x1000 % 1000L);

    /* Verification output */
    printf("\n--- Verification ---\n");
    printf("Int_Glob:    %d\n", Int_Glob);
    printf("Bool_Glob:   %d\n", Bool_Glob);
    printf("Ch_1_Glob:   %c\n", Ch_1_Glob);
    printf("Ch_2_Glob:   %c\n", Ch_2_Glob);
    printf("Arr_1[8]:    %d\n", Arr_1_Glob[8]);
    printf("Arr_2[8][7]: %d\n", Arr_2_Glob[8][7]);
    printf("Ptr->Discr:  %d\n", Ptr_Glob->Discr);
    printf("Ptr->Enum:   %d\n", Ptr_Glob->Enum_Comp);
    printf("Ptr->Int:    %d\n", Ptr_Glob->Int_Comp);
    printf("Ptr->Str:    %s\n", Ptr_Glob->Str_Comp);
    printf("Nxt->Discr:  %d\n", Next_Ptr_Glob->Discr);
    printf("Nxt->Enum:   %d\n", Next_Ptr_Glob->Enum_Comp);
    printf("Nxt->Int:    %d\n", Next_Ptr_Glob->Int_Comp);
    printf("Nxt->Str:    %s\n", Next_Ptr_Glob->Str_Comp);
    printf("Int_1_Loc:   %d\n", Int_1_Loc);
    printf("Int_2_Loc:   %d\n", Int_2_Loc);
    printf("Int_3_Loc:   %d\n", Int_3_Loc);
    printf("Enum_Loc:    %d\n", Enum_Loc);
    printf("Str_1_Loc:   %s\n", Str_1_Loc);
    printf("Str_2_Loc:   %s\n", Str_2_Loc);

    return 0;
}

/* ================================================================== */
/* Proc_1                                                             */
/* ================================================================== */

void Proc_1(Rec_Pointer Ptr_Val_Par)
{
    Rec_Pointer Next_Record;

    Next_Record = Ptr_Val_Par->Ptr_Comp;

    memcpy(Ptr_Val_Par->Ptr_Comp, Ptr_Glob, sizeof(Rec_Type));

    Ptr_Val_Par->Int_Comp = 5;
    Next_Record->Int_Comp = Ptr_Val_Par->Int_Comp;
    Next_Record->Ptr_Comp = Ptr_Val_Par->Ptr_Comp;
    Proc_3(&Next_Record->Ptr_Comp);

    if (Next_Record->Discr == Ident_1) {
        Next_Record->Int_Comp = 6;
        Proc_6(Ptr_Val_Par->Enum_Comp,
               &Next_Record->Enum_Comp);
        Next_Record->Ptr_Comp = Ptr_Glob->Ptr_Comp;
        Proc_7(Next_Record->Int_Comp, 10,
               &Next_Record->Int_Comp);
    } else {
        memcpy(Ptr_Val_Par, Ptr_Val_Par->Ptr_Comp, sizeof(Rec_Type));
    }
}

/* ================================================================== */
/* Proc_2                                                             */
/* ================================================================== */

void Proc_2(One_Fifty *Int_Par_Ref)
{
    One_Fifty   Int_Loc;
    Enumeration Enum_Loc;

    Int_Loc = *Int_Par_Ref + 10;
    do {
        if (Ch_1_Glob == 'A') {
            Int_Loc -= 1;
            *Int_Par_Ref = Int_Loc - Int_Glob;
            Enum_Loc = Ident_1;
        }
    } while (Enum_Loc != Ident_1);
}

/* ================================================================== */
/* Proc_3                                                             */
/* ================================================================== */

void Proc_3(Rec_Pointer *Ptr_Ref_Par)
{
    if (Ptr_Glob != 0)
        *Ptr_Ref_Par = Ptr_Glob->Ptr_Comp;
    Proc_7(10, Int_Glob, &Ptr_Glob->Int_Comp);
}

/* ================================================================== */
/* Proc_4                                                             */
/* ================================================================== */

void Proc_4(void)
{
    Boolean Bool_Loc;

    Bool_Loc = (Ch_1_Glob == 'A');
    Bool_Glob = Bool_Loc | Bool_Glob;
    Ch_2_Glob = 'B';
}

/* ================================================================== */
/* Proc_5                                                             */
/* ================================================================== */

void Proc_5(void)
{
    Ch_1_Glob = 'A';
    Bool_Glob = FALSE;
}

/* ================================================================== */
/* Proc_6                                                             */
/* ================================================================== */

void Proc_6(Enumeration Enum_Val_Par, Enumeration *Enum_Ref_Par)
{
    *Enum_Ref_Par = Enum_Val_Par;
    if (!Func_3(Enum_Val_Par))
        *Enum_Ref_Par = Ident_4;
    switch (Enum_Val_Par) {
    case Ident_1:
        *Enum_Ref_Par = Ident_1;
        break;
    case Ident_2:
        if (Int_Glob > 100)
            *Enum_Ref_Par = Ident_1;
        else
            *Enum_Ref_Par = Ident_4;
        break;
    case Ident_3:
        *Enum_Ref_Par = Ident_2;
        break;
    case Ident_4:
        break;
    case Ident_5:
        *Enum_Ref_Par = Ident_3;
        break;
    }
}

/* ================================================================== */
/* Proc_7                                                             */
/* ================================================================== */

void Proc_7(One_Fifty Int_1_Par_Val, One_Fifty Int_2_Par_Val,
            One_Fifty *Int_Par_Ref)
{
    One_Fifty Int_Loc;

    Int_Loc = Int_1_Par_Val + 2;
    *Int_Par_Ref = Int_2_Par_Val + Int_Loc;
}

/* ================================================================== */
/* Proc_8                                                             */
/* ================================================================== */

void Proc_8(Arr_1_Dim Arr_1_Par_Ref, Arr_2_Dim Arr_2_Par_Ref,
            int Int_1_Par_Val, int Int_2_Par_Val)
{
    One_Fifty Int_Loc;
    One_Fifty Int_Index;

    Int_Loc = Int_1_Par_Val + 5;
    Arr_1_Par_Ref[Int_Loc] = Int_2_Par_Val;
    Arr_1_Par_Ref[Int_Loc + 1] = Arr_1_Par_Ref[Int_Loc];
    Arr_1_Par_Ref[Int_Loc + 30] = Int_Loc;

    for (Int_Index = Int_Loc; Int_Index <= Int_Loc + 1; ++Int_Index)
        Arr_2_Par_Ref[Int_Loc][Int_Index] = Int_Loc;

    Arr_2_Par_Ref[Int_Loc][Int_Loc - 1] += 1;
    Arr_2_Par_Ref[Int_Loc + 20][Int_Loc] = Arr_1_Par_Ref[Int_Loc];
    Int_Glob = 5;
}

/* ================================================================== */
/* Func_1                                                             */
/* ================================================================== */

Enumeration Func_1(Capital_Letter Ch_1_Par_Val,
                    Capital_Letter Ch_2_Par_Val)
{
    Capital_Letter Ch_1_Loc;
    Capital_Letter Ch_2_Loc;

    Ch_1_Loc = Ch_1_Par_Val;
    Ch_2_Loc = Ch_1_Loc;
    if (Ch_2_Loc != Ch_2_Par_Val)
        return Ident_1;
    else {
        Ch_1_Glob = Ch_1_Loc;
        return Ident_2;
    }
}

/* ================================================================== */
/* Func_2                                                             */
/* ================================================================== */

Boolean Func_2(Str_30 Str_1_Par_Ref, Str_30 Str_2_Par_Ref)
{
    One_Thirty      Int_Loc;
    Capital_Letter  Ch_Loc;

    Ch_Loc = 0;
    Int_Loc = 2;
    while (Int_Loc <= 2) {
        if (Func_1(Str_1_Par_Ref[Int_Loc],
                    Str_2_Par_Ref[Int_Loc + 1]) == Ident_1) {
            Ch_Loc = 'A';
            Int_Loc += 1;
        }
    }

    if (Ch_Loc >= 'W' && Ch_Loc < 'Z')
        Int_Loc = 7;
    if (Ch_Loc == 'R')
        return TRUE;
    else {
        if (strcmp(Str_1_Par_Ref, Str_2_Par_Ref) > 0) {
            Int_Loc += 7;
            Int_Glob = Int_Loc;
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

/* ================================================================== */
/* Func_3                                                             */
/* ================================================================== */

Boolean Func_3(Enumeration Enum_Par_Val)
{
    Enumeration Enum_Loc;

    Enum_Loc = Enum_Par_Val;
    if (Enum_Loc == Ident_3)
        return TRUE;
    else
        return FALSE;
}
