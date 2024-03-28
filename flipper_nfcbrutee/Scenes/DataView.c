#include "../nfcGoesbrr.h"

void scene_data_view_dialog_ex_callback(DialogExResult result, void* context) {
    nfcGoesbrr* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void scene_data_view_update_display(nfcGoesbrr* app) {
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_reset(app->dialog_ex);
    dialog_ex_set_context(app->dialog_ex, app);
    dialog_ex_set_result_callback(app->dialog_ex, scene_data_view_dialog_ex_callback);

    BlockView block_view = scene_manager_get_scene_state(app->scene_manager, SceneDataView);

    MfClassicData* mf_classic_data = app->mf_classic_data;
    Iso14443_3aData* iso14443_3a_data = mf_classic_data->iso14443_3a_data;

    furi_string_reset(app->data_view_text);

    if(block_view == BlockViewUID) {
        dialog_ex_set_header(dialog_ex, "UID", 63, 3, AlignCenter, AlignTop);

        furi_string_render_bytes(
            app->data_view_text, iso14443_3a_data->uid, iso14443_3a_data->uid_len);

        if(memcmp(
               iso14443_3a_data->uid, mf_classic_data->block[0].data, iso14443_3a_data->uid_len)) {
            // ISO-14443 UID does not match first bytes of block 0
            furi_string_cat_printf(app->data_view_text, "\nBlock 0 does not match UID!\n(");
            furi_string_render_bytes(
                app->data_view_text, mf_classic_data->block[0].data, iso14443_3a_data->uid_len);
            furi_string_push_back(app->data_view_text, ')');
            dialog_ex_set_center_button_text(dialog_ex, "Fix");
        }

        if(mf_classic_is_block_read(mf_classic_data, 0)) {
            dialog_ex_set_right_button_text(dialog_ex, "Edit");
        }
    } else if(block_view == BlockViewBCC) {
        dialog_ex_set_header(dialog_ex, "Block Check Character", 63, 3, AlignCenter, AlignTop);

        uint8_t stored_bcc = mf_classic_data->block[0].data[4];
        uint8_t calculated_bcc =
            calculate_uid_bcc(iso14443_3a_data->uid, iso14443_3a_data->uid_len);

        if(mf_classic_is_block_read(mf_classic_data, 0)) {
            furi_string_printf(
                app->data_view_text,
                "Stored BCC: %02X\nActual BCC: %02X",
                stored_bcc,
                calculated_bcc);

            if(stored_bcc != calculated_bcc) {
                furi_string_cat(app->data_view_text, "\n(Mismatch!)");
                dialog_ex_set_center_button_text(dialog_ex, "Fix");
            }
        } else {
            furi_string_printf(
                app->data_view_text,
                "Actual BCC: %02X\nStored BCC is unavailable\nas Block 0 has not been read.",
                calculated_bcc);
        }
    } else if(block_view == BlockViewManufacturerBytes) {
        dialog_ex_set_header(dialog_ex, "Manufacturer Bytes", 63, 3, AlignCenter, AlignTop);

        if(mf_classic_is_block_read(mf_classic_data, 0)) {
            // Skip BCC byte (not present on 7B UID cards)
            bool skip_byte = iso14443_3a_data->uid_len == 4;
            uint8_t start_index = iso14443_3a_data->uid_len + skip_byte;
            uint8_t byte_num = MF_CLASSIC_BLOCK_SIZE - iso14443_3a_data->uid_len - skip_byte;
            uint8_t line_len = byte_num / 2;

            furi_string_render_bytes(
                app->data_view_text, mf_classic_data->block[0].data + start_index, line_len);
            furi_string_push_back(app->data_view_text, '\n');
            furi_string_render_bytes(
                app->data_view_text,
                mf_classic_data->block[0].data + start_index + line_len,
                byte_num - line_len);

            dialog_ex_set_right_button_text(dialog_ex, "Edit");
        } else {
            furi_string_set(app->data_view_text, "Data unavailable.\nBlock 0 has not been read.");
        }
    } else if(block_view == BlockViewKeyA) {
        dialog_ex_set_header(dialog_ex, "Key A", 63, 3, AlignCenter, AlignTop);

        if(mf_classic_is_key_found(mf_classic_data, app->current_sector, MfClassicKeyTypeA)) {
            MfClassicSectorTrailer* sector_trailer =
                mf_classic_get_sector_trailer_by_sector(mf_classic_data, app->current_sector);
            furi_string_render_bytes(
                app->data_view_text, sector_trailer->key_a.data, MF_CLASSIC_KEY_SIZE);
            dialog_ex_set_right_button_text(dialog_ex, "Edit");
        } else {
            furi_string_set(app->data_view_text, "Key A has not been found\nfor this sector.");
        }
    } else if(block_view == BlockViewKeyB) {
        dialog_ex_set_header(dialog_ex, "Key B", 63, 3, AlignCenter, AlignTop);

        if(mf_classic_is_key_found(mf_classic_data, app->current_sector, MfClassicKeyTypeB)) {
            MfClassicSectorTrailer* sector_trailer =
                mf_classic_get_sector_trailer_by_sector(mf_classic_data, app->current_sector);
            furi_string_render_bytes(
                app->data_view_text, sector_trailer->key_b.data, MF_CLASSIC_KEY_SIZE);
            dialog_ex_set_right_button_text(dialog_ex, "Edit");
        } else {
            furi_string_set(app->data_view_text, "Key B has not been found\nfor this sector.");
        }
    } else if(block_view == BlockViewAccessBits) {
        uint8_t sector_trailer_num =
            mf_classic_get_sector_trailer_num_by_sector(app->current_sector);

        if(mf_classic_is_block_read(mf_classic_data, sector_trailer_num)) {
            furi_string_printf(
                app->data_view_header, "Access Bits (Block %u)", app->current_block);
            dialog_ex_set_header(
                dialog_ex,
                furi_string_get_cstr(app->data_view_header),
                63,
                3,
                AlignCenter,
                AlignTop);

            nfcAccessBits access_bits = get_block_access_bits(mf_classic_data, app->current_block);

            furi_string_printf(
                app->data_view_text,
                "C1: %i(%i), C2: %i(%i), C3: %i(%i)\n",
                FURI_BIT(access_bits.bits, 0),
                FURI_BIT(access_bits.check_bits, 0),
                FURI_BIT(access_bits.bits, 1),
                FURI_BIT(access_bits.check_bits, 1),
                FURI_BIT(access_bits.bits, 2),
                FURI_BIT(access_bits.check_bits, 2));

            if(access_bits.bits != access_bits.check_bits) {
                furi_string_cat(
                    app->data_view_text, "Access Bits are invalid.\nEntire sector inaccessible.");
            } else if(app->current_block == sector_trailer_num) {
                furi_string_cat(
                    app->data_view_text, access_sector_trailer_labels[access_bits.bits]);
            } else {
                furi_string_cat(app->data_view_text, access_data_block_labels[access_bits.bits]);
            }

            dialog_ex_set_center_button_text(dialog_ex, "Next");
            dialog_ex_set_left_button_text(dialog_ex, "Prev");
            dialog_ex_set_right_button_text(dialog_ex, "Edit");
        } else {
            dialog_ex_set_header(dialog_ex, "Access Bits", 63, 3, AlignCenter, AlignTop);
            furi_string_printf(
                app->data_view_text,
                "Access Bits unavailable.\nBlock %u has not been read.",
                sector_trailer_num);
        }
    } else if(block_view == BlockViewUserByte) {
        dialog_ex_set_header(dialog_ex, "User Byte", 63, 3, AlignCenter, AlignTop);

        uint8_t sector_trailer_num =
            mf_classic_get_sector_trailer_num_by_sector(app->current_sector);

        if(mf_classic_is_block_read(mf_classic_data, sector_trailer_num)) {
            furi_string_printf(
                app->data_view_text,
                "Free byte between\nAccess Bits and Key B:\n%02X",
                mf_classic_data->block[sector_trailer_num].data[9]);
            dialog_ex_set_right_button_text(dialog_ex, "Edit");
        } else {
            furi_string_printf(
                app->data_view_text,
                "Data unavailable.\nBlock %u has not been read.",
                sector_trailer_num);
        }
    } else {
        uint8_t current_block = app->current_block;
        furi_string_printf(app->data_view_header, "Block %u Data", current_block);
        dialog_ex_set_header(
            dialog_ex, furi_string_get_cstr(app->data_view_header), 63, 3, AlignCenter, AlignTop);

        // Only display a block if it is fully read, and, if it is a sector trailer, both keys are found
        if(mf_classic_is_block_read(mf_classic_data, current_block) &&
           (!mf_classic_is_sector_trailer(current_block) ||
            (mf_classic_is_key_found(mf_classic_data, app->current_sector, MfClassicKeyTypeA) &&
             mf_classic_is_key_found(mf_classic_data, app->current_sector, MfClassicKeyTypeB)))) {
            // Split block data across 2 even lines
            const uint8_t* block_data = mf_classic_data->block[current_block].data;
            furi_string_render_bytes(app->data_view_text, block_data, MF_CLASSIC_BLOCK_SIZE / 2);
            furi_string_push_back(app->data_view_text, '\n');
            furi_string_render_bytes(
                app->data_view_text,
                block_data + MF_CLASSIC_BLOCK_SIZE / 2,
                MF_CLASSIC_BLOCK_SIZE / 2);
            dialog_ex_set_right_button_text(dialog_ex, "Edit");
        } else {
            furi_string_set(
                app->data_view_text, "Data unavailable.\nBlock has not been fully read.");
        }
    }

    dialog_ex_set_text(
        dialog_ex, furi_string_get_cstr(app->data_view_text), 63, 31, AlignCenter, AlignCenter);
}

void nfcgoesbrr_scene_data_view_on_enter(void* context) {
    nfcGoesbrr* app = context;

    scene_data_view_update_display(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ViewDialogEx);
}

bool nfcgoesbrr_scene_data_view_on_event(void* context, SceneManagerEvent event) {
    nfcGoesbrr* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        BlockView block_view = scene_manager_get_scene_state(app->scene_manager, SceneDataView);
        if(block_view == BlockViewNormal) {
            if(event.event == DialogExResultRight) {
                // Block 0 and sector trailer blocks are risky edits

                //scene_manager_set_scene_state(app->scene_manager, SceneDataEdit, block_view);
                //scene_manager_next_scene(app->scene_manager, SceneDataEdit);
                furi_hal_vibro_on(true);
                furi_delay_ms(200);
                furi_hal_vibro_on(false);
                consumed = true;
            }
        } else if(block_view == BlockViewUID) {
            if(event.event == DialogExResultRight) {
                //scene_manager_set_scene_state(app->scene_manager, SceneDataEdit, block_view);
                //scene_manager_next_scene(app->scene_manager, SceneDataEdit);
                furi_hal_vibro_on(true);
                furi_delay_ms(200);
                furi_hal_vibro_on(false);
                consumed = true;
            } else if(event.event == DialogExResultCenter) {
                memcpy(
                    app->mf_classic_data->block[0].data,
                    app->mf_classic_data->iso14443_3a_data->uid,
                    app->mf_classic_data->iso14443_3a_data->uid_len);
                app->is_unsaved_changes = true;
                scene_data_view_update_display(app);

                consumed = true;
            }
        } else if(
            block_view == BlockViewManufacturerBytes || block_view == BlockViewKeyA ||
            block_view == BlockViewKeyB) {
            if(event.event == DialogExResultRight) {
                //scene_manager_set_scene_state(app->scene_manager, SceneDataEdit, block_view);
                //scene_manager_next_scene(app->scene_manager, SceneDataEdit);
                furi_hal_vibro_on(true);
                furi_delay_ms(200);
                furi_hal_vibro_on(false);
                consumed = true;
            }
        } else if(block_view == BlockViewBCC) {
            if(event.event == DialogExResultCenter) {
                // Fix BCC byte by setting it to calculated one
                app->mf_classic_data->block[0].data[4] = calculate_uid_bcc(
                    app->mf_classic_data->iso14443_3a_data->uid,
                    app->mf_classic_data->iso14443_3a_data->uid_len);
                app->is_unsaved_changes = true;
                scene_data_view_update_display(app);
                consumed = true;
            }
        } else if(block_view == BlockViewAccessBits) {
            if(event.event == DialogExResultLeft) {
                uint8_t new_sector = mf_classic_get_sector_by_block(--app->current_block);
                if(new_sector != app->current_sector) {
                    app->current_block =
                        mf_classic_get_sector_trailer_num_by_sector(app->current_sector);
                }
                scene_data_view_update_display(app);
                consumed = true;
            } else if(event.event == DialogExResultCenter) {
                uint8_t new_sector = mf_classic_get_sector_by_block(++app->current_block);
                if(new_sector != app->current_sector) {
                    app->current_block =
                        mf_classic_get_first_block_num_of_sector(app->current_sector);
                }
                scene_data_view_update_display(app);
                consumed = true;
            } else if(event.event == DialogExResultRight) {
                //scene_manager_next_scene(app->scene_manager, SceneDataEditAccessBits);
            }
        } else if(block_view == BlockViewUserByte) {
            //scene_manager_set_scene_state(app->scene_manager, SceneDataEdit, block_view);
            //scene_manager_next_scene(app->scene_manager, SceneDataEdit);
            furi_hal_vibro_on(true);
            furi_delay_ms(200);
            furi_hal_vibro_on(false);
            consumed = true;
        }
    }

    return consumed;
}

void nfcgoesbrr_scene_data_view_on_exit(void* context) {
    nfcGoesbrr* app = context;

    dialog_ex_reset(app->dialog_ex);
}
