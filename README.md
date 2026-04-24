
## 🔒 secure_mount – A Menu‑Driven Secure Wrapper for gocryptfs


<div>
  <img src="https://img.shields.io/badge/language-C-blue" alt="Language: C">
  <img src="https://img.shields.io/badge/license-MIT-yellow" alt="License: MIT">
  <img src="https://img.shields.io/badge/tool-gocryptfs%20wrapper-orange" alt="Tool: gocryptfs wrapper">
  <img src="https://img.shields.io/badge/security-core%20dump%20disabled%20%7C%200700%20perms-brightgreen" alt="Security: core dump disabled | 0700 perms">
  <img src="https://img.shields.io/badge/menu-interactive%20TUI-success" alt="Menu: interactive TUI">
  <img src="https://img.shields.io/badge/status-stable-lightgrey" alt="Status: stable">
  <img src="https://img.shields.io/badge/platform-Linux-blue" alt="Platform: Linux">
</div>

---

### 🧭 What is secure_mount?

Hey, so `secure_mount` is a tiny, menu‑driven program that wraps around **gocryptfs**.  
You know how gocryptfs has all those command‑line flags? This tool hides them. You just pick an option from a simple menu, type your paths, and it does the rest – plus it adds a little bit of security hardening automatically.

No more remembering `-reverse`, `-allow_other`, or `-ro`. Just clean, interactive choices.

---

### ✨ Features – what you get

- 🧭 **Simple interactive menu** – three core operations, one key press each.
- 🔐 **gocryptfs integration** – mounts encrypted directories with a clean interface. All password prompts are still handled by gocryptfs itself (so your credentials never touch this wrapper).
- 🧹 **Clean initialisation** – when you set up a new vault, it creates the cipher directory with `0700` permissions, runs `gocryptfs -init`, and walks you through the master password creation.
- 🧲 **Smart unmounting** – tries `fusermount3 -u` first, then falls back to `fusermount -u`, and finally `umount` if needed. Works across different Linux distributions.
- 🛡️ **Security hardening** – disables core dumps using `prctl(PR_SET_DUMPABLE, 0)`. So even if the program crashes, no sensitive memory (like passwords) leaks to disk.
- 📁 **Path validation** – refuses to mount if the cipher directory and mount point are the same, checks that they exist, and auto‑creates the mount point with safe permissions (`0700`).
- 🪶 **Tiny footprint** – single C file. Compiles to a small static binary. No extra dependencies beyond a working gocryptfs and standard POSIX tools.

---

### 📦 Installation – how to get it running

**What you need** (requirements)
- **gocryptfs** installed and available in your `$PATH`.  
  (Check the [gocryptfs installation guide](https://github.com/rfjakob/gocryptfs#installation) if you don't have it yet.)
- **FUSE utilities** – they usually come with your Linux distribution.
- A C compiler – `gcc` or `clang`.

**Build it** (two easy steps)

```bash
git clone https://github.com/effjy/secure_mount.git
cd secure_mount
gcc secure_mount.c -o secure_mount -O2
```

If you want a fully static binary (good for moving between systems):  
`gcc secure_mount.c -o secure_mount -static -O2`

Then move it somewhere in your `PATH`, for example:
```bash
sudo cp secure_mount /usr/local/bin/
```

**Run it**  
Just type `./secure_mount` (or `secure_mount` if it's in your PATH). You'll see a menu like this:

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

### 🎮 Usage – how to drive it

**1. Initialise a new encrypted filesystem** (option 3)  
- Choose `3` from the menu.  
- Enter the path where the encrypted data will live (example: `/home/user/vault/encrypted`).  
- `secure_mount` creates that directory with `0700` permissions and then calls `gocryptfs -init`.  
- Follow the on‑screen prompts from gocryptfs to set (and confirm) a master password.  
- Your encrypted volume is now ready to be mounted.

**2. Mount an existing volume** (option 1)  
- Choose `1`.  
- Type the **cipher directory** – that's the folder containing `gocryptfs.diriv` and the encrypted files.  
- Type the **mount point** – the empty folder where you want to see the decrypted files.  
- If the mount point doesn't exist, the program creates it automatically with `0700` permissions.  
- Then it runs `gocryptfs <cipher> <mount>`. You'll be prompted by gocryptfs for the master password.  
- Done – your files appear at the mount point.

**3. Unmount a volume** (option 2)  
- Choose `2`.  
- Enter the mount point path (the one you used earlier).  
- `secure_mount` tries to unmount with `fusermount3 -u`, then `fusermount -u`, then `umount`.  
- The volume is safely detached.

---

### 🔐 Security considerations (the important bit)

- **Core dumps disabled** – Right at the start, the program calls `prctl(PR_SET_DUMPABLE, 0)`. So even if it crashes, no sensitive information (like your master password) ends up in a core file on disk.
- **No stored credentials** – `secure_mount` never sees or remembers your passwords. It hands everything off to gocryptfs, which handles authentication securely.
- **Single‑purpose design** – Tiny codebase means a small attack surface. No network services, no background daemons.
- **Safe directory creation** – Mount points and cipher directories are created with `0700` permissions (read/write/execute for owner only). No accidental exposure to other users.
- **Path collision prevention** – If you accidentally try to use the same directory as both cipher and mount point, the program refuses – no weird self‑overwrites.

> ⚠️ **Important reminder**: `secure_mount` is just a convenience wrapper. The real encryption strength comes from gocryptfs itself. Keep gocryptfs updated and always use a strong master passphrase.

---

### 🛠️ Error handling – what happens when something goes wrong

- Any fork/exec failure prints an error to `stderr` (so you know what broke).
- If `gocryptfs` itself fails – wrong password, missing directory, whatever – its exit code is shown.
- The unmount routine tries multiple tools (`fusermount3`, `fusermount`, `umount`) to work on different distributions (Arch, Debian, Fedora, etc.). If none works, you'll see a clear error.

---

### 🤝 Contributing – want to help?

Yes, please! Open an issue or a pull request at the [GitHub repo](https://github.com/effjy/secure_mount).

Some ideas for improvements:
- Support `cryfs` as an alternative backend.
- Add a read‑only mount option (`gocryptfs -ro`).
- Use a config file to remember frequently used cipher/mount pairs.

---

### 📜 License

MIT License. See the [LICENSE](LICENSE) file for details.

---

### 🙏 Acknowledgements

- Huge thanks to [gocryptfs](https://github.com/rfjakob/gocryptfs) – the fantastic encrypted filesystem that makes this possible.
- Everyone who helps keep secure‑by‑default tools accessible.

---

<p align="center">
  <strong>Encrypt your data with confidence. Secure_mount makes gocryptfs effortless.</strong><br>
  <sub>Built with ❤️ by <a href="https://github.com/effjy">effjy</a></sub>
</p>
