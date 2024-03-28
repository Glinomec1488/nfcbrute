#include "../nfcGoesbrr.h"

enum SubmenuIndex {
    // Reserve first indices for opening normal block
    SubmenuIndexReserved = MF_CLASSIC_TOTAL_BLOCKS_MAX,

    // Special options - Sector 0 only
    SubmenuIndexUID,
    SubmenuIndexBCC,
    SubmenuIndexManufacturerBytes,

    // Special options - All sectors
    SubmenuIndexKeyA,
    SubmenuIndexKeyB,
    SubmenuIndexAccessBits,
    SubmenuIndexUserByte,
};

void mfc_editor_scene_block_select_submenu_callback(void* context, uint32_t index) {
    nfcGoesbrr* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void nfcgoesbrr_scene_block_select_on_enter(void* context) {
    nfcGoesbrr* app = context;

    Submenu* submenu = app->submenu;

    uint8_t first_block = mf_classic_get_first_block_num_of_sector(app->current_sector);
    uint8_t block_num = mf_classic_get_blocks_num_in_sector(app->current_sector);

    FuriString* label = furi_string_alloc();
    for(uint8_t i = 0; i < block_num; i++) {
        uint8_t block_index = first_block + i;
        furi_string_printf(label, "Block %u", block_index);
        submenu_add_item(
            submenu,
            furi_string_get_cstr(label),
            block_index,
            mfc_editor_scene_block_select_submenu_callback,
            app);
    }
    furi_string_free(label);

    if(app->current_sector == 0) {
        submenu_add_item(
            submenu, "UID", SubmenuIndexUID, mfc_editor_scene_block_select_submenu_callback, app);
        if(app->mf_classic_data->iso14443_3a_data->uid_len == 4) {
            // 7-byte UID cards don't store a BCC byte
            submenu_add_item(
                submenu,
                "BCC",
                SubmenuIndexBCC,
                mfc_editor_scene_block_select_submenu_callback,
                app);
        }
        submenu_add_item(
            submenu,
            "Manufacturer Bytes",
            SubmenuIndexManufacturerBytes,
            mfc_editor_scene_block_select_submenu_callback,
            app);
    }

    submenu_add_item(
        submenu, "Key A", SubmenuIndexKeyA, mfc_editor_scene_block_select_submenu_callback, app);
    submenu_add_item(
        submenu, "Key B", SubmenuIndexKeyB, mfc_editor_scene_block_select_submenu_callback, app);
    submenu_add_item(
        submenu,
        "Access Bits",
        SubmenuIndexAccessBits,
        mfc_editor_scene_block_select_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "User Byte",
        SubmenuIndexUserByte,
        mfc_editor_scene_block_select_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, SceneBlockSelect));
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewSubmenu);
}

bool nfcgoesbrr_scene_block_select_on_event(void* context, SceneManagerEvent event) {
    nfcGoesbrr* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, SceneBlockSelect, event.event);

        BlockView block_view;
        if(event.event == SubmenuIndexUID) {
            block_view = BlockViewUID;
        } else if(event.event == SubmenuIndexBCC) {
            block_view = BlockViewBCC;
        } else if(event.event == SubmenuIndexManufacturerBytes) {
            block_view = BlockViewManufacturerBytes;
        } else if(event.event == SubmenuIndexKeyA) {
            block_view = BlockViewKeyA;
        } else if(event.event == SubmenuIndexKeyB) {
            block_view = BlockViewKeyB;
        } else if(event.event == SubmenuIndexAccessBits) {
            block_view = BlockViewAccessBits;
            app->current_block = mf_classic_get_first_block_num_of_sector(app->current_sector);
        } else if(event.event == SubmenuIndexUserByte) {
            block_view = BlockViewUserByte;
        } else {
            block_view = BlockViewNormal;
            app->current_block = event.event;
        }

        scene_manager_set_scene_state(app->scene_manager, SceneDataView, block_view);
        scene_manager_next_scene(app->scene_manager, SceneDataView);
        consumed = true;
    }

    return consumed;
}

void nfcgoesbrr_scene_block_select_on_exit(void* context) {
    nfcGoesbrr* app = context;

    submenu_reset(app->submenu);
}
