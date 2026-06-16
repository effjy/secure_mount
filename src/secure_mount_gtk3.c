/*
 * secure_mount_gtk3.c - GTK3 GUI version of secure gocryptfs manager
 * Compile: gcc secure_mount_gtk3.c -o secure_mount_gtk3 `pkg-config --cflags --libs gtk+-3.0`
 * Usage:   ./secure_mount_gtk3
 */

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <errno.h>
#include <time.h>

#define MAX_PATH 1024

typedef struct {
    GtkWidget *window;
    GtkWidget *cipher_entry;
    GtkWidget *mount_entry;
    GtkWidget *mount_button;
    GtkWidget *unmount_entry;
    GtkWidget *init_entry;
} SecureMountApp;

/* Expand shell paths like ~ to actual paths */
static int expand_path(const char *input, char *output, size_t output_size) {
    if (input[0] == '~') {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(output, output_size, "%s%s", home, input + 1);
            return 0;
        }
    }
    strncpy(output, input, output_size - 1);
    output[output_size - 1] = '\0';
    return 0;
}

static int run_cmd_interactive(const char *cmd, char *const argv[]) {
    /* For interactive commands like gocryptfs, we need a terminal */
    char *terminal_cmd = NULL;
    
    /* Find available terminal emulator */
    if (system("which mate-terminal >/dev/null 2>&1") == 0) {
        terminal_cmd = "mate-terminal";
    } else if (system("which gnome-terminal >/dev/null 2>&1") == 0) {
        terminal_cmd = "gnome-terminal";
    } else if (system("which xfce4-terminal >/dev/null 2>&1") == 0) {
        terminal_cmd = "xfce4-terminal";
    } else if (system("which xterm >/dev/null 2>&1") == 0) {
        terminal_cmd = "xterm";
    } else {
        return -1; /* No terminal found */
    }
    
    /* Build command string */
    char cmd_str[MAX_PATH * 4] = {0};
    strcat(cmd_str, cmd);
    
    for (int i = 1; argv[i] != NULL; i++) {
        strcat(cmd_str, " ");
        /* Escape spaces in arguments */
        if (strchr(argv[i], ' ')) {
            strcat(cmd_str, "\"");
            strcat(cmd_str, argv[i]);
            strcat(cmd_str, "\"");
        } else {
            strcat(cmd_str, argv[i]);
        }
    }
    
    /* Execute in terminal */
    char terminal_cmd_full[MAX_PATH * 8] = {0};
    snprintf(terminal_cmd_full, sizeof(terminal_cmd_full), 
             "%s --title \"Secure Mount - %s\" -- bash -c '%s; echo \"Press Enter to continue...\"; read'", 
             terminal_cmd, cmd, cmd_str);
    
    int ret = system(terminal_cmd_full);
    return WEXITSTATUS(ret);
}

static int run_cmd(const char *cmd, char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0) { perror("[-] fork failed"); return -1; }
    if (pid == 0) {
        prctl(PR_SET_DUMPABLE, 0);
        execvp(cmd, argv);
        perror("[-] exec failed");
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;
}

static void update_status(SecureMountApp *app, const char *message, gboolean show_progress) {
    /* Status bar removed as requested */
    (void)app;
    (void)message;
    (void)show_progress;
    while (gtk_events_pending()) gtk_main_iteration();
}

static void show_error_dialog(GtkWindow *parent, const char *title, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void show_info_dialog(GtkWindow *parent, const char *title, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* Helper to add a feature row to the about dialog */
static void add_feature_row(GtkWidget *box, const char *title, const char *desc, const char *icon_symbol) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(row), 10);
    
    GtkWidget *icon_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(icon_label), g_strdup_printf("<span size='xx-large'>%s</span>", icon_symbol));
    gtk_widget_set_valign(icon_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(row), icon_label, FALSE, FALSE, 0);
    
    GtkWidget *text_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    
    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), g_strdup_printf("<span weight='bold' size='large' color='#00ffcc'>%s</span>", title));
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0);
    gtk_box_pack_start(GTK_BOX(text_vbox), title_label, FALSE, FALSE, 0);
    
    GtkWidget *desc_label = gtk_label_new(desc);
    gtk_label_set_line_wrap(GTK_LABEL(desc_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(desc_label), 0.0);
    gtk_label_set_max_width_chars(GTK_LABEL(desc_label), 60);
    gtk_box_pack_start(GTK_BOX(text_vbox), desc_label, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(row), text_vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), row, FALSE, FALSE, 5);
    
    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_opacity(sep, 0.1);
    gtk_box_pack_start(GTK_BOX(box), sep, FALSE, FALSE, 0);
}

static void on_menu_about_activate(GtkMenuItem *menuitem, gpointer user_data) {
    (void)menuitem;
    SecureMountApp *app = (SecureMountApp *)user_data;
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "About Secure Mount",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close", GTK_RESPONSE_CLOSE,
        NULL
    );
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 600);
    
    /* Apply local dark theme to dialog */
    GtkCssProvider *cp = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp, "dialog { background-color: #0a0a0a; color: #e0e0e0; }", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(dialog), GTK_STYLE_PROVIDER(cp), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 20);
    gtk_container_add(GTK_CONTAINER(content_area), main_vbox);
    
    /* Header Section */
    GtkWidget *icon_image = NULL;
    GdkPixbuf *pb = NULL;
    
    /* Force absolute paths first to bypass broken system icon caches */
    const char *fallback_paths[] = {
        "/usr/share/pixmaps/secure_mount_gtk3.png",
        "/usr/share/pixmaps/secure_mount_gtk3.svg",
        "/usr/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg",
        "/usr/local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg",
        "secure_mount_gtk3.svg",
        "secure_mount_gtk3.png",
        NULL
    };
    
    for (int i = 0; fallback_paths[i] != NULL; i++) {
        pb = gdk_pixbuf_new_from_file_at_size(fallback_paths[i], 64, 64, NULL);
        if (pb) break;
    }
    
    /* Fallback to local user path if global paths fail */
    if (!pb) {
        char *local_path = g_strdup_printf("%s/.local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg", g_get_home_dir());
        pb = gdk_pixbuf_new_from_file_at_size(local_path, 64, 64, NULL);
        g_free(local_path);
    }
    
    if (pb) {
        icon_image = gtk_image_new_from_pixbuf(pb);
        g_object_unref(pb);
    } else {
        /* Ultimate fallback to theme or generic icon */
        icon_image = gtk_image_new_from_icon_name("secure_mount_gtk3", GTK_ICON_SIZE_DIALOG);
        if (!icon_image) {
            icon_image = gtk_image_new_from_icon_name("security-high", GTK_ICON_SIZE_DIALOG);
        }
    }
    gtk_box_pack_start(GTK_BOX(main_vbox), icon_image, FALSE, FALSE, 0);
    
    GtkWidget *name_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(name_label), "<span size='xx-large' weight='bold' color='white'>Secure Mount</span>");
    gtk_box_pack_start(GTK_BOX(main_vbox), name_label, FALSE, FALSE, 0);
    
    GtkWidget *ver_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(ver_label), "<span color='#aaaaaa'>Version 1.2.0 \xe2\x80\xa2 Encrypted Privacy</span>");
    gtk_box_pack_start(GTK_BOX(main_vbox), ver_label, FALSE, FALSE, 5);
    
    GtkWidget *sep_top = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_vbox), sep_top, FALSE, FALSE, 10);
    
    /* Features Section */
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled, -1, 270);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled, TRUE, TRUE, 0);
    
    GtkWidget *feature_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(scrolled), feature_box);
    
    add_feature_row(feature_box, "gocryptfs Integration", 
        "Seamlessly manages FUSE-based encrypted overlays using industry-standard AES-256-GCM.", "\xf0\x9f\x94\x92"); /* Lock */

    add_feature_row(feature_box, "Plausible Deniability", 
        "Supports hidden volumes and encrypted directory structures that reveal nothing without the key.", "\xf0\x9f\x91\xbb"); /* Ghost */

    add_feature_row(feature_box, "Automatic Mount Point Management", 
        "Handles directory creation and cleanup for mount points automatically.", "\xf0\x9f\x93\x82"); /* Folder */

    add_feature_row(feature_box, "Secure Credential Handling", 
        "Invokes terminal for password entry to ensure sensitive keys never touch the GUI memory.", "\xf0\x9f\x8c\x9f"); /* Star */

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void on_menu_quit_activate(GtkMenuItem *menuitem, gpointer user_data) {
    (void)menuitem;
    (void)user_data;
    gtk_main_quit();
}

static gboolean do_mount(gpointer data) {
    SecureMountApp *app = (SecureMountApp *)data;
    const char *cipher = gtk_entry_get_text(GTK_ENTRY(app->cipher_entry));
    const char *mount = gtk_entry_get_text(GTK_ENTRY(app->mount_entry));
    
    if (!cipher || strlen(cipher) == 0) {
        show_error_dialog(GTK_WINDOW(app->window), "Error", "Please enter a cipher directory path");
        return FALSE;
    }
    
    if (!mount || strlen(mount) == 0) {
        show_error_dialog(GTK_WINDOW(app->window), "Error", "Please enter a mount point path");
        return FALSE;
    }
    
    if (strcmp(cipher, mount) == 0) {
        show_error_dialog(GTK_WINDOW(app->window), "Error", 
                           "Cipher directory and mount point CANNOT be the same folder.\n\n"
                           "Use two separate directories. Example:\n"
                           "Cipher: ~/data/encrypted\n"
                           "Mount:  ~/data/decrypted");
        return FALSE;
    }
    
    char expanded_cipher[MAX_PATH];
    char expanded_mount[MAX_PATH];
    expand_path(cipher, expanded_cipher, sizeof(expanded_cipher));
    expand_path(mount, expanded_mount, sizeof(expanded_mount));
    
    struct stat st;
    if (stat(expanded_cipher, &st) != 0) {
        char error_msg[MAX_PATH * 2];
        snprintf(error_msg, sizeof(error_msg), 
                "Cipher directory '%s' does not exist.\n\nInitialize it first with option [3].", expanded_cipher);
        show_error_dialog(GTK_WINDOW(app->window), "Error", error_msg);
        return FALSE;
    }
    
    if (stat(expanded_mount, &st) != 0) {
        update_status(app, "Creating mount point...", TRUE);
        /* Create parent directories if needed */
        char *parent_dir = strdup(expanded_mount);
        char *last_slash = strrchr(parent_dir, '/');
        if (last_slash) {
            *last_slash = '\0';
            if (strlen(parent_dir) > 0) {
                char mkdir_cmd[MAX_PATH * 2];
                snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", parent_dir);
                if (system(mkdir_cmd) != 0) {
                    show_error_dialog(GTK_WINDOW(app->window), "Error", "Failed to create parent mount directory");
                    free(parent_dir);
                    return FALSE;
                }
            }
        }
        free(parent_dir);
        
        if (mkdir(expanded_mount, 0700) != 0) {
            char error_msg[MAX_PATH * 2];
            snprintf(error_msg, sizeof(error_msg), "Failed to create mount point directory '%s': %s", expanded_mount, strerror(errno));
            show_error_dialog(GTK_WINDOW(app->window), "Error", error_msg);
            return FALSE;
        }
    }
    
    update_status(app, "Opening terminal for gocryptfs password...", TRUE);
    
    char *argv[] = {"gocryptfs", expanded_cipher, expanded_mount, NULL};
    int ret = run_cmd_interactive("gocryptfs", argv);
    
    update_status(app, "Ready", FALSE);
    
    if (ret == 0) {
        show_info_dialog(GTK_WINDOW(app->window), "Success", "Encrypted volume mounted successfully!");
    } else {
        show_error_dialog(GTK_WINDOW(app->window), "Error", "Failed to mount encrypted volume");
    }
    
    return FALSE;
}

static gboolean do_unmount(gpointer data) {
    SecureMountApp *app = (SecureMountApp *)data;
    const char *mount = gtk_entry_get_text(GTK_ENTRY(app->unmount_entry));
    
    if (!mount || strlen(mount) == 0) {
        show_error_dialog(GTK_WINDOW(app->window), "Error", "Please enter a mount point path");
        return FALSE;
    }
    
    char expanded_mount[MAX_PATH];
    expand_path(mount, expanded_mount, sizeof(expanded_mount));
    
    update_status(app, "Unmounting...", TRUE);
    
    char *argv[] = {"fusermount3", "-u", expanded_mount, NULL};
    int ret = run_cmd("fusermount3", argv);
    if (ret != 0) {
        char *argv2[] = {"fusermount", "-u", expanded_mount, NULL};
        ret = run_cmd("fusermount", argv2);
    }
    if (ret != 0) {
        char *argv3[] = {"umount", expanded_mount, NULL};
        ret = run_cmd("umount", argv3);
    }
    
    update_status(app, "Ready", FALSE);
    
    if (ret == 0) {
        show_info_dialog(GTK_WINDOW(app->window), "Success", "Volume unmounted successfully!");
    } else {
        show_error_dialog(GTK_WINDOW(app->window), "Error", "Failed to unmount volume");
    }
    
    return FALSE;
}

static gboolean do_init(gpointer data) {
    SecureMountApp *app = (SecureMountApp *)data;
    const char *cipher = gtk_entry_get_text(GTK_ENTRY(app->init_entry));
    
    if (!cipher || strlen(cipher) == 0) {
        show_error_dialog(GTK_WINDOW(app->window), "Error", "Please enter a path for the new encrypted directory");
        return FALSE;
    }
    
    char expanded_cipher[MAX_PATH];
    expand_path(cipher, expanded_cipher, sizeof(expanded_cipher));
    
    struct stat st;
    if (stat(expanded_cipher, &st) == 0) {
        char error_msg[MAX_PATH * 2];
        snprintf(error_msg, sizeof(error_msg), "Directory '%s' already exists.", expanded_cipher);
        show_error_dialog(GTK_WINDOW(app->window), "Error", error_msg);
        return FALSE;
    }
    
    /* Create parent directories if needed */
    char *parent_dir = strdup(expanded_cipher);
    char *last_slash = strrchr(parent_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (strlen(parent_dir) > 0) {
            char mkdir_cmd[MAX_PATH * 2];
            snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p \"%s\"", parent_dir);
            if (system(mkdir_cmd) != 0) {
                show_error_dialog(GTK_WINDOW(app->window), "Error", "Failed to create parent directory");
                free(parent_dir);
                return FALSE;
            }
        }
    }
    free(parent_dir);
    
    if (mkdir(expanded_cipher, 0700) != 0) {
        char error_msg[MAX_PATH * 2];
        snprintf(error_msg, sizeof(error_msg), "Failed to create directory '%s': %s", expanded_cipher, strerror(errno));
        show_error_dialog(GTK_WINDOW(app->window), "Error", error_msg);
        return FALSE;
    }
    
    update_status(app, "Opening terminal for gocryptfs initialization...", TRUE);
    
    char *argv[] = {"gocryptfs", "-init", expanded_cipher, NULL};
    int ret = run_cmd_interactive("gocryptfs", argv);
    
    update_status(app, "Ready", FALSE);
    
    if (ret == 0) {
        show_info_dialog(GTK_WINDOW(app->window), "Success", "Encrypted volume initialized successfully!");
    } else {
        show_error_dialog(GTK_WINDOW(app->window), "Error", "Failed to initialize encrypted volume");
    }
    
    return FALSE;
}

static void on_mount_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SecureMountApp *app = (SecureMountApp *)data;
    g_idle_add(do_mount, app);
}

static void on_unmount_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SecureMountApp *app = (SecureMountApp *)data;
    g_idle_add(do_unmount, app);
}

static void on_init_clicked(GtkWidget *widget, gpointer data) {
    (void)widget;
    SecureMountApp *app = (SecureMountApp *)data;
    g_idle_add(do_init, app);
}

static void on_window_destroy(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
    gtk_main_quit();
}

static void create_gui(SecureMountApp *app) {
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Secure gocryptfs Manager");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 650, 500);
    gtk_window_set_position(GTK_WINDOW(app->window), GTK_WIN_POS_CENTER);

    /* Set program class to match desktop file */
    g_set_prgname("secure_mount_gtk3");
    
    /* Set taskbar icon using absolute fallbacks first to bypass broken themes */
    const char *fallback_paths[] = {
        "/usr/share/pixmaps/secure_mount_gtk3.png",
        "/usr/share/pixmaps/secure_mount_gtk3.svg",
        "/usr/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg",
        "/usr/local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg",
        "secure_mount_gtk3.svg",
        "secure_mount_gtk3.png",
        NULL
    };
    GdkPixbuf *pb_win = NULL;
    for (int i = 0; fallback_paths[i] != NULL; i++) {
        pb_win = gdk_pixbuf_new_from_file_at_size(fallback_paths[i], 64, 64, NULL);
        if (pb_win) break;
    }
    
    if (!pb_win) {
        char *local_path = g_strdup_printf("%s/.local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg", g_get_home_dir());
        pb_win = gdk_pixbuf_new_from_file_at_size(local_path, 64, 64, NULL);
        g_free(local_path);
    }
    
    if (pb_win) {
        gtk_window_set_icon(GTK_WINDOW(app->window), pb_win);
        g_object_unref(pb_win);
    } else {
        gtk_window_set_icon_name(GTK_WINDOW(app->window), "secure_mount_gtk3");
    }
    
    g_signal_connect(app->window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    /* Apply Premium Dark Theme CSS */
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "window { background-color: #0a0a0a; color: #e0e0e0; }"
        "frame { color: #00ffcc; font-weight: bold; border: 1px solid #333; }"
        "label { color: #e0e0e0; min-height: 0px; padding: 2px 0px; }"
        "button { background-color: #222; border: 1px solid #444; border-radius: 4px; padding: 2px 12px; color: white; min-height: 15px; }"
        "button:hover { background-color: #333; border-color: #00ffcc; }"
        "button:disabled { background-color: #111; color: #555; }"
        "entry { background-color: #1a1a1a; color: #00ffcc; border: 1px solid #333; min-height: 15px; padding: 4px; }"
        "menubar { background-color: #111; color: #e0e0e0; border-bottom: 1px solid #333; min-height: 22px; }"
        "menuitem { color: #e0e0e0; padding: 4px 12px; }"
        "menuitem:hover { background-color: #444; color: white; }",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), main_box);
    
    /* Menu Bar */
    GtkWidget *menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(main_box), menu_bar, FALSE, FALSE, 0);
    
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    
    GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_menu_quit_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);
    
    GtkWidget *about_item = gtk_menu_item_new_with_label("About");
    g_signal_connect(about_item, "activate", G_CALLBACK(on_menu_about_activate), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);

    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(content_box), 12);
    gtk_box_pack_start(GTK_BOX(main_box), content_box, TRUE, TRUE, 0);
    
    // Title with logo icon
    GtkWidget *title_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(title_hbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(title_hbox, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(content_box), title_hbox, FALSE, FALSE, 8);

    GdkPixbuf *pb_logo = NULL;
    for (int i = 0; fallback_paths[i] != NULL; i++) {
        pb_logo = gdk_pixbuf_new_from_file_at_size(fallback_paths[i], 48, 48, NULL);
        if (pb_logo) break;
    }
    if (!pb_logo) {
        char *local_path = g_strdup_printf("%s/.local/share/icons/hicolor/scalable/apps/secure_mount_gtk3.svg", g_get_home_dir());
        pb_logo = gdk_pixbuf_new_from_file_at_size(local_path, 48, 48, NULL);
        g_free(local_path);
    }

    GtkWidget *logo_image = NULL;
    if (pb_logo) {
        logo_image = gtk_image_new_from_pixbuf(pb_logo);
        g_object_unref(pb_logo);
    } else {
        logo_image = gtk_image_new_from_icon_name("secure_mount_gtk3", GTK_ICON_SIZE_DND);
        if (!logo_image) {
            logo_image = gtk_image_new_from_icon_name("security-high", GTK_ICON_SIZE_DND);
        }
    }

    if (logo_image) {
        gtk_widget_set_valign(logo_image, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(title_hbox), logo_image, FALSE, FALSE, 0);
    }

    GtkWidget *title_text_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_valign(title_text_vbox, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(title_hbox), title_text_vbox, FALSE, FALSE, 0);

    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), 
                         "<span size='xx-large' weight='bold' color='#00ffcc'>Secure Mount Manager</span>");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(title_text_vbox), title_label, FALSE, FALSE, 0);

    GtkWidget *subtitle_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(subtitle_label),
                         "<span size='medium' color='#aaaaaa'>GTK3 Hardened Interface</span>");
    gtk_widget_set_halign(subtitle_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(title_text_vbox), subtitle_label, FALSE, FALSE, 0);
    
    // Mount section
    GtkWidget *mount_frame = gtk_frame_new(" \xf0\x9f\x94\x90 Mount Encrypted Volume ");
    gtk_box_pack_start(GTK_BOX(content_box), mount_frame, FALSE, FALSE, 4);
    
    GtkWidget *mount_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(mount_box), 10);
    gtk_container_add(GTK_CONTAINER(mount_frame), mount_box);
    
    GtkWidget *cipher_label = gtk_label_new("\xf0\x9f\x93\x81 Cipher Directory (Encrypted):");
    gtk_widget_set_halign(cipher_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(mount_box), cipher_label, FALSE, FALSE, 0);
    
    app->cipher_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->cipher_entry), "~/my_encrypted_data");
    gtk_box_pack_start(GTK_BOX(mount_box), app->cipher_entry, FALSE, FALSE, 0);
    
    GtkWidget *mount_label = gtk_label_new("\xf0\x9f\x93\x82 Mount Point (Decrypted):");
    gtk_widget_set_halign(mount_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(mount_box), mount_label, FALSE, FALSE, 0);
    
    app->mount_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->mount_entry), "~/decrypted_data");
    gtk_box_pack_start(GTK_BOX(mount_box), app->mount_entry, FALSE, FALSE, 0);
    
    app->mount_button = gtk_button_new_with_label("\xf0\x9f\x94\x92 Mount Volume");
    gtk_widget_set_halign(app->mount_button, GTK_ALIGN_CENTER);
    g_signal_connect(app->mount_button, "clicked", G_CALLBACK(on_mount_clicked), app);
    gtk_box_pack_start(GTK_BOX(mount_box), app->mount_button, FALSE, FALSE, 4);
    
    // Unmount section
    GtkWidget *unmount_frame = gtk_frame_new(" \xe2\x8f\x8f Unmount Volume ");
    gtk_box_pack_start(GTK_BOX(content_box), unmount_frame, FALSE, FALSE, 4);
    
    GtkWidget *unmount_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(unmount_box), 10);
    gtk_container_add(GTK_CONTAINER(unmount_frame), unmount_box);
    
    GtkWidget *unmount_label = gtk_label_new("\xf0\x9f\x93\x8d Mount Point Path:");
    gtk_widget_set_halign(unmount_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(unmount_box), unmount_label, FALSE, FALSE, 0);
    
    app->unmount_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->unmount_entry), "~/decrypted_data");
    gtk_box_pack_start(GTK_BOX(unmount_box), app->unmount_entry, FALSE, FALSE, 0);
    
    GtkWidget *unmount_button = gtk_button_new_with_label("\xe2\x8f\xba Unmount Now");
    gtk_widget_set_halign(unmount_button, GTK_ALIGN_CENTER);
    g_signal_connect(unmount_button, "clicked", G_CALLBACK(on_unmount_clicked), app);
    gtk_box_pack_start(GTK_BOX(unmount_box), unmount_button, FALSE, FALSE, 4);
    
    // Initialize section
    GtkWidget *init_frame = gtk_frame_new(" \xe2\x9c\xa8 Initialize Partition ");
    gtk_box_pack_start(GTK_BOX(content_box), init_frame, FALSE, FALSE, 4);
    
    GtkWidget *init_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(init_box), 10);
    gtk_container_add(GTK_CONTAINER(init_frame), init_box);
    
    app->init_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->init_entry), "Path for NEW encrypted directory");
    gtk_box_pack_start(GTK_BOX(init_box), app->init_entry, FALSE, FALSE, 0);
    
    GtkWidget *init_button = gtk_button_new_with_label("\xe2\x9c\xa8 Initialize Vault");
    gtk_widget_set_halign(init_button, GTK_ALIGN_CENTER);
    g_signal_connect(init_button, "clicked", G_CALLBACK(on_init_clicked), app);
    gtk_box_pack_start(GTK_BOX(init_box), init_button, FALSE, FALSE, 4);
    
    // Exit button
    GtkWidget *exit_button = gtk_button_new_with_label("\xf0\x9f\x9a\xaa Exit Program");
    gtk_widget_set_halign(exit_button, GTK_ALIGN_CENTER);
    g_signal_connect(exit_button, "clicked", G_CALLBACK(on_window_destroy), NULL);
    gtk_box_pack_start(GTK_BOX(content_box), exit_button, FALSE, FALSE, 10);
}

int main(int argc, char *argv[]) {
    prctl(PR_SET_DUMPABLE, 0);
    gtk_init(&argc, &argv);
    SecureMountApp app = {0};
    create_gui(&app);
    gtk_widget_show_all(app.window);
    gtk_main();
    return 0;
}
