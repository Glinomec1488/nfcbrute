/*
**************************************************************************************
*This is my first C project so the following code might not have the best structure!!*
*You are welcome to change whatever you want                                         *
*Also im trying to comment every step of the application                             *
*So dont be confused why there is so many unnesesary comment lines!                  *
**************************************************************************************
*/
#pragma once //makes this code only runnable once

#include <furi.h>

#include <dialogs/dialogs.h>

#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

//#include <gui/elements.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/submenu.h>
//#include <gui/view.h>
#include <gui/modules/widget.h>
#include <gui/modules/popup.h>
#include <gui/modules/dialog_ex.h>

#include <nfc/nfc.h>
#include <nfc/nfc_device.h>
#include <nfc/protocols/mf_classic/mf_classic.h>

#include <storage/storage.h>

#include "scenes/nfcgoesbrr_scene.h"
#include "nfc_sector_brute_icons.h"
#include "nfcGoesbrrAbout.h"
#include "nfcGoesbrrI.h"

#define TAG "MFbrute"

#define NFC_APP_FOLDER ANY_PATH("/nfc")
#define NFC_APP_EXTENSION ".nfc"
#define NFC_APP_SHADOW_EXTENSION ".shd"

enum CustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    CustomEventReserved = 100,

    CustomEventViewExit,
    CustomEventSave,
};

typedef enum {
    nfcGoesbrrSubmenuIndexFileSelect,
    nfcGoesbrrSubmenuIndexSavedConfs,
    nfcGoesbrrSubmenuIndexAbout,
} nfcGoesbrrSubmenuIndex;

typedef struct {
    uint8_t bits : 3;
    uint8_t check_bits : 3;
} nfcAccessBits;

struct nfcGoesbrr { //structure declarations
    Gui* gui; //invoking a gui
    ViewPort* view_port; //declaring a ViewPort
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    Popup* popup;
    SceneManager* scene_manager;

    Storage* storage;
    DialogsApp* dialogs;
    DialogEx* dialog_ex;
    ByteInput* byte_input;
    bool is_unsaved_changes;

    MfClassicData* mf_classic_data;
    FuriString* file_path;
    uint8_t current_sector;
    uint8_t current_block;
    FuriString* data_view_header;
    FuriString* data_view_text;
    uint8_t* edit_buffer;
    nfcAccessBits access_bits_edit;
};

typedef enum {
    ViewSubmenu,
    ViewPopup,
    ViewDialogEx,
    ViewByteInput,
    ViewAbout,
} AppView;

typedef enum {
    // Generic
    PromptResponseSuccess,
    PromptResponseFailure,

    PromptResponseNotMfClassic,

    // Backed out of a prompt
    PromptResponseExitedFile,
    PromptResponseExitedShadow,
} PromptResponse;

typedef enum {
    SaveResponseSave,
    SaveResponseDiscard,
    SaveResponseCancel,
} SaveResponse;

typedef enum {
    BlockViewNormal,

    // Special options - Sector 0 only
    BlockViewUID,
    BlockViewBCC,
    BlockViewManufacturerBytes,

    // Special options - All sectors
    BlockViewKeyA,
    BlockViewKeyB,
    BlockViewAccessBits,
    BlockViewUserByte,
} BlockView;

PromptResponse prompt_load_file(nfcGoesbrr* app);
PromptResponse load_file(nfcGoesbrr* app, FuriString* file_path);

extern const char* access_data_block_labels[8];

extern const char* access_sector_trailer_labels[8];

nfcAccessBits get_block_access_bits(const MfClassicData* data, uint8_t block_num);
uint8_t calculate_uid_bcc(uint8_t* uid, uint8_t uid_len);
void furi_string_render_bytes(FuriString* string, const uint8_t* data, uint8_t length);
