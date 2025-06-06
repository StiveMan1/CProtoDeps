# 🧠 CPD — Compact Portable Data Library

Welcome to the world of CPD, a minimalistic (yet sneaky powerful) serialization library built in C. It’s what you reach for when you want to encode and decode structured data your way — compactly, efficiently, and with no strings attached (except actual strings — we do support those).

## 🗂️ Project Layout

Here's what you're working with:

```graphql
./
├── src/
│   ├── cpd.c         ← Core logic: serialization, trees, varints, and black magic
│   └── cpd.h         ← Header file for external use
├── main.c            ← Sample / entry-point if you want to test things
├── .gitignore        ← Keep your build junk out of git
├── CMakeLists.txt    ← CMake build setup
└── LICENSE           ← MIT, because freedom
```

## ⚙️ What’s Inside?

- **Varint encoding/decoding** like a boss
- **Red-black tree** for tracking object/position mappings (yes, memory-safe insertions and lookups!)
- **Basic type system** (ints, strings, references, compositions)
- **Marshalling and unmarshalling** contexts, so you don’t lose your head
- **Recursive serialization** — supports self-referencing and shared structures via links
- Error codes, macros, and some brutalist C to keep you sharp

## 🚀 Quick Start

Clone it. Build it with CMake. Run `main.c`. No runtime deps. No fluff.

```bash
mkdir build && cd build
cmake ..
make
./cpd-demo # Or whatever target name you gave it
```

## 🧪 Why?

Because sometimes you don’t need JSON, or Protobuf, or XML, or YAML, or MessagePack, or BSON, or Cap’n Proto, or Flatbuffers...\
You just want to move structured data around with full control and almost no overhead. That’s CPD.

## 🧩 Extend It

- Add your own object types via `cpd_marshal_func` and `cpd_unmarshal_func`
- Hook into tree logic if you need advanced reference tracking
- Plug in allocators, pools, or arena-style memory if you need performance

## 🧼 Known Things to Clean Up

- More docs (inline and otherwise)
- More examples (maybe a toy VM or config loader)
- Better error messages (CPD_FLAG_ERR_NULLPTR is... honest, at least)

## 📄 License

MIT — do what you want. Attribution appreciated, not required.
But if you build a rocket or AI with this, please name it something cool.
