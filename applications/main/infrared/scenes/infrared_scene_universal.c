#include "../infrared_app_i.h"

typedef enum {
    SubmenuIndexUniversalTV,
    SubmenuIndexUniversalProjector,
    SubmenuIndexUniversalAudio,
    SubmenuIndexUniversalAirConditioner,
    SubmenuIndexUniversalLEDs,
    SubmenuIndexUniversalFan,
    SubmenuIndexUniversalBluray,
    SubmenuIndexUniversalMonitor,
    SubmenuIndexUniversalDigitalSign,
} SubmenuIndex;

static void infrared_scene_universal_submenu_callback(void* context, uint32_t index) {
    InfraredApp* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, index);
}

void infrared_scene_universal_on_enter(void* context) {
    InfraredApp* infrared = context;
    Submenu* submenu = infrared->submenu;

    submenu_add_item(
        submenu,
        "TVs",
        SubmenuIndexUniversalTV,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "Projectors",
        SubmenuIndexUniversalProjector,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "Audio",
        SubmenuIndexUniversalAudio,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "ACs",
        SubmenuIndexUniversalAirConditioner,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "LEDs",
        SubmenuIndexUniversalLEDs,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "Fans",
        SubmenuIndexUniversalFan,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "Blu-ray/DVDs",
        SubmenuIndexUniversalBluray,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "Monitors",
        SubmenuIndexUniversalMonitor,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_add_item(
        submenu,
        "Digital Signs",
        SubmenuIndexUniversalDigitalSign,
        infrared_scene_universal_submenu_callback,
        context);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(infrared->scene_manager, InfraredSceneUniversal));

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewSubmenu);
}

bool infrared_scene_universal_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexUniversalTV) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalTV);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalProjector) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalProjector);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalAudio) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalAudio);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalAirConditioner) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalAC);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalLEDs) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalLEDs);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalFan) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalFan);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalBluray) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalBluray);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalMonitor) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalMonitor);
            consumed = true;
        } else if(event.event == SubmenuIndexUniversalDigitalSign) {
            scene_manager_next_scene(scene_manager, InfraredSceneUniversalDigitalSign);
            consumed = true;
        }
        scene_manager_set_scene_state(scene_manager, InfraredSceneUniversal, event.event);
    }

    return consumed;
}

void infrared_scene_universal_on_exit(void* context) {
    InfraredApp* infrared = context;
    submenu_reset(infrared->submenu);
}
