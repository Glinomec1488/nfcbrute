#include "../nfcGoesbrr.h"

void nfcgoesbrr_submenu_callback(void* context, uint32_t index) {
    nfcGoesbrr* app = context;

    switch(index) {
    case nfcGoesbrrSubmenuIndexFileSelect:
        scene_manager_next_scene(app->scene_manager, SceneFileSelect);
        break;
    case nfcGoesbrrSubmenuIndexSavedConfs:
        furi_hal_vibro_on(true);
        furi_delay_ms(200);
        furi_hal_vibro_on(false);
        break;
    case nfcGoesbrrSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, nfcGoesbrrViewAbout);
        break;
    default:
        break;
    }
}

void nfcgoesbrr_scene_main_menu_on_enter(void* context) {
    nfcGoesbrr* app = context;

    app->widget_about = widget_alloc();
    widget_add_text_scroll_element(app->widget_about, 0, 0, 128, 64, NFCGOESBRR_ABOUT_TEXT);
    view_dispatcher_add_view(
        app->view_dispatcher, nfcGoesbrrViewAbout, widget_get_view(app->widget_about));
    view_dispatcher_add_view(
        app->view_dispatcher, nfcGoesbrrViewSubmenu, submenu_get_view(app->submenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, nfcGoesbrrViewSubmenu);
    submenu_add_item(
        app->submenu,
        "Select a file",
        nfcGoesbrrSubmenuIndexFileSelect,
        nfcgoesbrr_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Saved configurations",
        nfcGoesbrrSubmenuIndexSavedConfs,
        nfcgoesbrr_submenu_callback,
        app);
    submenu_add_item(
        app->submenu, "About", nfcGoesbrrSubmenuIndexAbout, nfcgoesbrr_submenu_callback, app);
}

bool nfcgoesbrr_scene_main_menu_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfcgoesbrr_scene_main_menu_on_exit(void* context) {
    UNUSED(context);
}