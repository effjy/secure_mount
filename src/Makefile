CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`
TARGET = secure_mount_gtk3
SRCDIR = .
SOURCES = $(SRCDIR)/secure_mount_gtk3.c
PREFIX = /usr
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
ICONDIR = $(DATADIR)/icons/hicolor
DESKTOPDIR = $(DATADIR)/applications

# Icon sizes to install
ICON_SIZES = 16x16 22x22 24x24 32x32 48x48 64x64 128x128 256x256 512x512

# Colors for output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

all: $(TARGET)

$(TARGET): $(SOURCES)
	@echo "$(BLUE)Building Secure Mount GTK3...$(NC)"
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	@if [ -x $(TARGET) ]; then \
		echo "$(GREEN)✓ Build successful!$(NC)"; \
	else \
		echo "$(RED)✗ Build failed!$(NC)"; \
		exit 1; \
	fi

# Generate PNG icons from SVG
icons: secure_mount_gtk3.svg
	@echo "$(BLUE)Generating PNG icons from SVG...$(NC)"
	@mkdir -p icons
	@for size in $(ICON_SIZES); do \
		mkdir -p icons/$$size; \
		if command -v rsvg-convert >/dev/null 2>&1; then \
			rsvg-convert -w $${size%x*} -h $${size%x*} secure_mount_gtk3.svg -o icons/$$size/secure_mount_gtk3.png 2>/dev/null; \
		elif command -v convert >/dev/null 2>&1; then \
			convert -resize $${size%x*} secure_mount_gtk3.svg icons/$$size/secure_mount_gtk3.png 2>/dev/null; \
		else \
			echo "$(YELLOW)⚠ Neither rsvg-convert nor convert found. Please install librsvg2-bin or imagemagick$(NC)"; \
			break; \
		fi; \
		if [ -f icons/$$size/secure_mount_gtk3.png ]; then \
			echo "$(GREEN)  ✓ $$size$(NC)"; \
		else \
			echo "$(RED)  ✗ $$size$(NC)"; \
		fi; \
	done
	@echo "$(GREEN)Icon generation complete.$(NC)"

# Install with full Ubuntu MATE integration
install: $(TARGET) icons
	@echo "$(BLUE)Installing Secure Mount GTK3 with Ubuntu MATE integration...$(NC)"
	@echo ""
	
	# Check if running as root for global install
	@if [ "$(shell id -u)" != "0" ] && [ "$(DESTDIR)" = "" ]; then \
		echo "$(YELLOW)⚠ Warning: Global installation typically requires sudo$(NC)"; \
		echo "$(YELLOW)  Use 'sudo make install' or 'make install-local' for user installation$(NC)"; \
		echo ""; \
	fi
	
	# Install binary
	@echo "$(BLUE)Installing binary to $(DESTDIR)$(BINDIR)...$(NC)"
	@install -d $(DESTDIR)$(BINDIR) || { echo "$(RED)✗ Failed to create bindir$(NC)"; exit 1; }
	@install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/ || { echo "$(RED)✗ Failed to install binary$(NC)"; exit 1; }
	@echo "$(GREEN)✓ Binary installed$(NC)"
	
	# Install icons
	@echo "$(BLUE)Installing application icons...$(NC)"
	@icon_count=0; \
	for size in $(ICON_SIZES); do \
		if [ -f icons/$$size/secure_mount_gtk3.png ]; then \
			install -d $(DESTDIR)$(ICONDIR)/$$size/apps/ ; \
			install -m 644 icons/$$size/secure_mount_gtk3.png $(DESTDIR)$(ICONDIR)/$$size/apps/ && icon_count=$$((icon_count + 1)); \
		fi; \
	done; \
	# Install scalable SVG
	@echo "$(BLUE)Installing scalable SVG icon...$(NC)"
	@install -d $(DESTDIR)$(ICONDIR)/scalable/apps/ 
	@install -m 644 secure_mount_gtk3.svg $(DESTDIR)$(ICONDIR)/scalable/apps/ && echo "$(GREEN)✓ SVG icon installed$(NC)"
	
	# Fallback pixmap installation (using PNG for maximum compatibility with menus)
	@echo "$(BLUE)Installing fallback pixmap...$(NC)"
	@install -d $(DESTDIR)$(DATADIR)/pixmaps/
	@install -m 644 icons/64x64/secure_mount_gtk3.png $(DESTDIR)$(DATADIR)/pixmaps/secure_mount_gtk3.png
	
	# Install desktop file
	@echo "$(BLUE)Installing desktop file...$(NC)"
	@install -d $(DESTDIR)$(DESKTOPDIR) || { echo "$(RED)✗ Failed to create desktopdir$(NC)"; exit 1; }
	@install -m 644 secure_mount.desktop $(DESTDIR)$(DESKTOPDIR)/ || { echo "$(RED)✗ Failed to install desktop file$(NC)"; exit 1; }
	@sed -i 's|^Icon=.*|Icon=$(DATADIR)/pixmaps/secure_mount_gtk3.png|' $(DESTDIR)$(DESKTOPDIR)/secure_mount.desktop
	@echo "$(GREEN)✓ Desktop file installed with absolute icon path$(NC)"
	
	# Update desktop database
	@if command -v update-desktop-database >/dev/null 2>&1; then \
		echo "$(BLUE)Updating desktop database...$(NC)"; \
		update-desktop-database $(DESTDIR)$(DESKTOPDIR) && echo "$(GREEN)✓ Desktop database updated$(NC)" || echo "$(YELLOW)⚠ Desktop database update failed$(NC)"; \
	fi
	
	# Update icon cache
	@if command -v gtk-update-icon-cache >/dev/null 2>&1; then \
		echo "$(BLUE)Updating icon cache...$(NC)"; \
		gtk-update-icon-cache -f -t $(DESTDIR)$(ICONDIR) && echo "$(GREEN)✓ Icon cache updated$(NC)" || echo "$(YELLOW)⚠ Icon cache update failed$(NC)"; \
	fi
	
	@echo ""
	@echo "$(GREEN)✅ Installation complete!$(NC)"
	@echo "$(BLUE)📱 Secure Mount is now available in:$(NC)"
	@echo "   • Applications menu (System category)"
	@echo "   • Right-click context menu actions"
	@echo "   • Command line: secure_mount_gtk3"

# Local installation (for testing without sudo)
install-local: $(TARGET) icons
	@echo "$(BLUE)Installing Secure Mount GTK3 locally...$(NC)"
	@echo ""
	
	# Create local directories
	@mkdir -p $$HOME/.local/bin $$HOME/.local/share/icons/hicolor $$HOME/.local/share/applications 2>/dev/null || { echo "$(RED)✗ Failed to create local directories$(NC)"; exit 1; }
	
	# Install binary
	@echo "$(BLUE)Installing binary to $$HOME/.local/bin...$(NC)"
	@install $(TARGET) $$HOME/.local/bin/ 2>/dev/null && echo "$(GREEN)✓ Binary installed$(NC)" || { echo "$(RED)✗ Failed to install binary$(NC)"; exit 1; }
	
	# Install icons
	@echo "$(BLUE)Installing application icons...$(NC)"
	@icon_count=0; \
	for size in $(ICON_SIZES); do \
		if [ -f icons/$$size/secure_mount_gtk3.png ]; then \
			mkdir -p $$HOME/.local/share/icons/hicolor/$$size/apps/ 2>/dev/null; \
			install icons/$$size/secure_mount_gtk3.png $$HOME/.local/share/icons/hicolor/$$size/apps/ 2>/dev/null && icon_count=$$((icon_count + 1)); \
		fi; \
	done; \
	if [ $$icon_count -gt 0 ]; then \
		echo "$(GREEN)✓ $$icon_count icons installed$(NC)"; \
	else \
		echo "$(YELLOW)⚠ No icons installed (run 'make icons' first)$(NC)"; \
	fi
	
	# Install desktop file
	@echo "$(BLUE)Installing desktop file...$(NC)"
	@install secure_mount.desktop $$HOME/.local/share/applications/ 2>/dev/null && echo "$(GREEN)✓ Desktop file installed$(NC)" || { echo "$(RED)✗ Failed to install desktop file$(NC)"; exit 1; }
	
	# Update local databases
	@if command -v update-desktop-database >/dev/null 2>&1; then \
		echo "$(BLUE)Updating local desktop database...$(NC)"; \
		update-desktop-database $$HOME/.local/share/applications/ 2>/dev/null && echo "$(GREEN)✓ Desktop database updated$(NC)" || echo "$(YELLOW)⚠ Desktop database update failed$(NC)"; \
	fi
	
	@if command -v gtk-update-icon-cache >/dev/null 2>&1; then \
		echo "$(BLUE)Updating local icon cache...$(NC)"; \
		gtk-update-icon-cache -f -t $$HOME/.local/share/icons/hicolor 2>/dev/null && echo "$(GREEN)✓ Icon cache updated$(NC)" || echo "$(YELLOW)⚠ Icon cache update failed$(NC)"; \
	fi
	
	@echo ""
	@echo "$(GREEN)✅ Local installation complete!$(NC)"
	@echo # Full uninstall with menu cleanup
uninstall:
	@echo "$(BLUE)Removing Secure Mount GTK3 and menu integration...$(NC)"
	@echo ""
	
	# Check if running as root for global uninstall
	@if [ "$(shell id -u)" != "0" ] && [ "$(DESTDIR)" = "" ]; then \
		echo "$(YELLOW)⚠ Warning: Global uninstall typically requires sudo$(NC)"; \
		echo "$(YELLOW)  Use 'sudo make uninstall' or 'make uninstall-local' for user uninstall$(NC)"; \
		echo ""; \
	fi
	
	# Remove binary
	@if [ -f $(DESTDIR)$(BINDIR)/$(TARGET) ]; then \
		echo "$(BLUE)Removing binary from $(DESTDIR)$(BINDIR)...$(NC)"; \
		rm -f $(DESTDIR)$(BINDIR)/$(TARGET) && echo "$(GREEN)✓ Binary removed$(NC)" || echo "$(RED)✗ Failed to remove binary$(NC)"; \
	fi
	@# Clean up /usr/local as well in case of previous installations
	@if [ -f $(DESTDIR)/usr/local/bin/$(TARGET) ]; then \
		echo "$(BLUE)Removing leftover binary from /usr/local/bin...$(NC)"; \
		rm -f $(DESTDIR)/usr/local/bin/$(TARGET); \
	fi
	
	# Remove icons
	@echo "$(BLUE)Removing application icons...$(NC)"; \
	icon_count=0; \
	for size in $(ICON_SIZES); do \
		if [ -f $(DESTDIR)$(ICONDIR)/$$size/apps/secure_mount_gtk3.png ]; then \
			rm -f $(DESTDIR)$(ICONDIR)/$$size/apps/secure_mount_gtk3.png && icon_count=$$((icon_count + 1)); \
		fi; \
		if [ -f $(DESTDIR)/usr/local/share/icons/hicolor/$$size/apps/secure_mount_gtk3.png ]; then \
			rm -f $(DESTDIR)/usr/local/share/icons/hicolor/$$size/apps/secure_mount_gtk3.png; \
		fi; \
	done; \
	if [ -f $(DESTDIR)$(ICONDIR)/scalable/apps/secure_mount_gtk3.svg ]; then \
		rm -f $(DESTDIR)$(ICONDIR)/scalable/apps/secure_mount_gtk3.svg && icon_count=$$((icon_count + 1)); \
	fi; \
	if [ -f $(DESTDIR)/usr/local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg ]; then \
		rm -f $(DESTDIR)/usr/local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg; \
	fi; \
	rm -f $(DESTDIR)$(DATADIR)/pixmaps/secure_mount_gtk3.png 2>/dev/null; \
	rm -f $(DESTDIR)/usr/local/share/pixmaps/secure_mount_gtk3.png 2>/dev/null; \
	if [ $$icon_count -gt 0 ]; then \
		echo "$(GREEN)✓ $$icon_count icons removed$(NC)"; \
	else \
		echo "$(YELLOW)⚠ No icons found to remove$(NC)"; \
	fi
	
	# Remove desktop file
	@if [ -f $(DESTDIR)$(DESKTOPDIR)/secure_mount.desktop ]; then \
		echo "$(BLUE)Removing desktop file...$(NC)"; \
		rm -f $(DESTDIR)$(DESKTOPDIR)/secure_mount.desktop && echo "$(GREEN)✓ Desktop file removed$(NC)" || echo "$(RED)✗ Failed to remove desktop file$(NC)"; \
	fi
	@if [ -f $(DESTDIR)/usr/local/share/applications/secure_mount.desktop ]; then \
		rm -f $(DESTDIR)/usr/local/share/applications/secure_mount.desktop; \
	fi
	
	# Update databases
	@if command -v update-desktop-database >/dev/null 2>&1; then \
		echo "$(BLUE)Updating desktop database...$(NC)"; \
		update-desktop-database $(DESTDIR)$(DESKTOPDIR) && echo "$(GREEN)✓ Desktop database updated$(NC)" || echo "$(YELLOW)⚠ Desktop database update failed$(NC)"; \
		update-desktop-database $(DESTDIR)/usr/local/share/applications 2>/dev/null; \
	fi
	
	@if command -v gtk-update-icon-cache >/dev/null 2>&1; then \
		echo "$(BLUE)Updating icon cache...$(NC)"; \
		gtk-update-icon-cache -f -t $(DESTDIR)$(ICONDIR) && echo "$(GREEN)✓ Icon cache updated$(NC)" || echo "$(YELLOW)⚠ Icon cache update failed$(NC)"; \
		gtk-update-icon-cache -f -t $(DESTDIR)/usr/local/share/icons/hicolor 2>/dev/null; \
	fi
	
	@# Aggressive cleanup of local installation if running as root via sudo
	@if [ "$(shell id -u)" = "0" ] && [ -n "$(SUDO_USER)" ]; then \
		USER_HOME=$$(getent passwd $(SUDO_USER) | cut -d: -f6); \
		if [ -d $$USER_HOME/.local ]; then \
			echo "$(BLUE)Cleaning up local installation for user '$(SUDO_USER)'...$(NC)"; \
			rm -f $$USER_HOME/.local/bin/$(TARGET) 2>/dev/null; \
			rm -f $$USER_HOME/.local/share/applications/secure_mount.desktop 2>/dev/null; \
			for size in $(ICON_SIZES); do \
				rm -f $$USER_HOME/.local/share/icons/hicolor/$$size/apps/secure_mount_gtk3.png 2>/dev/null; \
			done; \
			rm -f $$USER_HOME/.local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg 2>/dev/null; \
			if command -v update-desktop-database >/dev/null 2>&1; then \
				update-desktop-database $$USER_HOME/.local/share/applications/ 2>/dev/null; \
			fi; \
			echo "$(GREEN)✓ Local files for '$(SUDO_USER)' removed$(NC)"; \
		fi \
	fi
	
	@echo ""
	@echo "$(GREEN)✅ Global and local cleanup complete!$(NC)"

# Local uninstall
uninstall-local:
	@echo "$(BLUE)Removing local Secure Mount GTK3 installation...$(NC)"
	@echo ""
	
	# Remove binary
	@if [ -f $$HOME/.local/bin/$(TARGET) ]; then \
		echo "$(BLUE)Removing binary from $$HOME/.local/bin...$(NC)"; \
		rm -f $$HOME/.local/bin/$(TARGET) && echo "$(GREEN)✓ Binary removed$(NC)" || echo "$(RED)✗ Failed to remove binary$(NC)"; \
	else \
		echo "$(YELLOW)⚠ Binary not found at $$HOME/.local/bin/$(TARGET)$(NC)"; \
	fi
	
	# Remove icons
	@echo "$(BLUE)Removing application icons...$(NC)"; \
	icon_count=0; \
	for size in $(ICON_SIZES); do \
		if [ -f $$HOME/.local/share/icons/hicolor/$$size/apps/secure_mount_gtk3.png ]; then \
			rm -f $$HOME/.local/share/icons/hicolor/$$size/apps/secure_mount_gtk3.png && icon_count=$$((icon_count + 1)); \
		fi; \
	done; \
	if [ -f $$HOME/.local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg ]; then \
		rm -f $$HOME/.local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg && icon_count=$$((icon_count + 1)); \
	fi; \
	if [ $$icon_count -gt 0 ]; then \
		echo "$(GREEN)✓ $$icon_count icons removed$(NC)"; \
	else \
		echo "$(YELLOW)⚠ No icons found to remove$(NC)"; \
	fi
	
	# Remove desktop file
	@if [ -f $$HOME/.local/share/applications/secure_mount.desktop ]; then \
		echo "$(BLUE)Removing desktop file...$(NC)"; \
		rm -f $$HOME/.local/share/applications/secure_mount.desktop && echo "$(GREEN)✓ Desktop file removed$(NC)" || echo "$(RED)✗ Failed to remove desktop file$(NC)"; \
	else \
		echo "$(YELLOW)⚠ Desktop file not found$(NC)"; \
	fi
	
	# Update local databases
	@if command -v update-desktop-database >/dev/null 2>&1; then \
		echo "$(BLUE)Updating local desktop database...$(NC)"; \
		update-desktop-database $$HOME/.local/share/applications/ 2>/dev/null && echo "$(GREEN)✓ Desktop database updated$(NC)" || echo "$(YELLOW)⚠ Desktop database update failed$(NC)"; \
	fi
	
	@if command -v gtk-update-icon-cache >/dev/null 2>&1; then \
		echo "$(BLUE)Updating local icon cache...$(NC)"; \
		gtk-update-icon-cache -f -t $$HOME/.local/share/icons/hicolor 2>/dev/null && echo "$(GREEN)✓ Icon cache updated$(NC)" || echo "$(YELLOW)⚠ Icon cache update failed$(NC)"; \
	fi
	
	@echo ""
	@echo "$(GREEN)✅ Local uninstall complete!$(NC)"


# Enhanced clean with confirmation
clean:
	@echo "$(BLUE)Cleaning build artifacts...$(NC)"
	@rm -f $(TARGET) 2>/dev/null && echo "$(GREEN)✓ Binary removed$(NC)" || echo "$(YELLOW)⚠ No binary to remove$(NC)"
	@rm -rf icons/ 2>/dev/null && echo "$(GREEN)✓ Icons directory removed$(NC)" || echo "$(YELLOW)⚠ No icons directory to remove$(NC)"
	@echo "$(GREEN)Clean complete.$(NC)"

# Enhanced dependency checking
check-deps:
	@echo "$(BLUE)Checking dependencies...$(NC)"
	@echo ""
	@echo "$(YELLOW)Required packages:$(NC)"
	@pkg-config --exists gtk+-3.0 && echo "$(GREEN)✓ GTK3 development packages found$(NC)" || echo "$(RED)✗ GTK3 development packages not found$(NC)"
	@pkg-config --exists gtk+-3.0 || echo "   $(YELLOW)Install with: sudo apt-get install libgtk-3-dev$(NC)"
	@echo ""
	@echo "$(YELLOW)Optional packages for icon generation:$(NC)"
	@command -v rsvg-convert >/dev/null 2>&1 && echo "$(GREEN)✓ rsvg-convert found (librsvg2-bin)$(NC)" || echo "$(YELLOW)⚠ rsvg-convert not found (optional)$(NC)"
	@command -v rsvg-convert >/dev/null 2>&1 || echo "   $(YELLOW)Install with: sudo apt-get install librsvg2-bin$(NC)"
	@command -v convert >/dev/null 2>&1 && echo "$(GREEN)✓ ImageMagick convert found$(NC)" || echo "$(YELLOW)⚠ ImageMagick convert not found (optional)$(NC)"
	@command -v convert >/dev/null 2>&1 || echo "   $(YELLOW)Install with: sudo apt-get install imagemagick$(NC)"
	@echo ""
	@echo "$(YELLOW)Runtime dependencies:$(NC)"
	@command -v gocryptfs >/dev/null 2>&1 && echo "$(GREEN)✓ gocryptfs found$(NC)" || echo "$(RED)✗ gocryptfs not found$(NC)"
	@command -v gocryptfs >/dev/null 2>&1 || echo "   $(YELLOW)Install with: sudo apt-get install gocryptfs$(NC)"
	@command -v fusermount3 >/dev/null 2>&1 && echo "$(GREEN)✓ fusermount3 found$(NC)" || echo "$(YELLOW)⚠ fusermount3 not found$(NC)"
	@command -v fusermount >/dev/null 2>&1 && echo "$(GREEN)✓ fusermount found$(NC)" || echo "$(YELLOW)⚠ fusermount not found$(NC)"
	@command -v fusermount3 >/dev/null 2>&1 || command -v fusermount >/dev/null 2>&1 || echo "   $(YELLOW)Install with: sudo apt-get install fuse3$(NC)"
	@echo ""

# Enhanced test installation
test-install: install-local
	@echo ""
	@echo "$(BLUE)🧪 Testing local installation...$(NC)"
	@echo ""
	@if [ -x $$HOME/.local/bin/secure_mount_gtk3 ]; then \
		echo "$(GREEN)✓ Binary executable$(NC)"; \
	else \
		echo "$(RED)✗ Binary not executable$(NC)"; \
	fi
	@if [ -f $$HOME/.local/share/applications/secure_mount.desktop ]; then \
		echo "$(GREEN)✓ Desktop file installed$(NC)"; \
	else \
		echo "$(RED)✗ Desktop file missing$(NC)"; \
	fi
	@icon_count=0; \
	for size in $(ICON_SIZES); do \
		if [ -f $$HOME/.local/share/icons/hicolor/$$size/apps/secure_mount_gtk3.png ]; then \
			icon_count=$$((icon_count + 1)); \
		fi; \
	done; \
	if [ $$icon_count -gt 0 ]; then \
		echo "$(GREEN)✓ $$icon_count icons installed$(NC)"; \
	else \
		echo "$(YELLOW)⚠ No icons installed$(NC)"; \
	fi
	@echo ""
	@echo "$(BLUE)To test the application:$(NC)"
	@echo "   $(YELLOW)$$HOME/.local/bin/secure_mount_gtk3$(NC)"
	@echo "   $(YELLOW)Or find it in your applications menu under 'System' category$(NC)"
	@echo ""

# Quick install (build + install-local)
quick-install: all install-local
	@echo "$(GREEN)🚀 Quick install complete!$(NC)"

# Quick uninstall (uninstall-local)
quick-uninstall: uninstall-local
	@echo "$(GREEN)🗑️ Quick uninstall complete!$(NC)"

# Show installation status
status:
	@echo "$(BLUE)Secure Mount GTK3 Installation Status$(NC)"
	@echo "====================================$(NC)"
	@echo ""
	@if [ -x $$HOME/.local/bin/secure_mount_gtk3 ]; then \
		echo "$(GREEN)✓ Local installation found$(NC)"; \
		echo "   Binary: $$HOME/.local/bin/secure_mount_gtk3"; \
	else \
		echo "$(RED)✗ No local installation found$(NC)"; \
	fi
	@if [ -f $(DESTDIR)$(BINDIR)/secure_mount_gtk3 ]; then \
		echo "$(GREEN)✓ Global installation found$(NC)"; \
		echo "   Binary: $(DESTDIR)$(BINDIR)/secure_mount_gtk3"; \
	else \
		echo "$(RED)✗ No global installation found$(NC)"; \
	fi
	@if [ -f $$HOME/.local/share/applications/secure_mount.desktop ]; then \
		echo "$(GREEN)✓ Local desktop file installed$(NC)"; \
	fi
	@if [ -f $(DESTDIR)$(DESKTOPDIR)/secure_mount.desktop ]; then \
		echo "$(GREEN)✓ Global desktop file installed$(NC)"; \
	fi
	@echo ""

help:
	@echo "$(BLUE)Secure Mount GTK3 Version Makefile$(NC)"
	@echo "$(YELLOW)====================================$(NC)"
	@echo ""
	@echo "$(YELLOW)Build targets:$(NC)"
	@echo "  $(GREEN)all$(NC)           - Build the GTK3 application"
	@echo "  $(GREEN)icons$(NC)         - Generate PNG icons from SVG"
	@echo "  $(GREEN)clean$(NC)         - Remove build artifacts and generated icons"
	@echo ""
	@echo "$(YELLOW)Installation targets:$(NC)"
	@echo "  $(GREEN)install$(NC)       - Install globally with menu integration (requires sudo)"
	@echo "  $(GREEN)install-local$(NC) - Install for current user only"
	@echo "  $(GREEN)uninstall$(NC)     - Remove global installation"
	@echo "  $(GREEN)uninstall-local$(NC) - Remove local installation"
	@echo "  $(GREEN)quick-install$(NC) - Build and install locally"
	@echo "  $(GREEN)quick-uninstall$(NC) - Remove local installation"
	@echo ""
	@echo "$(YELLOW)Testing targets:$(NC)"
	@echo "  $(GREEN)test-install$(NC)  - Install locally and test installation"
	@echo "  $(GREEN)status$(NC)        - Show installation status"
	@echo "  $(GREEN)check-deps$(NC)    - Check for required and optional dependencies"
	@echo ""
	@echo "$(YELLOW)Utility targets:$(NC)"
	@echo "  $(GREEN)help$(NC)          - Show this help message"
	@echo ""
	@echo "$(YELLOW)Examples:$(NC)"
	@echo "  $(BLUE)make check-deps && make icons && make install-local$(NC)"
	@echo "  $(BLUE)make test-install$(NC)"
	@echo "  $(BLUE)make quick-install$(NC)"
	@echo "  $(BLUE)sudo make install$(NC)"
	@echo "  $(BLUE)sudo make uninstall$(NC)"
	@echo ""
	@echo "$(YELLOW)Quick start:$(NC)"
	@echo "  $(BLUE)make quick-install && secure_mount_gtk3$(NC)"

.PHONY: all icons install install-local uninstall uninstall-local clean check-deps test-install quick-install quick-uninstall status help
