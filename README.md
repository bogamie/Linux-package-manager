# Linux Package Manager TUI

<div align="center">

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Language](https://img.shields.io/badge/language-C-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux-orange.svg)

**Terminal Package Manager for Debian/Ubuntu**

[Introduction](#introduction) • [Features](#features) • [Installation](#installation) • [Usage](#usage) • [Screenshots](#screenshots) • [Security](#security) • [Architecture](#architecture)

</div>

---

## 📋 Introduction <a name="introduction"></a>

**Linux Package Manager TUI** is a terminal-based UI application for managing system packages safely and efficiently on Debian/Ubuntu systems.
Built in **C** using **ncurses**, it is lightweight, responsive, and secure — combining Vim-style navigation with strict whitelist-based input validation.

**Key Highlights**

* Lightweight design: simple, stable behavior with `-O2` optimized C code
* Security-first: input validation preventing command injection
* Usability: Vim-style keybindings (j/k, gg/G, /, i, d, u)
* Modular architecture: separation of UI / logic / utilities following SOLID principles

---

## ✨ Features <a name="features"></a>

### Core Capabilities

* 📦 Package management — install, remove, show details, and run system update/upgrade
* 🔍 Smart search — apt-cache–based package lookup and filtering
* 🎨 Modern TUI — clean ncurses interface, adaptive layout, smooth pagination

### Technical Highlights

* ⚙️ Efficient build with `-O2` optimization
* 🔒 Whitelist-based input validation (prevents shell injection)
* 🧩 Modular structure separating UI / logic / utils / constants
* 💾 Centralized memory management and leak prevention
* 🛡️ Robust error handling across all execution paths

---

## 🚀 Installation <a name="installation"></a>

### Prerequisites (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev
gcc --version  # C11-compatible GCC recommended
```

### Build

```bash
git clone https://github.com/bogamie/Linux-package-manager.git
cd Linux-package-manager
make          # Release build (optimized)
./package_manager
```

### Build Options

```bash
make          # Standard build (-O2)
make debug    # Debug build (-g)
make clean    # Remove build artifacts
make rebuild  # Clean and rebuild
make help     # Show available targets
```

---

## 💻 Usage <a name="usage"></a>

### Quick Start

```bash
./package_manager         # Run
./package_manager -help   # Display help
./package_manager -U      # Auto update/upgrade (sudo required)
```

### Keyboard Shortcuts

| Key     | Action  | Description                   |
| ------- | ------- | ----------------------------- |
| `j`     | Down    | Move to next package          |
| `k`     | Up      | Move to previous package      |
| `gg`    | Top     | Jump to first entry           |
| `G`     | Bottom  | Jump to last entry            |
| `Enter` | Details | View package details          |
| `i`     | Install | Install selected package      |
| `d`     | Delete  | Remove selected package       |
| `u`     | Update  | Perform system update/upgrade |
| `/`     | Search  | Search for a package          |
| `q`     | Quit    | Exit program                  |

### Examples

**Install**

```
1) Run ./package_manager → 2) Move with j/k → 3) Press 'i' → 4) Enter sudo password
```

**Search**

```
1) Press '/' → 2) Enter package name (e.g. "vim") → 3) Navigate with j/k → 4) Press Enter for details
```

### Execution Options

| Option                | Description                                           |
| --------------------- | ----------------------------------------------------- |
| `-help`, `--help`     | Print help and exit                                   |
| `-U`, `--auto-update` | Automatically update & upgrade system (requires sudo) |

The default execution (`./package_manager`) does **not** require sudo.
Root privileges are only needed when performing **install (i)**, **remove (d)**, or **update (u)** actions.

---

## 🖼️ Screenshots <a name="screenshots"></a>

<div align="center">

![Main Package List](screenshot/Screenshot%20from%202025-01-14%2015-43-08.png)

| Install                                                              | Remove                                                              | Details                                                              |
| -------------------------------------------------------------------- | ------------------------------------------------------------------- | -------------------------------------------------------------------- |
| ![Install](screenshot/Screenshot%20from%202025-01-14%2015-47-03.png) | ![Remove](screenshot/Screenshot%20from%202025-01-14%2015-47-09.png) | ![Details](screenshot/Screenshot%20from%202025-01-14%2015-47-23.png) |

</div>

---

## 🔒 Security <a name="security"></a>

The application validates all user input using a **strict whitelist policy**, blocking any shell metacharacters, redirection symbols, quotes, or escape sequences.

**Allowed Characters**

* Alphanumeric: `a-z`, `A-Z`, `0-9`
* Special: `-`, `.`, `_`, `+`, `:`

**Blocked Examples**

* Metacharacters: `;`, `|`, `&`, `$`, `` ` ``
* Redirection: `<`, `>`, `>>`, `<<`
* Quotes: `'`, `"`
* Others: `()`, `\`

**Length Limits**

* Package name ≤ 255
* Search query ≤ 256
* Command buffer ≤ 512

---

## 🏗️ Architecture <a name="architecture"></a>

```
src/
├── main.c              # Entry point
├── package_manager.c   # Core logic (dpkg/apt integration)
├── package_manager.h   # Interface definition
├── ui.c                # ncurses rendering layer
├── ui.h                # UI interface
├── utils.c             # Utilities (validation, memory, etc.)
├── utils.h             # Utility interface
└── constants.h         # Constant definitions
```

**Design Principles**

* Separation of concerns (SoC) and SOLID-oriented modularity
* Unified error-handling paths and safe memory release

**Example Data Structure**

```c
typedef struct Package {
    char *name;        // Package name
    char *version;     // Version
    char *description; // Description
} Package;
```

---

## 🛠️ Technical Details <a name="technical-details"></a>

**Compile Flags**

```makefile
CFLAGS = -Wall -Wextra -O2 -std=c11
```

**Dependencies**

* ncurses, dpkg-query, apt/apt-get, apt-cache

**Memory & Error Handling**

* Centralized deallocation (`freePackages`), safe string duplication, NULL-safe free
* `popen/pclose` error handling, terminal size validation, graceful error recovery

---

## 📊 Performance Notes <a name="performance"></a>

* Performance varies with hardware, network speed, and number of installed packages.
* Searching can appear slow due to `apt-cache` behavior.
* Future improvements may include asynchronous search or enhanced caching.

---

## 🧪 Testing <a name="testing"></a>

**Manual Test Checklist**

* [x] List navigation, install/remove (sudo required)
* [x] System update/upgrade
* [x] Valid/invalid input validation (security)
* [x] Terminal resize handling
* [x] Memory leak test (valgrind)

**Security Test Example**

```bash
# Example: test; rm -rf /tmp/test  → should be rejected
```

---

## 📄 License <a name="license"></a>

MIT License © 2025 bogamie

---