// web_crawler_callback.h
static bool sent_get_request = false;
static bool get_success = false;
static bool already_success = false;
static WebCrawlerApp* app_instance = NULL;

// Forward declaration of callback functions
static void web_crawler_setting_item_path_clicked(void* context, uint32_t index);
static void web_crawler_setting_item_ssid_clicked(void* context, uint32_t index);
static void web_crawler_setting_item_password_clicked(void* context, uint32_t index);
static void web_crawler_setting_item_file_type_clicked(void* context, uint32_t index);
static void web_crawler_setting_item_file_rename_clicked(void* context, uint32_t index);
static void web_crawler_setting_item_file_delete_clicked(void* context, uint32_t index);
static void web_crawler_setting_item_file_read_clicked(void* context, uint32_t index);

static void web_crawler_view_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    if(!app_instance) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    if(!canvas) {
        FURI_LOG_E(TAG, "Canvas is NULL");
        return;
    }

    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    if(fhttp.state == INACTIVE) {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If you board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your Dev Board with the");
        canvas_draw_str(canvas, 0, 62, "FlipperHTTP firmware.");
        return;
    }

    if(app_instance->path) {
        if(!sent_get_request) {
            canvas_draw_str(canvas, 0, 10, "Sending GET request...");

            // Perform GET request and handle the response
            get_success = flipper_http_get_request(app_instance->path);

            canvas_draw_str(canvas, 0, 20, "Sent!");

            if(get_success) {
                canvas_draw_str(canvas, 0, 30, "Receiving data...");
                // Set the state to RECEIVING to ensure we continue to see the receiving message
                fhttp.state = RECEIVING;
            } else {
                canvas_draw_str(canvas, 0, 30, "Failed.");
            }

            sent_get_request = true;
        } else {
            // print state
            if(get_success && fhttp.state == RECEIVING) {
                canvas_draw_str(canvas, 0, 10, "Receiving and parsing data...");
            } else if(get_success && fhttp.state == IDLE) {
                canvas_draw_str(canvas, 0, 10, "Data saved to file.");
                canvas_draw_str(canvas, 0, 20, "Press BACK to return.");
            } else {
                if(fhttp.state == ISSUE) {
                    if(strstr(
                           fhttp.last_response,
                           "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL) {
                        canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
                        canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
                        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
                    } else if(
                        strstr(fhttp.last_response, "[ERROR] Failed to connect to Wifi.") !=
                        NULL) {
                        canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
                        canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
                        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
                    } else {
                        canvas_draw_str(canvas, 0, 10, "[ERROR] Failed to sync data.");
                        canvas_draw_str(canvas, 0, 30, "If this is your third attempt,");
                        canvas_draw_str(canvas, 0, 40, "it's likely your URL is not");
                        canvas_draw_str(canvas, 0, 50, "compabilbe or correct.");
                        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
                    }
                } else {
                    canvas_draw_str(canvas, 0, 10, "GET request failed.");
                    canvas_draw_str(canvas, 0, 20, "Press BACK to return.");
                }
                get_success = false;
            }
        }
    } else {
        canvas_draw_str(canvas, 0, 10, "URL not set.");
    }
}

/**
 * @brief      Navigation callback to handle exiting from other views to the submenu.
 * @param      context   The context - WebCrawlerApp object.
 * @return     WebCrawlerViewSubmenu
 */
static uint32_t web_crawler_back_to_configure_callback(void* context) {
    UNUSED(context);
    // free file read widget if it exists
    if(app_instance->widget_file_read) {
        widget_reset(app_instance->widget_file_read);
    }
    return WebCrawlerViewSubmenuConfig; // Return to the configure screen
}

/**
 * @brief      Navigation callback to handle returning to the Wifi Settings screen.
 * @param      context   The context - WebCrawlerApp object.
 * @return     WebCrawlerViewSubmenu
 */
static uint32_t web_crawler_back_to_main_callback(void* context) {
    UNUSED(context);
    // reset GET request flags
    sent_get_request = false;
    get_success = false;
    already_success = false;
    // free file read widget if it exists
    if(app_instance->widget_file_read) {
        widget_reset(app_instance->widget_file_read);
    }
    return WebCrawlerViewSubmenuMain; // Return to the main submenu
}

static uint32_t web_crawler_back_to_file_callback(void* context) {
    UNUSED(context);
    return WebCrawlerViewVariableItemListFile; // Return to the file submenu
}

static uint32_t web_crawler_back_to_wifi_callback(void* context) {
    UNUSED(context);
    return WebCrawlerViewVariableItemListWifi; // Return to the wifi submenu
}

static uint32_t web_crawler_back_to_request_callback(void* context) {
    UNUSED(context);
    return WebCrawlerViewVariableItemListRequest; // Return to the request submenu
}

/**
 * @brief      Navigation callback to handle exiting the app from the main submenu.
 * @param      context   The context - unused
 * @return     VIEW_NONE to exit the app
 */
static uint32_t web_crawler_exit_app_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

/**
 * @brief      Handle submenu item selection.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The WebCrawlerSubmenuIndex item that was clicked.
 */
static void web_crawler_submenu_callback(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;

    if(app->view_dispatcher) {
        switch(index) {
        case WebCrawlerSubmenuIndexRun:
            sent_get_request = false; // Reset the flag
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewRun);
            break;
        case WebCrawlerSubmenuIndexAbout:
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewAbout);
            break;
        case WebCrawlerSubmenuIndexConfig:
            view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewSubmenuConfig);
            break;
        case WebCrawlerSubmenuIndexWifi:
            view_dispatcher_switch_to_view(
                app->view_dispatcher, WebCrawlerViewVariableItemListWifi);
            break;
        case WebCrawlerSubmenuIndexRequest:
            view_dispatcher_switch_to_view(
                app->view_dispatcher, WebCrawlerViewVariableItemListRequest);
            break;
        case WebCrawlerSubmenuIndexFile:
            view_dispatcher_switch_to_view(
                app->view_dispatcher, WebCrawlerViewVariableItemListFile);
            break;
        default:
            FURI_LOG_E(TAG, "Unknown submenu index");
            break;
        }
    }
}

/**
 * @brief      Configuration enter callback to handle different items.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The index of the item that was clicked.
 */
static void web_crawler_wifi_enter_callback(void* context, uint32_t index) {
    switch(index) {
    case 0: // SSID
        web_crawler_setting_item_ssid_clicked(context, index);
        break;
    case 1: // Password
        web_crawler_setting_item_password_clicked(context, index);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief      Configuration enter callback to handle different items.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The index of the item that was clicked.
 */
static void web_crawler_file_enter_callback(void* context, uint32_t index) {
    switch(index) {
    case 0: // File Read
        web_crawler_setting_item_file_read_clicked(context, index);
        break;
    case 1: // FIle Type
        web_crawler_setting_item_file_type_clicked(context, index);
        break;
    case 2: // File Rename
        web_crawler_setting_item_file_rename_clicked(context, index);
        break;
    case 3: // File Delete
        web_crawler_setting_item_file_delete_clicked(context, index);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief      Configuration enter callback to handle different items.
 * @param      context   The context - WebCrawlerApp object.
 * @param      index     The index of the item that was clicked.
 */
static void web_crawler_request_enter_callback(void* context, uint32_t index) {
    switch(index) {
    case 0: // URL
        web_crawler_setting_item_path_clicked(context, index);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief      Callback for when the user finishes entering the URL.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_path_updated(void* context) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    if(!app->path || !app->temp_buffer_path || !app->temp_buffer_size_path || !app->path_item) {
        FURI_LOG_E(TAG, "Invalid path buffer");
        return;
    }
    // Store the entered URL from temp_buffer_path to path
    strncpy(app->path, app->temp_buffer_path, app->temp_buffer_size_path - 1);

    if(app->path_item) {
        variable_item_set_current_value_text(app->path_item, app->path);

        // Save the URL to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type);

        // send to UART
        if(!flipper_http_save_wifi(app->ssid, app->password)) {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }

        FURI_LOG_D(TAG, "URL saved: %s", app->path);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListRequest);
}

/**
 * @brief      Callback for when the user finishes entering the SSID.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_ssid_updated(void* context) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    if(!app->temp_buffer_ssid || !app->temp_buffer_size_ssid || !app->ssid || !app->ssid_item) {
        FURI_LOG_E(TAG, "Invalid SSID buffer");
        return;
    }
    // Store the entered SSID from temp_buffer_ssid to ssid
    strncpy(app->ssid, app->temp_buffer_ssid, app->temp_buffer_size_ssid - 1);

    if(app->ssid_item) {
        variable_item_set_current_value_text(app->ssid_item, app->ssid);

        // Save the SSID to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type);

        // send to UART
        if(!flipper_http_save_wifi(app->ssid, app->password)) {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }

        FURI_LOG_D(TAG, "SSID saved: %s", app->ssid);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListWifi);
}

/**
 * @brief      Callback for when the user finishes entering the Password.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_password_update(void* context) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    if(!app->temp_buffer_password || !app->temp_buffer_size_password || !app->password ||
       !app->password_item) {
        FURI_LOG_E(TAG, "Invalid password buffer");
        return;
    }
    // Store the entered Password from temp_buffer_password to password
    strncpy(app->password, app->temp_buffer_password, app->temp_buffer_size_password - 1);

    if(app->password_item) {
        variable_item_set_current_value_text(app->password_item, app->password);

        // Save the Password to the settings
        save_settings(app->path, app->ssid, app->password, app->file_rename, app->file_type);

        // send to UART
        if(!flipper_http_save_wifi(app->ssid, app->password)) {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }

        FURI_LOG_D(TAG, "Password saved: %s", app->password);
    }

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListWifi);
}

/**
 * @brief      Callback for when the user finishes entering the File Type.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_file_type_update(void* context) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    if(!app->temp_buffer_file_type || !app->temp_buffer_size_file_type || !app->file_type ||
       !app->file_type_item) {
        FURI_LOG_E(TAG, "Invalid file type buffer");
        return;
    }
    // Temporary buffer to store the old name
    char old_file_type[256];

    strncpy(old_file_type, app->file_type, sizeof(old_file_type) - 1);
    old_file_type[sizeof(old_file_type) - 1] = '\0'; // Null-terminate
    strncpy(app->file_type, app->temp_buffer_file_type, app->temp_buffer_size_file_type - 1);

    if(app->file_type_item) {
        variable_item_set_current_value_text(app->file_type_item, app->file_type);
    }

    rename_received_data(app->file_rename, app->file_rename, app->file_type, old_file_type);

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListFile);
}

/**
 * @brief      Callback for when the user finishes entering the File Rename.
 * @param      context   The context - WebCrawlerApp object.
 */
static void web_crawler_set_file_rename_update(void* context) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    if(!app->temp_buffer_file_rename || !app->temp_buffer_size_file_rename || !app->file_rename ||
       !app->file_rename_item) {
        FURI_LOG_E(TAG, "Invalid file rename buffer");
        return;
    }

    // Temporary buffer to store the old name
    char old_name[256];

    // Ensure that app->file_rename is null-terminated
    strncpy(old_name, app->file_rename, sizeof(old_name) - 1);
    old_name[sizeof(old_name) - 1] = '\0'; // Null-terminate

    // Store the entered File Rename from temp_buffer_file_rename to file_rename
    strncpy(app->file_rename, app->temp_buffer_file_rename, app->temp_buffer_size_file_rename - 1);

    if(app->file_rename_item) {
        variable_item_set_current_value_text(app->file_rename_item, app->file_rename);
    }

    rename_received_data(old_name, app->file_rename, app->file_type, app->file_type);

    // Return to the Configure view
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewVariableItemListFile);
}

/**
 * @brief      Handler for Path configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_path_clicked(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    if(!app->text_input_path) {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }

    UNUSED(index);
    // Set up the text input
    text_input_set_header_text(app->text_input_path, "Enter URL");

    // Initialize temp_buffer with existing path
    if(app->path && strlen(app->path) > 0) {
        strncpy(app->temp_buffer_path, app->path, app->temp_buffer_size_path - 1);
    } else {
        strncpy(app->temp_buffer_path, "https://www.google.com/", app->temp_buffer_size_path - 1);
    }

    app->temp_buffer_path[app->temp_buffer_size_path - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_path,
        web_crawler_set_path_updated,
        app,
        app->temp_buffer_path,
        app->temp_buffer_size_path,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_path), web_crawler_back_to_request_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInput);
}

/**
 * @brief      Handler for SSID configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_ssid_clicked(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    UNUSED(index);
    if(!app->text_input_ssid) {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }
    // Set up the text input
    text_input_set_header_text(app->text_input_ssid, "Enter SSID");

    // Initialize temp_buffer with existing SSID
    if(app->ssid && strlen(app->ssid) > 0) {
        strncpy(app->temp_buffer_ssid, app->ssid, app->temp_buffer_size_ssid - 1);
    } else {
        strncpy(app->temp_buffer_ssid, "SSID-2G-", app->temp_buffer_size_ssid - 1);
    }

    app->temp_buffer_ssid[app->temp_buffer_size_ssid - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_ssid,
        web_crawler_set_ssid_updated,
        app,
        app->temp_buffer_ssid,
        app->temp_buffer_size_ssid,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_ssid), web_crawler_back_to_wifi_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputSSID);
}

/**
 * @brief      Handler for Password configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_password_clicked(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    UNUSED(index);
    if(!app->text_input_password) {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }
    // Set up the text input
    text_input_set_header_text(app->text_input_password, "Enter Password");

    // Initialize temp_buffer with existing password
    strncpy(app->temp_buffer_password, app->password, app->temp_buffer_size_password - 1);
    app->temp_buffer_password[app->temp_buffer_size_password - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_password,
        web_crawler_set_password_update,
        app,
        app->temp_buffer_password,
        app->temp_buffer_size_password,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_password), web_crawler_back_to_wifi_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputPassword);
}

/**
 * @brief      Handler for File Type configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_file_type_clicked(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    UNUSED(index);
    if(!app->text_input_file_type) {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }
    // Set up the text input
    text_input_set_header_text(app->text_input_file_type, "Enter File Type");

    // Initialize temp_buffer with existing file_type
    if(app->file_type && strlen(app->file_type) > 0) {
        strncpy(app->temp_buffer_file_type, app->file_type, app->temp_buffer_size_file_type - 1);
    } else {
        strncpy(app->temp_buffer_file_type, ".txt", app->temp_buffer_size_file_type - 1);
    }

    app->temp_buffer_file_type[app->temp_buffer_size_file_type - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_file_type,
        web_crawler_set_file_type_update,
        app,
        app->temp_buffer_file_type,
        app->temp_buffer_size_file_type,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_file_type), web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputFileType);
}

/**
 * @brief      Handler for File Rename configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_file_rename_clicked(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    UNUSED(index);
    if(!app->text_input_file_rename) {
        FURI_LOG_E(TAG, "Text input is NULL");
        return;
    }
    // Set up the text input
    text_input_set_header_text(app->text_input_file_rename, "Enter File Rename");

    // Initialize temp_buffer with existing file_rename
    if(app->file_rename && strlen(app->file_rename) > 0) {
        strncpy(
            app->temp_buffer_file_rename, app->file_rename, app->temp_buffer_size_file_rename - 1);
    } else {
        strncpy(
            app->temp_buffer_file_rename, "received_data", app->temp_buffer_size_file_rename - 1);
    }

    app->temp_buffer_file_rename[app->temp_buffer_size_file_rename - 1] = '\0';

    // Configure the text input
    bool clear_previous_text = false;
    text_input_set_result_callback(
        app->text_input_file_rename,
        web_crawler_set_file_rename_update,
        app,
        app->temp_buffer_file_rename,
        app->temp_buffer_size_file_rename,
        clear_previous_text);

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        text_input_get_view(app->text_input_file_rename), web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewTextInputFileRename);
}

/**
 * @brief      Handler for File Delete configuration item click.
 * @param      context  The context - WebCrawlerApp object.
 * @param      index    The index of the item that was clicked.
 */
static void web_crawler_setting_item_file_delete_clicked(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    UNUSED(index);

    if(!delete_received_data(app)) {
        FURI_LOG_E(TAG, "Failed to delete file");
    }

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        widget_get_view(app->widget_file_delete), web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewFileDelete);
}

static void web_crawler_setting_item_file_read_clicked(void* context, uint32_t index) {
    WebCrawlerApp* app = (WebCrawlerApp*)context;
    if(!app) {
        FURI_LOG_E(TAG, "WebCrawlerApp is NULL");
        return;
    }
    UNUSED(index);

    if(!load_received_data(app)) {
        if(app->widget_file_read) {
            widget_reset(app->widget_file_read);
            widget_add_text_scroll_element(app->widget_file_read, 0, 0, 128, 64, "File is empty.");
        }
    }

    // Set the previous callback to return to Configure screen
    view_set_previous_callback(
        widget_get_view(app->widget_file_read), web_crawler_back_to_file_callback);

    // Show text input dialog
    view_dispatcher_switch_to_view(app->view_dispatcher, WebCrawlerViewFileRead);
}
