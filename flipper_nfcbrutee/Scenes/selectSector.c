#include "../nfcGoesbrr.h"

void scene_sector_select_submenu_callback(void* context, uint32_t index) {
    nfcGoesbrr* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfcgoesbrr_scene_sector_select_on_enter(void* context) {
    nfcGoesbrr* app = context;

    Submenu* submenu = app->submenu;

    uint8_t sectors_num = mf_classic_get_total_sectors_num(app->mf_classic_data->type);

    FuriString* label = furi_string_alloc();
    for(uint8_t i = 0; i < sectors_num; i++) {
        furi_string_printf(label, "Sector %u", i);
        submenu_add_item(
            submenu, furi_string_get_cstr(label), i, scene_sector_select_submenu_callback, app);
    }
    furi_string_free(label);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, SceneSectorSelect));
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewSubmenu);
}

bool nfcgoesbrr_scene_sector_select_on_event(void* context, SceneManagerEvent event) {
    nfcGoesbrr* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, SceneSectorSelect, event.event);
        app->current_sector = event.event;
        scene_manager_set_scene_state(app->scene_manager, SceneBlockSelect, 0);
        scene_manager_next_scene(app->scene_manager, SceneBlockSelect);
    } else if(event.type == SceneManagerEventTypeBack) {
        if(app->is_unsaved_changes) {
        }
    }
    return 0;
}

void nfcgoesbrr_scene_sector_select_on_exit(void* context) {
    nfcGoesbrr* app = context;

    submenu_reset(app->submenu);
}
