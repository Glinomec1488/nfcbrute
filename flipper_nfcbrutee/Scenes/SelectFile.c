#include "../nfcGoesbrr.h"

void nfcgoesbrr_scene_file_select_on_enter(void* context) {
    nfcGoesbrr* app = context;
    // File select scene should repeat itself if the file load failed
    // or if the user quit the shadow file prompt, not the file selector
    PromptResponse prompt_response = PromptResponseFailure;
    while(prompt_response == PromptResponseFailure ||
          prompt_response == PromptResponseExitedShadow) {
        prompt_response = prompt_load_file(app);
    }

    if(prompt_response == PromptResponseSuccess) {
        scene_manager_set_scene_state(app->scene_manager, SceneSectorSelect, 0);
        scene_manager_next_scene(app->scene_manager, SceneSectorSelect);
    } else if(prompt_response == PromptResponseNotMfClassic) {
        scene_manager_next_scene(app->scene_manager, SceneInvalidFile);
    } else {
        // Exiting from file list stops the application
        view_dispatcher_stop(app->view_dispatcher);
    }
}

bool nfcgoesbrr_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfcgoesbrr_scene_file_select_on_exit(void* context) {
    UNUSED(context);
}

static void mfc_editor_get_shadow_file_path(FuriString* file_path, FuriString* shadow_file_path) {
    furi_assert(file_path);
    furi_assert(shadow_file_path);

    // Remove NFC extension from end of string then append shadow extension
    furi_string_set_n(shadow_file_path, file_path, 0, furi_string_size(file_path) - 4);
    furi_string_cat_printf(shadow_file_path, "%s", NFC_APP_SHADOW_EXTENSION);
}

static bool mfc_editor_file_has_shadow_file(nfcGoesbrr* app, FuriString* file_path) {
    furi_assert(app);
    furi_assert(file_path);

    FuriString* shadow_file_path = furi_string_alloc();
    mfc_editor_get_shadow_file_path(file_path, shadow_file_path);
    bool has_shadow_file =
        storage_common_exists(app->storage, furi_string_get_cstr(shadow_file_path));

    furi_string_free(shadow_file_path);

    return has_shadow_file;
}

PromptResponse load_file(nfcGoesbrr* app, FuriString* file_path) {
    furi_assert(app);
    furi_assert(file_path);

    PromptResponse result = PromptResponseSuccess;

    NfcDevice* nfc_device = nfc_device_alloc();

    if(!nfc_device_load(nfc_device, furi_string_get_cstr(file_path))) {
        result = PromptResponseFailure;
        dialog_message_show_storage_error(app->dialogs, "Cannot load\nkey file");
    } else {
        if(nfc_device_get_protocol(nfc_device) == NfcProtocolMfClassic) {
            const MfClassicData* mf_classic_data =
                nfc_device_get_data(nfc_device, NfcProtocolMfClassic);
            mf_classic_copy(app->mf_classic_data, mf_classic_data);
            app->is_unsaved_changes = false;
        } else {
            result = PromptResponseNotMfClassic;
        }
    }

    nfc_device_free(nfc_device);

    return result;
}

static DialogMessageButton mfc_editor_prompt_should_load_shadow(nfcGoesbrr* app) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "File has modifications", 63, 3, AlignCenter, AlignTop);
    dialog_message_set_text(
        message,
        "Would you like to edit the\nmodified file (recommended)\nor the original file?",
        63,
        31,
        AlignCenter,
        AlignCenter);
    dialog_message_set_buttons(message, "Original", NULL, "Modified");

    DialogMessageButton message_button = dialog_message_show(app->dialogs, message);

    dialog_message_free(message);

    return message_button;
}

PromptResponse prompt_load_file(nfcGoesbrr* app) {
    furi_assert(app);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, NFC_APP_EXTENSION, &I_amperka_ru_logo_128x35px);
    browser_options.base_path = NFC_APP_FOLDER;
    browser_options.hide_dot_files = true;

    PromptResponse result = PromptResponseSuccess;
    if(!dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options)) {
        result = PromptResponseExitedFile;
    } else {
        if(mfc_editor_file_has_shadow_file(app, app->file_path)) {
            DialogMessageButton message_button = mfc_editor_prompt_should_load_shadow(app);

            if(message_button == DialogMessageButtonRight) {
                // User selected to use shadow file, so replace selected path with that path
                FuriString* shadow_file_path = furi_string_alloc();
                mfc_editor_get_shadow_file_path(app->file_path, shadow_file_path);
                furi_string_set(app->file_path, shadow_file_path);
                furi_string_free(shadow_file_path);
            } else if(message_button == DialogMessageButtonBack) {
                result = PromptResponseExitedShadow;
            }
        }

        // Don't load the file if user was prompted for shadow file use but went back
        if(result == PromptResponseSuccess) {
            result = load_file(app, app->file_path);
        }
    }
    return result;
}
