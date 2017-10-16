#pragma once

#include "../Error.h"
#include "../Symbols.h"
#include "../SyntheticSections.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/Support/Endian.h"

namespace lld {
namespace elf {

// See CheriBSD crt_init_globals()
template <llvm::support::endianness E> struct InMemoryCapRelocEntry {
  using CapRelocUint64 = llvm::support::detail::packed_endian_specific_integral<
      uint64_t, E, llvm::support::aligned>;
  InMemoryCapRelocEntry(uint64_t Loc, uint64_t Obj, uint64_t Off, uint64_t S,
                        uint64_t Perms)
      : capability_location(Loc), object(Obj), offset(Off), size(S),
        permissions(Perms) {}
  CapRelocUint64 capability_location;
  CapRelocUint64 object;
  CapRelocUint64 offset;
  CapRelocUint64 size;
  CapRelocUint64 permissions;
};

struct SymbolAndOffset {
  SymbolAndOffset(SymbolBody *S, uint64_t O) : Symbol(S), Offset(O) {}
  SymbolAndOffset(const SymbolAndOffset &) = default;
  SymbolAndOffset &operator=(const SymbolAndOffset &) = default;
  SymbolBody *Symbol = nullptr;
  uint64_t Offset = 0;

  // for __cap_relocs against local symbols clang emits section+offset instead
  // of the local symbol so that it still works even if the local symbol table
  // is stripped. This function tries to find the local symbol to a better match
  template <typename ELFT> SymbolAndOffset findRealSymbol() const;
};

struct CheriCapRelocLocation {
  SymbolAndOffset Loc;
  bool NeedsDynReloc;
  bool operator==(const CheriCapRelocLocation &Other) const {
    return Loc.Symbol == Other.Loc.Symbol && Loc.Offset == Other.Loc.Offset &&
           NeedsDynReloc == Other.NeedsDynReloc;
  }
};

struct CheriCapReloc {
  // We can't use a plain SymbolBody* here as capabilities to string constants
  // will be e.g. `.rodata.str + 0x90` -> need to store offset as well
  SymbolAndOffset Target;
  int64_t CapabilityOffset;
  bool NeedsDynReloc;
};

template <class ELFT> class CheriCapRelocsSection : public SyntheticSection {
public:
  CheriCapRelocsSection();
  static constexpr size_t RelocSize = 40;
  // Add a __cap_relocs section from in input object file
  void addSection(InputSectionBase *S);
  bool empty() const override { return RelocsMap.empty(); }
  size_t getSize() const override { return RelocsMap.size() * Entsize; }
  void finalizeContents() override;
  void writeTo(uint8_t *Buf) override;
  void addCapReloc(const SymbolAndOffset &Location, bool LocNeedsDynReloc,
                   const SymbolAndOffset &Target, bool TargetNeedsDynReloc,
                   int64_t CapabilityOffset);

private:
  void processSection(InputSectionBase *S);
  // map or vector?
  llvm::MapVector<CheriCapRelocLocation, CheriCapReloc> RelocsMap;
  bool addEntry(CheriCapRelocLocation Loc, CheriCapReloc Relocation) {
    auto It = RelocsMap.insert(std::make_pair(Loc, Relocation));
    if (!It.second) {
      // Maybe happens with vtables?
      error("Symbol already added to cap relocs");
      return false;
    }
    return true;
  }
  // TODO: list of added dynamic relocations?
};

} // namespace elf
} // namespace lld

namespace llvm {
template <> struct DenseMapInfo<lld::elf::CheriCapRelocLocation> {
  static inline lld::elf::CheriCapRelocLocation getEmptyKey() {
    return {{nullptr, 0}, false};
  }
  static inline lld::elf::CheriCapRelocLocation getTombstoneKey() {
    return {{nullptr, std::numeric_limits<uint64_t>::max()}, false};
  }
  static unsigned getHashValue(const lld::elf::CheriCapRelocLocation &Val) {
    auto Pair = std::make_pair(Val.Loc.Symbol, Val.Loc.Offset);
    return DenseMapInfo<decltype(Pair)>::getHashValue(Pair);
  }
  static bool isEqual(const lld::elf::CheriCapRelocLocation &LHS,
                      const lld::elf::CheriCapRelocLocation &RHS) {
    return LHS == RHS;
  }
};
} // namespace llvm
