// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause
/*BEGIN_LEGAL
BSD License

Copyright (c)2022 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
class TWOLEVEL
{
public:
    TWOLEVEL();
    UINT64 References() { return _references;};
    UINT64 Predicts() { return _predicts;};
    UINT64 Icount() { return _icount;};
    UINT64 Mispredicts() { return _references - _predicts;};
    VOID Activate(ofstream *outfile)
    {
        _outfile = outfile;
        INS_AddInstrumentFunction(Instruction, this);
        PIN_AddFiniFunction(PrintStats, this);
    }

private:
    enum
    {
        TABLESIZE = 4096,
        PTHSIZE = 2
    };

    static VOID Instruction(INS ins, VOID *v);
    static VOID CondBranch(VOID *v, VOID * v2, VOID * v3, BOOL taken);
    static VOID Count(VOID *v);
    static VOID PrintStats(int n, VOID *v);
    static BOOL Predict(INT8 history);
    static VOID Update(INT8 *history_ptr, INT8 *pattern_ptr, BOOL outcome);
    INT8 * HistAddress(INS ins);

    INT8 * PatAddress(INS ins);
    INT8 * PatAddress2(INS ins);
//    INT8 * selector(INS ins);
    UINT8 Index(ADDRINT pc);

    ofstream *_outfile;

    INT8 _branchHistory[TABLESIZE];
    INT8 _branchPattern1[TABLESIZE];
    INT8 _branchPattern2[TABLESIZE];

    UINT64 _references, _predicts, _icount;
};

TWOLEVEL::TWOLEVEL()
{
    _references = 0;
    _predicts = 0;
    _icount = 0;
    _outfile = NULL;
}

VOID TWOLEVEL::Instruction(INS ins, VOID *v)
{
    TWOLEVEL *bm = reinterpret_cast<TWOLEVEL*>(v);
    INS_InsertCall(ins, IPOINT_BEFORE, // For counting the number of instructions
                   (AFUNPTR)Count, IARG_PTR, (VOID *)bm, IARG_END);
    if (INS_IsBranch(ins) && INS_HasFallThrough(ins)) // If a conditional instruction
    {
        INS_InsertCall(ins, IPOINT_BEFORE, // Run the CondBranch function
                       (AFUNPTR)CondBranch, IARG_PTR, (VOID *)bm,
                       IARG_PTR, (VOID *)bm->HistAddress(ins),
                       IARG_PTR, (VOID *)bm->PatAddress(ins),
                       IARG_PTR, (VOID *)bm->PatAddress2(ins),
//                       IARG_PTR, (VOID *)bm->selector(ins),
                       IARG_BRANCH_TAKEN, IARG_END);
    }
}

UINT8 TWOLEVEL::Index(ADDRINT pc)
{
    return (pc % TABLESIZE); // Simple linear hashing
}

INT8 * TWOLEVEL::HistAddress(INS ins)
{
    ADDRINT pc = INS_Address(ins);
    UINT8 index = Index(pc);
    return &(_branchHistory[index]);
}

INT8 * TWOLEVEL::PatAddress(INS ins)
{
    ADDRINT pc = INS_Address(ins);
    UINT8 index = Index(pc);
    return &(_branchPattern1[_branchHistory[index]]);
}

INT8 * TWOLEVEL::PatAddress2(INS ins)
{
    ADDRINT pc = INS_Address(ins);
    UINT8 index = Index(pc);
    return &(_branchPattern2[_branchPattern1[_branchHistory[index]]]);
}

//INT8 * BIMODAL::selector(INS ins){
//    ADDRINT pc = INS_Address(ins);
//    UINT8 index = Index(pc);
//    if () {
//
//    } else {
//
//    }
//
//    return
//}

BOOL TWOLEVEL::Predict(INT8 history_pattern) {
    if (history_pattern >= 2) return true; // Predict 'taken'
    if (history_pattern <= 1) return false; // Predict 'not taken'
    ASSERT(false, "Invalid history\n");
    return false;// control should not reach here
}

VOID TWOLEVEL::Update(INT8 *history_ptr, INT8 *pattern_ptr, BOOL outcome)
{
    (*history_ptr) = (*history_ptr)<<1;
    *history_ptr += outcome;
    //Counter saturates at 0 and 3
    //branch is taken
//    if (outcome && Predict(*pattern_ptr)) {
//        *pattern2_ptr = *pattern2_ptr + 3;
//    } else if (outcome) {
//        *pattern2_ptr = *pattern2_ptr + 2;
//    }
//    //new branch is not taken
//    else if (!outcome && !Predict(*pattern_ptr)){
//        *pattern2_ptr = *pattern2_ptr - 3;
//    } else if (outcome) {
//        *pattern2_ptr = *pattern2_ptr - 2;
//    }
    if(!outcome && !Predict(*pattern_ptr))
        *pattern_ptr -= 3;
    else if(!outcome)
        *pattern_ptr -= 2;
    else if(outcome && Predict(*pattern_ptr))
        *pattern_ptr += 3;
    else if(outcome)
        *pattern_ptr += 2;
    if(*pattern_ptr<0)
        *pattern_ptr = 0;
    else if(*pattern_ptr>12)
        *pattern_ptr = 12;
}

VOID TWOLEVEL::CondBranch(VOID *v, VOID * v2, VOID * v3,  BOOL outcome)
{
    TWOLEVEL *bm = reinterpret_cast<TWOLEVEL*>(v);
    INT8  *branchHistory = reinterpret_cast<INT8*>(v2);
    INT8  *patternHistory = reinterpret_cast<INT8*>(v3);
//    INT8  *patternHistory2 = reinterpret_cast<INT8*>(v4);
    // INT8 history, pattern;

    bm->_references += 1; // Count of conditional instructions

    (*branchHistory) = (*branchHistory) & 0xff;
    (*patternHistory) = (*patternHistory) & 0x7FF;
//    (*patternHistory2) = (*patternHistory2) & 0x7FF;
    if((outcome && Predict(*branchHistory)) || (!outcome && !Predict(*branchHistory)))
        bm->_predicts += 1;

    Update(branchHistory, patternHistory, outcome);
}

VOID TWOLEVEL::PrintStats(int n, VOID *v)
{
    TWOLEVEL *bm = reinterpret_cast<TWOLEVEL*>(v);
    *(bm->_outfile) << endl;
    *(bm->_outfile) << "Icount: " << dec << bm->Icount() << " Mispredicts: " << dec << bm->Mispredicts() << " MPKI: " << fixed << ((double)bm->Mispredicts()*1000)/(double)bm->Icount() << endl;
    *(bm->_outfile) << "References: " << dec << bm->References() << " Predicts: " << dec << bm->Predicts() << endl;
}

VOID TWOLEVEL::Count(VOID *v)
{
    TWOLEVEL *bm = reinterpret_cast<TWOLEVEL*>(v);
    bm->_icount++;
}

