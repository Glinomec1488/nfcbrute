#include "../nfcGoesbrr.h"

void nfcgoesbrr_scene_invalid_file_popup_callback(void* context) {
    nfcGoesbrr* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, CustomEventViewExit);
}

void nfcgoesbrr_scene_invalid_file_on_enter(void* context) {
    nfcGoesbrr* app = context;

    Popup* popup = app->popup;
    popup_set_header(popup, "Invalid file", 63, 10, AlignCenter, AlignTop);
    popup_set_text(
        popup, "Only MIFARE Classic files\nare supported", 63, 40, AlignCenter, AlignCenter);
    popup_set_context(popup, app);
    popup_set_callback(popup, nfcgoesbrr_scene_invalid_file_popup_callback);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewPopup);
}

bool nfcgoesbrr_scene_invalid_file_on_event(void* context, SceneManagerEvent event) {
    nfcGoesbrr* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == CustomEventViewExit) {
            consumed = scene_manager_previous_scene(app->scene_manager);
        }
    }

    return consumed;
}

void nfcgoesbrr_scene_invalid_file_on_exit(void* context) {
    nfcGoesbrr* app = context;

    popup_reset(app->popup);
}
