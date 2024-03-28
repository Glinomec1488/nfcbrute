#include "nfcGoesbrr.h"

const char* access_data_block_labels[8] = {
    // C3, C2, C1
    "Key A: Read, Write, Inc, Dec\nKey B: Read, Write, Inc, Dec", // 000
    "Key A: Read\nKey B: Read, Write", // 001
    "Key A: Read\nKey B: Read", // 010
    "Key A: Read, Dec\nKey B: Read, Write, Inc, Dec", // 011
    "Key A: Read, Dec\nKey B: Read, Dec", // 100
    "Key A: No Access\nKey B: Read", // 101
    "Key A: No Access\nKey B: Read, Write", // 110
    "Key A: No Access\nKey B: No Access", // 111
};

const char* access_sector_trailer_labels[8] = {
    // C3, C2, C1
    "Key A: KA-W, AB-R, KB-RW\nKey B: No Access", // 000
    "Key A: AB-R\nKey B: KA+KB-W, AB-R", // 001
    "Key A: AB+KB-R\nKey B: No Access", // 010
    "Key A: AB-R\nKey B: AB-R", // 011
    "Key A: KA-W, AB+KB-RW\nKey B: No Access", // 100
    "Key A: AB-R\nKey B: AB-RW", // 101
    "Key A: AB-R\nKey B: KA+KB-W, AB-RW", // 110
    "Key A: AB-R\nKey B: AB-R", // 111
};

bool custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    nfcGoesbrr* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

bool back_event_callback(void* context) {
    furi_assert(context);
    nfcGoesbrr* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

void tick_event_callback(void* context) {
    furi_assert(context);
    nfcGoesbrr* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

nfcGoesbrr* nfcgoesbrr_app_alloc() { //allocate memory

    nfcGoesbrr* app = malloc(sizeof(nfcGoesbrr)); //allocate memory

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&nfcgoesbrr_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, custom_event_callback);

    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, back_event_callback);

    view_dispatcher_set_tick_event_callback(app->view_dispatcher, tick_event_callback, 100);

    app->gui = furi_record_open(RECORD_GUI);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->storage = furi_record_open(RECORD_STORAGE);

    app->dialogs = furi_record_open(RECORD_DIALOGS);

    app->mf_classic_data = mf_classic_alloc();
    app->file_path = furi_string_alloc_set(NFC_APP_FOLDER);

    app->data_view_header = furi_string_alloc();
    app->data_view_text = furi_string_alloc();

    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, ViewAbout, widget_get_view(app->widget));

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, ViewSubmenu, submenu_get_view(app->submenu));

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, ViewPopup, popup_get_view(app->popup));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, ViewDialogEx, dialog_ex_get_view(app->dialog_ex));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, ViewByteInput, byte_input_get_view(app->byte_input));

    return app;
}

void nfcgoesbrr_app_free(nfcGoesbrr* app) { //free allocated memory

    view_dispatcher_remove_view(app->view_dispatcher, ViewSubmenu);
    submenu_free(app->submenu);

    view_dispatcher_remove_view(app->view_dispatcher, ViewPopup);
    popup_free(app->popup);

    view_dispatcher_remove_view(app->view_dispatcher, ViewDialogEx);
    dialog_ex_free(app->dialog_ex);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    furi_record_close(RECORD_STORAGE);
    app->storage = NULL;

    furi_record_close(RECORD_DIALOGS);
    app->dialogs = NULL;

    mf_classic_free(app->mf_classic_data);
    furi_string_free(app->file_path);

    furi_string_free(app->data_view_header);
    furi_string_free(app->data_view_text);

    free(app);
}

int32_t nfcgoesbrr_entry(void* p) { //entry point of the app <MAIN FUNCTION>
    UNUSED(p);
    nfcGoesbrr* app = nfcgoesbrr_app_alloc();

    scene_manager_next_scene(app->scene_manager, SceneMsubmenu);

    view_dispatcher_run(app->view_dispatcher);

    nfcgoesbrr_app_free(app); //invoke the function
    return 0;
}
