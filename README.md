```markdown
# 🔒 secure_mount

<div align="center">

![Language](https://img.shields.io/badge/language-C-green)
![License](https://img.shields.io/badge/license-MIT-yellow)
[![GitHub stars](https://img.shields.io/github/stars/effjy/secure_mount?style=social)](https://github.com/effjy/secure_mount)

</div>

**`secure_mount` is a menu‑driven, secure wrapper for gocryptfs that streamlines the initialization, mounting, and unmounting of encrypted filesystems.**  
No more memorising command‑line flags – just pick an option, enter your paths, and let it handle the rest, all while enforcing basic security hardening.

---

## ✨ Features

- 🧭 **Simple interactive menu** – 3 core operations accessible with a single key press.
- 🔐 **gocryptfs integration** – Mounts gocryptfs‑encrypted directories with a clean interface; all password prompts are handled by gocryptfs itself.
- 🧹 **Clean initialisation** – Creates the cipher directory with `0700` permissions, runs `gocryptfs -init`, and walks you through the master password creation.
- 🧲 **Smart unmounting** – Tries `fusermount3`, falls back to `fusermount`, and even tries `umount` if needed – making it resilient across different systems.
- 🛡️ **Security hardening** – Disables core dumps (`prctl(PR_SET_DUMPABLE, 0)`) to prevent sensitive memory from leaking to disk.
- 📁 **Path validation** – Refuses to mount if the cipher and mount point are the same directory, checks for existence, and auto‑creates the mount point with safe permissions.
- 🪶 **Tiny footprint** – Single C file, compiles to a small, static binary; no additional dependencies beyond a working gocryptfs and standard POSIX tools.

---

## 📦 Installation

### Requirements

- **gocryptfs** installed and available in `$PATH`  
  ([Installation guide](https://github.com/rfjakob/gocryptfs#installation))
- **FUSE** utilities (should come with your Linux distribution)
- A C compiler (`gcc` or `clang`)

### Build

```bash
git clone https://github.com/effjy/secure_mount.git
cd secure_mount
gcc secure_mount.c -o secure_mount -O2
# Optional: create a statically linked version
gcc secure_mount.c -o secure_mount -static -O2
```

Move the binary to a location in your `PATH` for easy access:

```bash
sudo cp secure_mount /usr/local/bin/
```

### Run

```bash
./secure_mount
```

The menu will appear:

```
========================================
       Secure gocryptfs Manager
========================================
 [1] Mount encrypted partition
 [2] Unmount encrypted partition
 [3] Initialize new encrypted partition
 [4] Exit
========================================
Select an option:
```

---

## 🎮 Usage

### 1. Initialise a new encrypted filesystem

- Choose **option 3**.
- Enter the path where encrypted data will be stored (e.g., `/home/user/vault/encrypted`).
- `secure_mount` creates the directory with `0700` permissions and calls `gocryptfs -init`.
- Follow the on‑screen gocryptfs prompts to set and confirm a master password.
- Your encrypted volume is ready to be mounted.

### 2. Mount an existing volume

- Choose **option 1**.
- Provide the **cipher directory** (the folder containing `gocryptfs.diriv` and encrypted files).
- Provide the **mount point** (the folder where you want to see the decrypted files).
- If the mount point doesn’t exist, it will be created automatically with `0700` permissions.
- The program then executes `gocryptfs <cipher> <mount>`. You’ll be prompted by gocryptfs for the master password.

### 3. Unmount a volume

- Choose **option 2**.
- Enter the mount point path.
- `secure_mount` attempts to unmount using `fusermount3 -u`, falls back to `fusermount -u`, and finally `umount` if necessary.
- The volume is safely detached.

---

## 🔐 Security Considerations

- **Core dumps disabled** – The process calls `prctl(PR_SET_DUMPABLE, 0)` immediately, so even if the program crashes, no sensitive information (like the master password used by gocryptfs) can leak into a core file.
- **No stored credentials** – `secure_mount` never sees or stores your passwords; it delegates all authentication to gocryptfs, which handles it securely.
- **Single‑purpose design** – The minimal codebase reduces attack surface. There are no network services, no background daemons.
- **Safe directory creation** – Mount points and cipher directories are created with `0700` permissions (owner‑read/write/execute only).
- **Path collision prevention** – The program refuses to operate if the cipher directory and mount point are identical, preventing accidental data exposure.

> ⚠️ **Important**: `secure_mount` is a convenience wrapper; the actual encryption strength comes from gocryptfs. Keep gocryptfs updated and use a strong passphrase.

---

## 🛠️ Error Handling

- All fork/exec failures print errors to `stderr`.
- If `gocryptfs` itself fails (wrong password, missing directory, etc.), its exit code is propagated and printed.
- The unmount routine tries multiple tools to accommodate different distributions (Arch uses `fusermount3`, Debian may prefer `fusermount`, some require `umount`).

---

## 🤝 Contributing

Contributions are welcome! Feel free to open an issue or a pull request at  
[https://github.com/effjy/secure_mount](https://github.com/effjy/secure_mount).

Ideas for enhancements:
- Integration with `cryfs` as an alternative backend.
- Adding support for `gocryptfs -ro` (read‑only mount).
- Configuration file to remember frequently used cipher/mount pairs.

---

## 📜 License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgements

- [gocryptfs](https://github.com/rfjakob/gocryptfs) – the fantastic encrypted filesystem that makes this possible.
- Everyone who helps keep secure‑by‑default tools accessible.

---

<p align="center">
  <strong>Encrypt your data with confidence. Secure_mount makes gocryptfs effortless.</strong><br>
  <sub>Built with ❤️ by <a href="https://github.com/effjy">effjy</a></sub>
</p>
```
