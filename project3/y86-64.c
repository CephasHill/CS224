#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

const int MAX_MEM_SIZE  = (1 << 13);

bool checkAddOverflow(wordType A, wordType B, wordType S) {
  if (A > 0 && B > 0 && S <= 0) {
    return TRUE;
  }
  else if (A < 0 && B < 0 && S >= 0) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

bool checkSubOverflow(wordType A, wordType B, wordType S) {
  if (B > 0 && A < 0 && S <= 0) {
    return TRUE;
  }
  else if (B < 0 && A > 0 && S >= 0) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

void fetchStage(int *icode, int *ifun, int *rA, int *rB, wordType *valC, wordType *valP) {
  wordType PC = getPC();
  byteType valPByte = getByteFromMemory(PC);
  *ifun = valPByte & 0xF;
  *icode = (valPByte >> 4) & 0xF;

  if (
      *icode == HALT ||
      *icode == NOP ||
      *icode == RET) {
    *valP = PC + 1;
  }
  else if (
      *icode == CMOVXX ||
      *icode == OPQ ||
      *icode == PUSHQ ||
      *icode == POPQ) {
    byteType rArBByte = getByteFromMemory(PC+1);
    *rB = rArBByte & 0xF;
    *rA = (rArBByte >> 4) & 0xF;
    *valP = PC + 2;
  }
  else if (
      *icode == IRMOVQ ||
      *icode == RMMOVQ ||
      *icode == MRMOVQ) {
    byteType rArBByte = getByteFromMemory(PC+1);
    *rB = rArBByte & 0xF;
    *rA = (rArBByte >> 4) & 0xF;
    *valC = getWordFromMemory(PC+2);
    *valP = PC + 10;
  }
  else if (
      *icode == JXX ||
      *icode == CALL) {
    *valC = getWordFromMemory(PC+1);
    *valP = PC + 9;
  }
}

void decodeStage(int icode, int rA, int rB, wordType *valA, wordType *valB) {
  if (icode == CMOVXX) {
    *valA = getRegister(rA);
  }
  else if (icode == MRMOVQ) {
    *valB = getRegister(rB);
  }
  else if (
      icode == RMMOVQ ||
      icode == OPQ) {
    *valA = getRegister(rA);
    *valB = getRegister(rB);
  }
  else if (icode == CALL) {
    *valB = getRegister(RSP);
  }
  else if (
      icode == RET ||
      icode == POPQ) {
    *valA = getRegister(RSP);
    *valB = getRegister(RSP);
  }
  else if (icode == PUSHQ) {
    *valA = getRegister(rA);
    *valB = getRegister(RSP);
  }
}

void executeStage(int icode, int ifun, wordType valA, wordType valB, wordType valC, wordType *valE, bool *Cnd) {
  if (icode == HALT) {
    setStatus(STAT_HLT);
  }
  else if (icode == CMOVXX) {
    *valE = valA;
    *Cnd = Cond(ifun);
  }
  else if (icode == IRMOVQ) {
    *valE = valC;
  }
  else if (
      icode == RMMOVQ ||
      icode == MRMOVQ) {
    *valE = valB + valC;
  }
  else if (icode == OPQ) {
    // operations
    if (ifun == ADD) {
      *valE = valB + valA;
    }
    else if (ifun == SUB) {
      *valE = valB - valA;
    }
    else if (ifun == AND) {
      *valE = valB & valA;
    }
    else if (ifun == XOR) {
      *valE = valB ^ valA;
    }
    // flags
    bool sf_flag = FALSE;
    bool zf_flag = FALSE;
    bool of_flag = FALSE;

    if (*valE == 0) {
      zf_flag = TRUE;
    }
    if (*valE < 0) {
      sf_flag = TRUE;
    }
    if (ifun == ADD || ifun == SUB) {
      if (ifun == ADD) {
        of_flag = checkAddOverflow(valA, valB, *valE);
      }
      else if (ifun == SUB) {
        of_flag = checkSubOverflow(valA, valB, *valE);
      }
    }
    setFlags(sf_flag, zf_flag, of_flag);
  }
  else if (icode == JXX) {
    *Cnd = Cond(ifun);
  }
  else if (
      icode == CALL ||
      icode == PUSHQ) {
    *valE = valB - 8;
  }
  else if (
      icode == RET ||
      icode == POPQ) {
    *valE = valB + 8;
  }
}

void memoryStage(int icode, wordType valA, wordType valP, wordType valE, wordType *valM) {
  if (icode == MRMOVQ) {
    *valM = getWordFromMemory(valE);
  }
  else if (
      icode == RMMOVQ ||
      icode == PUSHQ) {
    setWordInMemory(valE, valA);
  }
  else if (icode == CALL) {
    setWordInMemory(valE, valP);
  }
  else if (
      icode == RET ||
      icode == POPQ) {
    *valM = getWordFromMemory(valA);
  }
}

void writebackStage(int icode, int rA, int rB, wordType valE, wordType valM, bool Cnd) {
  if (icode == CMOVXX) {
    if (Cnd) {
      setRegister(rB, valE);
    }
  }
  else if (
      icode == IRMOVQ ||
      icode == OPQ) {
    setRegister(rB, valE);
  }
  else if (icode == MRMOVQ) {
    setRegister(rA, valM);
  }
  else if (
      icode == CALL ||
      icode == RET ||
      icode == PUSHQ) {
    setRegister(RSP, valE);
  }
  else if (icode == POPQ) {
    setRegister(RSP, valE);
    setRegister(rA, valM);
  }
}

void pcUpdateStage(int icode, wordType valC, wordType valP, bool Cnd, wordType valM) {
  if (icode == HALT) {
    setPC(valP);
  }
  else if (
      icode == NOP ||
      icode == CMOVXX ||
      icode == IRMOVQ ||
      icode == RMMOVQ ||
      icode == MRMOVQ ||
      icode == OPQ ||
      icode == PUSHQ ||
      icode == POPQ) {
    setPC(valP);
  }
  else if (icode == JXX) {
    setPC(Cnd ? valC : valP);
  }
  else if (icode == CALL) {
    setPC(valC);
  }
  else if (icode == RET) {
    setPC(valM);
  }
}

void stepMachine(int stepMode) {
  /* FETCH STAGE */
  int icode = 0, ifun = 0;
  int rA = 0, rB = 0;
  wordType valC = 0;
  wordType valP = 0;
 
  /* DECODE STAGE */
  wordType valA = 0;
  wordType valB = 0;

  /* EXECUTE STAGE */
  wordType valE = 0;
  bool Cnd = 0;

  /* MEMORY STAGE */
  wordType valM = 0;

  fetchStage(&icode, &ifun, &rA, &rB, &valC, &valP);
  applyStageStepMode(stepMode, "Fetch", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  decodeStage(icode, rA, rB, &valA, &valB);
  applyStageStepMode(stepMode, "Decode", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  executeStage(icode, ifun, valA, valB, valC, &valE, &Cnd);
  applyStageStepMode(stepMode, "Execute", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  memoryStage(icode, valA, valP, valE, &valM);
  applyStageStepMode(stepMode, "Memory", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  writebackStage(icode, rA, rB, valE, valM, Cnd);
  applyStageStepMode(stepMode, "Writeback", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);
  
  pcUpdateStage(icode, valC, valP, Cnd, valM);
  applyStageStepMode(stepMode, "PC update", icode, ifun, rA, rB, valC, valP, valA, valB, valE, Cnd, valM);

  incrementCycleCounter();
}

/** 
 * main
 * */
int main(int argc, char **argv) {
  int stepMode = 0;
  FILE *input = parseCommandLine(argc, argv, &stepMode);

  initializeMemory(MAX_MEM_SIZE);
  initializeRegisters();
  loadMemory(input);

  applyStepMode(stepMode);
  while (getStatus() != STAT_HLT) {
    stepMachine(stepMode);
    applyStepMode(stepMode);
#ifdef DEBUG
    printMachineState();
    printf("\n");
#endif
  }
  printMachineState();
  return 0;
}