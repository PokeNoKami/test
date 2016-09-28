/**
  * @file func.h
  * @brief Functions for iv-pid
  */

#ifndef __FUNC_H__
#define __FUNC_H__

#include <stdint.h> // so PID, RNG, ID, etc get the proper integer size

/**
  * @param state Current state of the RNG
  * @return The random number from the next RNG state
  * 
  * Moves the \p state one step forward and returns the
  * next random number, just like the in-game RNG does.
  */
uint16_t RNG(uint32_t &state);

/**
  * @param state Current state of the RNG
  * @return The random number from the previous RNG state
  * 
  * Moves the \p state one step back and returns the
  * random number that would be returned if the in-game
  * RNG reaches that previous state.
  */
uint16_t antiRNG(uint32_t &state);

/**
  * @brief Checks if a random number generates high enough IVs
  * @param n        Random number from the RNG
  * @param hp,at,df Minimum wanted HP, Attack, Defense IV
  * @return The IVs generated by n are at least hp, at, df each
  */
bool minIVtest(uint16_t n, int hp, int at, int df);

/**
  * @brief Checks if a random number generates some IVs
  * @param n        Random number from the RNG
  * @param hp,at,df Wanted HP, Attack, Defense IV
  * @return The IVs generated by n are exactly hp, at, df
  */
bool exactIVtest(uint16_t n, int hp, int at, int df);

typedef bool (*IVtester)(uint16_t, int, int, int);

/**
  * @param exact true if exact IVs are wanted, false otherwise
  * @return the wanted IV test function
  * @sa exactIVtest, minIVtest
  */
IVtester GetIVtester(bool exact);

/**
  * @brief Checks if the PID has the wanted nature and ability
  * @param pid     PID
  * @param nature  Wanted nature ID, or -1 if any
  * @param ability Wanted ability ID, or 2 if any
  * @return Whether the PID has the wanted nature and ability
  */
bool PIDtest(uint32_t pid, int nature, int ability);

/**
  * @brief Checks if a desired Hidden Power is possible from half of the IVs
  * @param iv  The RNG number that will throw the IVs 
  * @param hpt Wanted HP type ID, or -1 if any
  * @param hpp Minimum wanted HP base power, or -1 if any
  * @return Whether the wanted Hidden Power is possible
  * @sa HPtest
  */
bool HPpretest(uint16_t iv, int hpt, int hpp);

/**
  * @brief Checks if Hidden Power from some IVs has the wanted type and power
  * @param iv1 RNG number that holds the HP, Attack and Defense IVs
  * @param iv2 RNG number that led to the other IVs
  * @param hpt Wanted HP type ID, or -1 if any
  * @param hpp Minimum wanted HP base power, or -1 if any
  * @return Whether the Hidden Power matches the conditions
  */
bool HPtest(uint16_t iv1, uint16_t iv2, int hpt, int hpp);

/**
  * @brief Checks shininess
  * @param pid_l,pid_h Low and high 16-bits of PID
  * @param IDxorSID    (ID xor SID) and not 7. 1 if this test is not needed 
  * @return Whether the shiny check is passed
  */
bool XORtest(uint16_t pid_l, uint16_t pid_h, uint16_t IDxorSID);

/**
  * A structure to hold desired properties for a Pok�mon.
  * It will further filter the results of the IV to PID method
  */
struct PokeData {
  int hp, at, df,
      spa, spd, spe;  // Minimum or exact HP, Attack and Defense IVs
  int nature;         // Nature ID, or -1 if any
  int ability;        // Ability ID, or 2 if any
  int hp_type;        // Hidden Power type ID, or -1 if any
  int hp_power;       // Hidden Power minimum power, or -1 if any
  uint16_t IDxorSID;  // Trainer ID xor Trainer Secret ID

  PokeData()
    :nature(-1), ability(2), hp_type(-1), hp_power(-1), IDxorSID(1)
    { hp = at = df = spa = spd = spe = 0; }
};

/**
  * @brief Gets the PID for a state and prints if it matches any wanted pdata
  * @param seed   RNG state after the first call that has nothing to do with PID
  * @param iv1    RNG number that holds the HP, Attack and Defense IVs
  * @param iv2    RNG number that led the other IVs
  * @param pdata  Data to match
  * @param method Chosen method
  * @param count  Number of results so far
  * 
  * @note two PIDs might be found if both the PID for seed and the PID for
  * seed xor 0x8000 0000 match the conditions.
  */
void FindPID(uint32_t seed, uint16_t iv1, uint16_t iv2, const PokeData& pdata, int method, int &count);

/**
  * @brief Explores all possible seeds that might lead to results
  * @param pdata  IVs and data to match
  * @param gba    Allowed methods (0 = only NDS, 1 = NDS and common GBA, 2 = all, -1 = chained shiny)
  * @param exact  true if exact IVs are wanted, false otherwise
  * @param count  Number of results so far
  * 
  * @note Only seeds lower than 0x8000 0000 are explored.
  */
void TestAllPossibleSeeds(const PokeData& pdata, int gba, bool exact, int& count);

/**
  * @brief Gets the PID for a chained shiny and prints if it matches any wanted pdata
  * @param seed   RNG state after the first call that has nothing to do with PID
  * @param iv1    RNG number that holds the HP, Attack and Defense IVs
  * @param iv2    RNG number that led the other IVs
  * @param pdata  Data to match
  * @param count  Number of results so far
  */
void FindChainedPID(uint32_t seed, int iv1, int iv2, const PokeData& pdata, int &count);

/**
  * @brief Test if a given RNG state leads to wanted IVs and data
  * @param seed   RNG state after the last RNG call (which led to the last IVs)
  * @param pdata  IVs and data to match
  * @param gba    Allowed methods (0 = only NDS, 1 = NDS and common GBA, 2 = all, -1 = chained shiny)
  * @param IVtest Reference to the function that tells whether the IVs are ok
  * @param count  Number of results so far
  * 
  * Explores all possible results from a particular RNG state in order to find
  * any results that match the wanted \p pdata.
  */
void Test(uint32_t seed, const PokeData& pdata, int gba, IVtester& IVtest, int &count);

/**
  * @brief Checks if a RNG state generates a given PID
  * @param state RNG state that led to the low PID bits
  * @param pid_h Wanted high PID bits
  * @note The low bits of the PID are supposed to
  * be correct. Only the high bits are checked
  */
bool HighPIDmatches(uint32_t state, uint16_t pid_h);


/**
  * @brief Get all possible PID and IV distributions from RNG state
  * @param seed  RNG state after the first RNG call (which led to the low bits of PID)
  * @param count Number of results found so far
  * @param gba   Allowed methods
  */
void GetFromSeed(uint32_t seed, int &count, int gba);

#endif